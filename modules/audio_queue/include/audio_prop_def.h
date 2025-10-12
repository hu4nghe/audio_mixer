/**
 * @file audio_prop_def.h
 * @author  HUANG He (he.hu4ng@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2025-10-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <algorithm>
#include <optional>

#include "channel.h"

template<class InputType>
concept AudioSampleType = std::integral<InputType> || std::floating_point<InputType>;

enum class sample_rate : std::uint32_t
{
	SR44100	 = 44100,
	SR48000	 = 48000,
	SR88200	 = 88200,
	SR96000	 = 96000,
	SR176400 = 176400,
	SR192000 = 192000
};

struct audio_context
{
	sample_rate	   m_sample_rate = sample_rate::SR44100;
	channel_layout m_channel_num = channel_layout::Stereo;

	audio_context(sample_rate s_rate, std::string_view channel)
		: m_sample_rate(s_rate),
		  m_channel_num(channel)
	{}

	// Comparaison
	bool operator==(const audio_context& rhs) const = default;
	bool operator!=(const audio_context& rhs) const = default;

	/**
     * @brief Check if input need a channel mapping to fit current layout.
     * 
     * @param other input context
     * @return std::optional<channel_layout::MatrixType> Posible mapping channel
     */
	[[nodiscard]] auto need_conversion(const audio_context& other) const
		-> std::optional<channel_layout::MatrixType>
	{
		// Convert from other(usually input) to me(usually expected)
		if (m_channel_num != other.m_channel_num)
			return other.m_channel_num.matrix_to(m_channel_num);

		return std::nullopt;
	}

	/**
     * @brief Check if input need a resample to fit current sample rate.
     * 
     * @param other input context
     * @return std::optional<double> Possible resample ratio
     */
	[[nodiscard]] auto need_resample(const audio_context& other) const
		-> std::optional<double>
	{
		// Convert from other(usually input) to me(usually expected)
		return m_sample_rate != other.m_sample_rate ? std::make_optional(std::to_underlying(m_sample_rate) / std::to_underlying(other.m_sample_rate)) : std::nullopt;
	}
};

/**
 * @brief Creates type-specific conversion functions between audio_type and normalized float samples.
 *
 * This function returns a pair of callable objects (lambdas) that perform:
 * - **to_float**: converts a sample of type audio_type to a normalized float value in [-1.0, 1.0].
 * - **from_float**: converts a normalized float sample back to the target audio_type, applying proper scaling and clamping.
 *
 * These converters are designed to be lightweight, stateless, and compatible with C++ ranges pipelines.
 *
 * @tparam audio_type  The underlying audio sample type (e.g., int16_t, float, double, uint8_t, etc.)
 * @return constexpr auto  A pair { to_float, from_float } of conversion lambdas.
 *
 * @note Both lambdas are guaranteed to be trivially copyable and captureless, 
 *       making them safe for use in parallel algorithms and range-based transformations.
 *       
 */
template<AudioSampleType AudioType>
constexpr auto
make_audio_converters()
{
	if constexpr (std::is_floating_point_v<AudioType>)
		return std::pair{
			[](AudioType val) { return val; },
			[](float val) { return static_cast<AudioType>(val); }};

	else if constexpr (std::is_integral_v<AudioType>)
	{
		using Limits			  = std::numeric_limits<AudioType>;
		constexpr float max_float = 1.0F;

		if constexpr (std::is_signed_v<AudioType>)
		{
			constexpr float scale = -static_cast<float>(Limits::min());
			return std::pair{
				[](AudioType val) { return static_cast<float>(val) / scale; },
				[=](float val) { return static_cast<AudioType>(std::clamp(val, -max_float, max_float) * scale); }};
		}
		else // unsigned
		{
			constexpr float half_range = static_cast<float>(Limits::max()) / 2.0F;
			constexpr float offset	   = half_range + 0.5F;
			return std::pair{
				[](AudioType val) { return (static_cast<float>(val) - offset) / half_range; },
				[=](float val) { return static_cast<AudioType>((std::clamp(val, -max_float, max_float) * half_range) + offset); }};
		}
	}
}
