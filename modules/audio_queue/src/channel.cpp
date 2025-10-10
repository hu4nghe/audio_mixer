#include <format>
#include <stdexcept>
#include <algorithm>

#include "channel.h"


channel_layout::channel_layout(const std::string_view name)
{
    if      (name == "mono")   value = mono;
    else if (name == "stereo") value = stereo;
    else if (name == "5.1")    value = five_point_one;
    else if (name == "7.1")    value = seven_point_one;

    else throw std::invalid_argument(std::format("Unknown channel layout : {}", name));
}

auto channel_layout::matrix_to(channel_layout target) const -> matrix_type
{
    size_t origin = this-> channels();
    size_t dest   = target.channels();

    std::vector<std::vector<float>> conversion_matrix(dest, std::vector<float>(origin, 0.0f));

    // Mono (1) → Stereo (2)
    if (origin == 1 && dest == 2) 
    {
        conversion_matrix[0][0] = 1.0f;
        conversion_matrix[1][0] = 1.0f;
    }
    // Stereo (2) → Mono (1)
    else if (origin == 2 && dest == 1) 
    {
        conversion_matrix[0][0] = 0.5f;     conversion_matrix[0][1] = 0.5f;
    }
    // Stereo (2) → 5.1 (6)
    else if (origin == 2 && dest == 6) 
    {
        conversion_matrix[0][0] = 1.0f;     conversion_matrix[1][1] = 1.0f;
        conversion_matrix[2][0] = 0.5f;     conversion_matrix[2][1] = 0.5f; // Center
    }
    // 5.1 (6) → Stereo (2)
    else if (origin == 6 && dest == 2) 
    {
        conversion_matrix[0][0] = 1.0f;     conversion_matrix[1][1] = 1.0f;
        conversion_matrix[0][2] = 0.707f;   conversion_matrix[1][2] = 0.707f;
        conversion_matrix[0][4] = 0.707f;   conversion_matrix[1][5] = 0.707f;
    }
    // 5.1 (6) → Mono (1)
    else if (origin == 6 && dest == 1) 
    {
        conversion_matrix[0][0] = 0.325f;   conversion_matrix[0][1] = 0.325f;
        conversion_matrix[0][2] = 0.45f;    conversion_matrix[0][3] = 0.1f;
        conversion_matrix[0][4] = 0.325f;   conversion_matrix[0][5] = 0.325f;
    }
    // 7.1 (8) → Stereo (2)
    else if (origin == 8 && dest == 2) 
    {
        conversion_matrix[0][0] = 1.0f;     conversion_matrix[1][1] = 1.0f;
        conversion_matrix[0][2] = 0.707f;   conversion_matrix[1][2] = 0.707f;
        conversion_matrix[0][4] = 0.5f;     conversion_matrix[1][5] = 0.5f;
        conversion_matrix[0][6] = 0.5f;     conversion_matrix[1][7] = 0.5f;
    }
    // 7.1 (8) → 5.1 (6)
    else if (origin == 8 && dest == 6) 
    {
        conversion_matrix[0][0] = 1.0f;     conversion_matrix[1][1] = 1.0f;
        conversion_matrix[2][2] = 1.0f;     conversion_matrix[3][3] = 1.0f;
        conversion_matrix[4][4] = 0.5f;     conversion_matrix[4][6] = 0.5f;
        conversion_matrix[5][5] = 0.5f;     conversion_matrix[5][7] = 0.5f;
    }
    // Default case
    else 
    {
        for (size_t i = 0; i < std::min(origin, dest); ++i) 
        {
        conversion_matrix[i][i] = 1.0f;
        }
    }

    return conversion_matrix;
}

auto channel_layout::to_str() -> const char* const 
{
    switch (value) 
    {
        case mono:              return "Mono";
        case stereo:            return "Stereo";
        case five_point_one:    return "5.1";
        case seven_point_one:   return "7.1";
        default:                return "Unknown";
    }
}