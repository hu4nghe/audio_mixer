/**
 * @file channel.h
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
#include <vector>
#include <string_view>

class channel_layout 
{
public:
    enum value : std::uint8_t 
    {
        mono            = 1,  // 1.0
        stereo          = 2,  // 2.0
        five_point_one  = 6,  // 5.1
        seven_point_one = 8   // 7.1
    };

    constexpr channel_layout() = default;
    constexpr channel_layout(value v) : value(v) {}

    /**
     * @brief Construct from a string_view
     * 
     * @param name the channel layout name, available names :
     * 
     *      mono
     *      stereo
     *      5.1
     *      7.1
     *
     */
    channel_layout(const std::string_view name);
    channel_layout(const char* name) : channel_layout(std::string_view{name}) {}

    /**
     * @brief Convert enum to string. 
     * 
     * @return const char* const Name of channel layout, see the constrctor above.
     */
    auto to_str() -> const char* const;

    // Mathematical operation & switch support
    constexpr operator value() const { return value; }

    // Comparaison
    constexpr bool operator==(channel_layout other) const { return value == other.value; }
    constexpr bool operator!=(channel_layout other) const { return value != other.value; }

    // Get channel count
    constexpr auto channels() const { return static_cast<size_t>(value); }


    using matrix_type = std::vector<std::vector<float>>;
    /**
     * @brief Matrix for channel conversion
     * 
     * @param input The input channel layout.
     * @return std::vector<std::vector<float>> The matrix to use 
     */
    auto matrix_to(channel_layout target) const -> matrix_type;

    

private:
    value value = stereo;
};
