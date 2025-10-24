/**
 * @file sample_rate.h
 * @author HUANG He (he.hu4ng@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2025-10-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <cstdint>
#include <string_view>

struct sample_rate
{
	/**
     * @brief Sample rate enum
     * 
     */
	enum value : uint32_t
	{
		SR44100	 = 44100,
		SR48000	 = 48000,
		SR88200	 = 88200,
		SR96000	 = 96000,
		SR176400 = 176400,
		SR192000 = 192000
	};

	/**
     * @brief Construct from a enum value.
     * @param value Enum value for construction
     */
	constexpr sample_rate(value val)
		: m_value(val)
	{}

	/**
     * @brief Construct from a string_view.
     * 
     * @param name the channel layout name, available names :
     * 
     *      SR44100
     *      SR48000
     *      SR88200
     *      SR96000
     *      SR176400
     *      SR192000
     */
	sample_rate(std::string_view name);

	// Comparaison
	constexpr bool operator==(sample_rate rhs) const { return m_value == rhs.m_value; }
	constexpr bool operator==(value rhs) const { return m_value == rhs; }
	constexpr bool operator!=(sample_rate rhs) const { return m_value != rhs.m_value; }
	constexpr bool operator!=(value rhs) const { return m_value != rhs; }

	/**
     * @brief Value operator, return sample rate in uint32_t.
     * 
     * @return value sample rate in uint32_t
     */
	constexpr operator value() const { return m_value; }

private:

	value m_value = SR44100;
};
