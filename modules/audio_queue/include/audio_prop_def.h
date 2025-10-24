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
#include "sample_rate.h"

template <class InputType>
concept audio_sample_type = std::integral<InputType> || std::floating_point<InputType>;

struct audio_ctx
{
    sample_rate     m_sample_rate = sample_rate::SR44100;
    channel_layout  m_channel_num = channel_layout::Stereo;
    
    audio_ctx(sample_rate s_rate, std::string_view channel) : m_sample_rate(s_rate), m_channel_num(channel) {}

    // Comparaison
    bool operator==(const audio_ctx& rhs) const { return m_channel_num == rhs.m_channel_num && m_sample_rate == rhs.m_sample_rate; }
    bool operator!=(const audio_ctx& rhs) const { return m_channel_num != rhs.m_channel_num || m_sample_rate != rhs.m_sample_rate; }

    [[nodiscard]] auto 
    need_conversion(const audio_ctx& other) const
    -> std::optional<channel_layout::MatrixType> 
    {
        
        // Convert from other(usually input) to me(usually expected)
        if(m_channel_num != other.m_channel_num)
            return other.m_channel_num.matrix_to(m_channel_num);
    
        return std::nullopt;
    }

    [[nodiscard]] auto 
    need_resample(const audio_ctx& other) const
    -> std::optional<double>
    {
        // Convert from other(usually input) to me(usually expected)
        return m_sample_rate != other.m_sample_rate ? 
                std::make_optional(m_sample_rate / other.m_sample_rate): 
                std::nullopt;
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
template <audio_sample_type AudioType>
constexpr auto 
make_audio_converters()
{
    if constexpr (std::is_floating_point_v<AudioType>)
        return std::pair
        {
            [](AudioType val) { return val; },
            [](float val) { return static_cast<AudioType>(val); }
        };
    
    else if constexpr (std::is_integral_v<AudioType>)
    {
        using Limits = std::numeric_limits<AudioType>;
        constexpr float max_float = 1.0F;
        
        if constexpr (std::is_signed_v<AudioType>)
        {
            constexpr float scale = -static_cast<float>(Limits::min());
            return std::pair
            {
                [](AudioType val) { return static_cast<float>(val) / scale; },
                [=](float val) { return static_cast<AudioType>(std::clamp(val, -max_float, max_float) * scale); }
            };
        }
        else // unsigned
        {
            constexpr float half_range = static_cast<float>(Limits::max()) / 2.0F;
            constexpr float offset = half_range + 0.5F;
            return std::pair
            {
                [](AudioType val) { return (static_cast<float>(val) - offset) / half_range; },
                [=](float val) { return static_cast<AudioType>((std::clamp(val, -max_float, max_float) * half_range) + offset); }
            };
        }
    }
}
