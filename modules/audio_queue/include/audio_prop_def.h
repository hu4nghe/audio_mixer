#include <cstdint>

enum class channel_layout : std::uint8_t 
{
    mono            = 1,   // 1.0 
    stereo          = 2,   // 2.0 
    five_point_one  = 6,   // 5.1 
    seven_point_one = 8, // 7.1
};

#include <cstdint>
enum class sample_rate : std::uint32_t 
{
    SR8000   = 8000,
    SR16000  = 16000,
    SR22050  = 22050,
    SR32000  = 32000,
    SR44100  = 44100,
    SR48000  = 48000,
    SR88200  = 88200,
    SR96000  = 96000,
    SR176400 = 176400,
    SR192000 = 192000
};