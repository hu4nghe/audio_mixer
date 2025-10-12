#include <algorithm>
#include <print>

#include "channel.h"

channel_layout::channel_layout(const std::string_view name)
{
	if (name == "Mono")
		m_value = Mono;
	else if (name == "Stereo")
		m_value = Stereo;
	else if (name == "5.1")
		m_value = FivePointOne;
	else if (name == "7.1")
		m_value = SevenPointOne;
	else
	{
		std::println("Invalide value.");
		m_value = Stereo;
	}
}

auto channel_layout::matrix_to(channel_layout target) const -> MatrixType
{
	using Value = channel_layout::value;

	// Mapping coefficients
	constexpr float coef_full_gain	   = 1.0F;
	constexpr float coef_half_gain	   = 0.5F;
	constexpr float coef_surround_gain = 0.707F;
	constexpr float coef_centre_gain   = 0.45F;
	constexpr float coef_mono_mix_gain = 0.325F;
	constexpr float coef_mono_low_gain = 0.1F;

	// Origin channel number and destination channel number
	auto origin = static_cast<size_t>(*this);
	auto dest	= static_cast<size_t>(target);

	// Possible output matrix
	MatrixType conversion_matrix(dest, std::vector<float>(origin, 0.0F));

	if (*this == Value::Mono && target == Value::Stereo)
	{
		conversion_matrix[0][0] = coef_full_gain;
		conversion_matrix[1][0] = coef_full_gain;
	}
	else if (*this == Value::Stereo && target == Value::Mono)
	{
		conversion_matrix[0][0] = coef_half_gain;
		conversion_matrix[0][1] = coef_half_gain;
	}
	else if (*this == Value::Stereo && target == Value::FivePointOne)
	{
		conversion_matrix[0][0] = coef_full_gain;
		conversion_matrix[1][1] = coef_full_gain;
		conversion_matrix[2][0] = coef_half_gain; // Center
		conversion_matrix[2][1] = coef_half_gain;
	}
	else if (*this == Value::FivePointOne && target == Value::Stereo)
	{
		conversion_matrix[0][0] = coef_full_gain;
		conversion_matrix[1][1] = coef_full_gain;
		conversion_matrix[0][2] = coef_surround_gain;
		conversion_matrix[1][2] = coef_surround_gain;
		conversion_matrix[0][4] = coef_surround_gain;
		conversion_matrix[1][5] = coef_surround_gain;
	}
	else if (*this == Value::FivePointOne && target == Value::Mono)
	{
		conversion_matrix[0][0] = coef_mono_mix_gain;
		conversion_matrix[0][1] = coef_mono_mix_gain;
		conversion_matrix[0][2] = coef_centre_gain;
		conversion_matrix[0][3] = coef_mono_low_gain;
		conversion_matrix[0][4] = coef_mono_mix_gain;
		conversion_matrix[0][5] = coef_mono_mix_gain;
	}
	else if (*this == Value::SevenPointOne && target == Value::Stereo)
	{
		conversion_matrix[0][0] = coef_full_gain;
		conversion_matrix[1][1] = coef_full_gain;
		conversion_matrix[0][2] = coef_surround_gain;
		conversion_matrix[1][2] = coef_surround_gain;
		conversion_matrix[0][4] = coef_half_gain;
		conversion_matrix[1][5] = coef_half_gain;
		conversion_matrix[0][6] = coef_half_gain;
		conversion_matrix[1][7] = coef_half_gain;
	}
	else if (*this == Value::SevenPointOne && target == Value::FivePointOne)
	{
		conversion_matrix[0][0] = coef_full_gain;
		conversion_matrix[1][1] = coef_full_gain;
		conversion_matrix[2][2] = coef_full_gain;
		conversion_matrix[3][3] = coef_full_gain;
		conversion_matrix[4][4] = coef_half_gain;
		conversion_matrix[4][6] = coef_half_gain;
		conversion_matrix[5][5] = coef_half_gain;
		conversion_matrix[5][7] = coef_half_gain;
	}
	else
		for (size_t i = 0; i < std::min(origin, dest); ++i)
			conversion_matrix[i][i] = coef_full_gain;

	return conversion_matrix;
}

auto channel_layout::to_str() -> const char*
{
	switch (m_value)
	{
		case Mono:
			return "Mono";
		case Stereo:
			return "Stereo";
		case FivePointOne:
			return "5.1";
		case SevenPointOne:
			return "7.1";
		default:
			return "Unknown";
	}
}
