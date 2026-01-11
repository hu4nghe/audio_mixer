#include <format>
#include <stdexcept>

#include "sample_rate.h"

sample_rate::sample_rate(const std::string_view name)
{
    if (name == "SR44100") m_value = SR44100;
    else if (name == "SR48000") m_value = SR48000;
    else if (name == "SR88200") m_value = SR88200;
    else if (name == "SR96000") m_value = SR96000;
    else throw std::invalid_argument(std::format("Unknown channel layout : {}", name));
}
