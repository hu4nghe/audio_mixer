/**
 * @file channel.h
 * @author  HUANG He (he.hu4ng@outlook.com)
 * @brief 
 * @version 0.2
 * @date 2025-10-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <cstdint>
#include <string_view>
#include <vector>


struct channel_layout
{
	/**
     * @brief Channel layout enum
     * 
     */
	enum value : uint8_t
	{
		Mono		  = 1, // 1.0
		Stereo		  = 2, // 2.0
		FivePointOne  = 6, // 5.1
		SevenPointOne = 8  // 7.1
	};

	// Channel mapping matrix type
	using MatrixType = std::vector<std::vector<float>>;

	/**
     * @brief Construct from a enum value.
     * @param value Enum value for construction
     */
	constexpr channel_layout(value val)
		: m_value(val)
	{}

	/**
     * @brief Construct from a string_view.
     * 
     * @param name the channel layout name, available names :
     * 
     *      Mono
     *      Stereo
     *      5.1
     *      7.1
     */
	channel_layout(std::string_view name);

	// Comparaison
	constexpr bool operator==(channel_layout rhs) const { return m_value == rhs.m_value; }
	constexpr bool operator==(value rhs) const { return m_value == rhs; }
	constexpr bool operator!=(channel_layout rhs) const { return m_value != rhs.m_value; }
	constexpr bool operator!=(value rhs) const { return m_value != rhs; }

	/**
     * @brief Value operator, return number of channels.
     * 
     * @return value number of channels
     */
	constexpr operator value() const { return m_value; }

	/**
     * @brief Matrix for channel conversion
     * 
     * @param input The input channel layout.
     * @return std::vector<std::vector<float>> The matrix to use 
     */
	[[nodiscard]] 
    MatrixType matrix_to(channel_layout target) const;

	/**
     * @brief Return the value name in string.
     * 
     * @return const char* const Name of channel layout, see the constrctor above.
     */
	[[nodiscard]] 
    const char* to_str();

private:

	value m_value = Stereo;
};
