/**
 * @file audio_prop_def.h
 * @author  ()
 * @brief 
 * @version 0.1
 * @date 2025-10-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>

#include "channel.h"

template <class input_type>
concept audio_sample_type = std::integral<input_type> || std::floating_point<input_type>;

enum class sample_rate : std::uint32_t 
{
    SR44100  = 44100,
    SR48000  = 48000,
    SR88200  = 88200,
    SR96000  = 96000,
    SR176400 = 176400,
    SR192000 = 192000
};

struct audio_ctx
{
    sample_rate     _sample_rate = sample_rate::SR44100;
    channel_layout  _channel_num = channel_layout::stereo;
    
    auto need_conversion(const audio_ctx& other) const
    -> std::optional<channel_layout::matrix_type> 
    {
        if(_channel_num != other._channel_num)
            // Convert from other(usually input) to me(usually expected)
            return other._channel_num.matrix_to(_channel_num);
        else 
            return std::nullopt;
    }

    auto need_resample(const audio_ctx& other) const
    -> std::optional<double>
    {
        if(_sample_rate != other._sample_rate)
            // Convert from other(usually input) to me(usually expected)
            return std::to_underlying(_sample_rate) / std::to_underlying(other._sample_rate);
        else
            return std::nullopt;
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
*        making them safe for use in parallel algorithms and range-based transformations.
 *       
 */

template <audio_sample_type audio_type>
constexpr auto make_audio_converters()
{

    if constexpr (std::same_as<audio_type, float>)
    {
        return std::pair
        {
            [](float s) { return s; },
            [](float s) { return s; }
        };
    }
    else if constexpr (std::same_as<audio_type, double>)
    {
        return std::pair
        {
            [](double s) { return static_cast<float>(clamp(s, -1.0, 1.0)); },
            [](float  s) { return static_cast<double>(clamp(s, -1.0f, 1.0f)); }
        };
    }
    else if constexpr (std::same_as<audio_type, int16_t>)
    {
        return std::pair
        {
            [](int16_t s) { return static_cast<float>(s) / 32768.0f; },
            [](float   s) { return static_cast<int16_t>(clamp(s, -1.0f, 1.0f) * 32767.0f); }
        };
    }
    else if constexpr (std::same_as<audio_type, int32_t>)
    {
        return std::pair
        {
            [](int32_t s) { return static_cast<float>(s) / 2147483648.0f; },
            [](float   s) { return static_cast<int32_t>(clamp(s, -1.0f, 1.0f) * 2147483647.0f); }
        };
    }
    else if constexpr (std::same_as<audio_type, uint8_t>)
    {
        return std::pair
        {
            [](uint8_t s) { return (static_cast<float>(s) - 128.0f) / 128.0f; },
            [](float   s) { return static_cast<uint8_t>(clamp(s, -1.0f, 1.0f) * 128.0f + 128.0f); }
        };
    }
    else
    {
        static_assert(!sizeof(audio_type), "Unsupported type");
    }
}