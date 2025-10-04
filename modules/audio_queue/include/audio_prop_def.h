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

#include <cstdint>
#include <optional>
#include <utility>

#include "channel.h"

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