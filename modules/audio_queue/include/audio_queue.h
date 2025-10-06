/**
 * @file audio_queue.h
 * @author  ()
 * @brief 
 * @version 0.1
 * @date 2025-10-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "lockfree_queue.h"
#include "audio_prop_def.h"
#include "samplerate.h"

#include <numeric>
#include <ranges>
#include <print>

template <audio_sample_type audio_type>
class audio_queue
{
    static constexpr auto default_latency_ms = 200;
    
    audio_ctx              _expected_context;
    lockfree::queue<float> _queue;
public :
    /**
     * @brief Default Constructor : 44.1 KHz, Stereo, Capacity for 200 ms
     * 
     */
    audio_queue() : _queue(_expected_context._channel_num * std::to_underlying(_expected_context._sample_rate) * default_latency_ms / 1000) {}
    
    /**
     * @brief Construct a queue with user expected audio context and latence.
     * 
     * @param user_expected_ctx User expected output audio context.
     * @param user_expected_lat_ms User expected queue capacity (in latency, ms)
     */
    audio_queue(audio_ctx user_expected_ctx, size_t user_expected_lat_ms = 200) 
        :   _queue(user_expected_ctx._channel_num * std::to_underlying(_expected_context._sample_rate) * user_expected_lat_ms / 1000),
            _expected_context(user_expected_ctx) {}
    
    /* Copy or move a queue is not allowed */
    audio_queue(const audio_queue&) = delete;
    audio_queue(audio_queue&&)      = delete;

    /**
     * @brief Push a sequence of audio into the audio queue.
     * 
     * @param input_context Input audio context
     * @param input_data Input audio data array
     * @param input_frame Input audio frame count
     * @return true Push operation succeeded
     * @return false Push operation failed
     */
    bool push_audio(const audio_ctx& input_context, audio_type* input_data, std::size_t input_frame)
    {
        // Converte all sample into float format.
        const uint8_t input_channels = input_context._channel_num;
        
        auto [to_float, _] = make_audio_converters<audio_type>();
        auto input_data_float = std::span{input_data, input_frame * input_channels}
            | std::views::transform(to_float)
            | std::ranges::to<std::vector<float>>();
        
        // Buffer for possible resample operation.
        std::vector<float> temp; 
        // Buffer for possible channel mapping operation
        std::vector<float> output_audio;

        if (auto ratio_opt = _expected_context.need_resample(input_context))
        {
            const double ratio = *ratio_opt;
            const size_t expected_output_frame = static_cast<size_t>(input_frame * ratio) + 1;
            
            temp.resize(expected_output_frame * input_channels);

            int err_code = 0;
            auto src_state = src_new(SRC_SINC_BEST_QUALITY, input_channels, &err_code);
            if (src_state && !err_code)
            {
                SRC_DATA src_data{};
                src_data.end_of_input  = false;
                src_data.data_in       = input_data_float.data();
                src_data.data_out      = temp.data();
                src_data.input_frames  = input_frame;
                src_data.output_frames = expected_output_frame;
                src_data.src_ratio     = ratio;

                err_code = src_process(src_state, &src_data);

                if(err_code) // Print error message(src_process failed).
                    std::println(stderr,"libsamplerate error : {}", src_strerror(err_code));
                else // Resample succeeded, cut the buffer size base on generated frame.
                    temp.resize(src_data.output_frames_gen * input_channels);
                
                src_delete (src_state);
            }
            else // Print error message(src_new failed).
                std::println(stderr, "libsamplerate error : {}", src_strerror(err_code));
                
            if(err_code) // Return false in case of failure.
                return false;
        }
        else // No need of resample 
            temp = std::move(input_data_float);

        if (auto matrix_opt = _expected_context.need_conversion(input_context)) 
        {
            const size_t output_channel_number = _expected_context._channel_num;
            const size_t temp_frame            = temp.size() / input_channels;
            const auto   convert_matrix        = *matrix_opt;

            // Resize for channel mapping result
            output_audio.resize(temp_frame * output_channel_number);
            
            for (size_t frame_idx = 0; frame_idx < temp_frame; frame_idx++)
            {
                float* in_frame  = &temp[frame_idx * input_channels];
                float* out_frame = &output_audio[frame_idx * output_channel_number];

                for (auto& row : convert_matrix)
                    out_frame[&row - &convert_matrix[0]] = std::inner_product(row.begin(), row.end(), in_frame, 0.0f);
            }
        }

        // Push audio data into audio queue
        size_t drops = 0;
        for (const auto sample : output_audio.empty() ? temp : output_audio) 
            if (!_queue.enqueue(sample)) 
                drops++;

        if (drops) 
        {
            std::println(stderr, "push_audio: dropped {} samples (queue full?)\n", drops);
            return false;
        }
        else
            return true;
    }
    
    bool pop_audio(const audio_ctx& output_ctx, audio_type* output_buffer, std::size_t frame_count)
    {
        const size_t total_samples = frame_count * output_ctx._channel_num;

        auto [to_float, from_float] = make_audio_converters<audio_type>();

        std::vector<float> output_as_float(total_samples);
        std::ranges::transform( std::span{output_buffer, total_samples}, output_as_float.begin(), to_float);

        size_t popped = 0;
        float sample;
        while (popped < total_samples && _queue.dequeue(sample)) 
        {
            output_as_float[popped] = std::clamp(output_as_float[popped++] + sample, -1.0f, 1.0f);
            ++popped;
        }

        std::ranges::transform(output_as_float, output_buffer, from_float);

        return popped == total_samples;
    }
};