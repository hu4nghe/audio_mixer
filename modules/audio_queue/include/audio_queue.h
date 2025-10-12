/**
 * @file audio_queue.h
 * @author  HUANG He (he.hu4ng@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2025-10-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <numeric>
#include <print>
#include <ranges>

#include "audio_prop_def.h"
#include "lockfree_queue.h"
#include "samplerate.h"

template<AudioSampleType AudioType>
struct audio_queue
{
	/**
     * @brief Default Constructor : 44.1 KHz, Stereo, Capacity for 200 ms
     * 
     */
	audio_queue()
		: m_queue(m_expected_context.m_channel_num * std::to_underlying(m_expected_context.m_sample_rate) * default_latency_ms / 1000)
	{}

	/**
     * @brief Construct a queue with user expected audio context and latence.
     *
     * @param user_expected_ctx User expected output audio context.
     * @param user_expected_lat_ms User expected queue capacity (in latency, ms)
     */
	audio_queue(audio_context user_expected_ctx, size_t user_expected_lat_ms = 200)
		: m_expected_context(user_expected_ctx),
		  m_queue(static_cast<size_t>(user_expected_ctx.m_channel_num * std::to_underlying(user_expected_ctx.m_sample_rate)) * user_expected_lat_ms / 1000)
	{}

	/* Copy or move a queue is not allowed */
	audio_queue(const audio_queue&)			   = delete;
	audio_queue(audio_queue&&)				   = delete;
	audio_queue& operator=(const audio_queue&) = delete;
	audio_queue& operator=(audio_queue&&)	   = delete;

	/**
     * @brief Default destructor.
     * 
     */
	~audio_queue() = default;

	/**
     * @brief Push a sequence of audio into the audio queue.
     * 
     * @param input_context Input audio context
     * @param input_data Input audio data array
     * @param input_frame Input audio frame count
     * @return true Push operation succeeded
     * @return false Push operation failed
     */
	bool push_audio(const audio_context& input_context, AudioType* input_data, std::size_t input_frame)
	{
		// Converte all sample into float format.
		const uint8_t input_channels = input_context.m_channel_num;

		auto [to_float, _]			 = make_audio_converters<AudioType>();
		auto input_data_float		 = std::span{input_data, input_frame * input_channels} | std::views::transform(to_float) | std::ranges::to<std::vector<float>>();

		// Buffer for possible resample operation.
		std::vector<float> temp;
		// Buffer for possible channel mapping operation
		std::vector<float> output_audio;

		if (auto ratio_opt = m_expected_context.need_resample(input_context))
		{
			const auto ratio				 = *ratio_opt;
			const auto expected_output_frame = static_cast<size_t>(static_cast<double>(input_frame) * ratio) + 1;

			temp.resize(expected_output_frame * input_channels);

			// Resample with libsamplerate.
			int	  err_code	= 0;
			auto* src_state = src_new(SRC_SINC_BEST_QUALITY, input_channels, &err_code);
			if (src_state && (err_code == 0))
			{
				SRC_DATA src_data{};
				src_data.end_of_input  = 0;
				src_data.data_in	   = input_data_float.data();
				src_data.data_out	   = temp.data();
				src_data.input_frames  = static_cast<long>(input_frame);
				src_data.output_frames = static_cast<long>(expected_output_frame);
				src_data.src_ratio	   = ratio;

				err_code			   = src_process(src_state, &src_data);

				if (err_code != 0) // Print error message(src_process failed).
					std::println(stderr, "libsamplerate error : {}", src_strerror(err_code));
				else // Resample succeeded, cut the buffer size base on generated frame.
					temp.resize(static_cast<long>(src_data.output_frames_gen * input_channels));

				src_delete(src_state);
			}
			else // Print error message(src_new failed).
				std::println(stderr, "libsamplerate error : {}", src_strerror(err_code));

			if (err_code != 0)
				return false;
		}
		else // No need of resample
			temp = std::move(input_data_float);

		if (auto matrix_opt = m_expected_context.need_conversion(input_context))
		{
			const size_t output_channel_number = m_expected_context.m_channel_num;
			const size_t temp_frame			   = temp.size() / input_channels;
			const auto&	 convert_matrix		   = *matrix_opt;

			// Resize for channel mapping result
			output_audio.resize(temp_frame * output_channel_number);

			for (size_t frame_idx = 0; frame_idx < temp_frame; frame_idx++)
			{
				float* in_frame	 = &temp[frame_idx * input_channels];
				float* out_frame = &output_audio[frame_idx * output_channel_number];

				for (const auto& row : convert_matrix)
					out_frame[&row - convert_matrix.data()] = std::inner_product(row.begin(), row.end(), in_frame, 0.0F);
			}
		}

		// Push audio data into audio queue
		size_t drops = 0;
		for (const auto sample : output_audio.empty() ? temp : output_audio)
			if (!m_queue.enqueue(sample))
				drops++;

		if (drops != 0)
		{
			std::println(stderr, "push_audio: dropped {} samples (queue full?)\n", drops);
			return false;
		}

		return true;
	}

	/**
     * @brief Pop a sequence of audio from the audio queue.
     * 
     * @param output_ctx Output buffer context(if it is not the same with the expected context, operation will fail)
     * @param output_buffer Output buffer array
     * @param frame_count Output buffer frame count
     * @return true Pop operation succeeded
     * @return false Pop operation failed
     */
	bool pop_audio(const audio_context& output_ctx, AudioType* output_buffer, std::size_t frame_count)
	{
		// Verify consistency between output and expectated context
		if (output_ctx != m_expected_context)
		{
			std::println(stderr, "pop_audio : output_ctx must match expected_context");
			return false;
		}
		// Theoretical sample array size
		const size_t	   total_samples = frame_count * m_expected_context.m_channel_num;
		std::vector<float> output_as_float(total_samples);

		// Convert existing data to float
		auto [to_float, from_float] = make_audio_converters<AudioType>();
		std::ranges::transform(std::span{output_buffer, total_samples}, output_as_float.begin(), to_float);

		// Try pop from queue
		size_t popped = 0;
		float  sample = NAN;
		while (popped < total_samples && m_queue.dequeue(sample))
		{
			// Mixing mode : Add pop element to existing audio data.
			output_as_float[popped] = std::clamp(output_as_float[popped] + sample, -1.0F, 1.0F);
			++popped;
		}

		// Convert to original type
		std::ranges::transform(output_as_float, output_buffer, from_float);

		return popped == total_samples;
	}

private:

	static constexpr auto  default_latency_ms = 200;

	audio_context		   m_expected_context;
	lockfree::queue<float> m_queue;
};