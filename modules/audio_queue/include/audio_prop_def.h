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
 * @brief Tool functions to convert all input into float type(libsamplerate's resample method takes float* )
 * 
 * @param input input sequence
 * @return std::vector<float> data converted to float
 */
template <audio_sample_type audio_type>
auto convert_to_float(std::span<const audio_type> input) 
-> std::vector<float> 
{
    std::vector<float> output(input.size());
    if constexpr (std::same_as<audio_type, float>) 
        std::ranges::copy(input, output.begin());
    else if constexpr (std::same_as<audio_type, double>)
        std::ranges::transform(input, output.begin(), [](double  s){ return static_cast<float>(std::clamp(s, -1.0, 1.0)); });
    else if constexpr (std::same_as<audio_type, int16_t>) 
        std::ranges::transform(input, output.begin(), [](int16_t s){ return static_cast<float>(s) / 32768.0f; });
    else if constexpr (std::same_as<audio_type, int32_t>) 
        std::ranges::transform(input, output.begin(), [](int32_t s){ return static_cast<float>(s) / 2147483648.0f; });
    else if constexpr (std::same_as<audio_type, uint8_t>) 
        std::ranges::transform(input, output.begin(), [](uint8_t s){ return (static_cast<float>(s) - 128.0f) / 128.0f; });
    return output;
}

/**
 * @brief Tool functions to convert internal float type to another audio type(for output)
 * 
 * @tparam audio_type 
 * @param input 
 * @return std::vector<audio_type> 
 */
template <audio_sample_type audio_type>
auto convert_from_float(std::span<const float> input)
    -> std::vector<audio_type>
{
    std::vector<audio_type> output(input.size());
    if constexpr (std::same_as<audio_type, float>)
        std::ranges::copy(input, output.begin());
    else if constexpr (std::same_as<audio_type, double>)
        std::ranges::transform(input, output.begin(), [](float s) { return static_cast<double >(std::clamp(s, -1.0f, 1.0f)); });
    else if constexpr (std::same_as<audio_type, int16_t>)
        std::ranges::transform(input, output.begin(), [](float s) { return static_cast<int16_t>(std::clamp(s, -1.0f, 1.0f) * 32767.0f); });
    else if constexpr (std::same_as<audio_type, int32_t>)
        std::ranges::transform(input, output.begin(), [](float s) { return static_cast<int32_t>(std::clamp(s, -1.0f, 1.0f) * 2147483647.0f); });
    else if constexpr (std::same_as<audio_type, uint8_t>)
        std::ranges::transform(input, output.begin(), [](float s) { return static_cast<uint8_t>(std::clamp(s, -1.0f, 1.0f) * 128.0f + 128.0f); });
    return output;
}
