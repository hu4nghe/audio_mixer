#include "lockfree_queue.h"
#include "audio_prop_def.h"

#include <utility>

constexpr std::size_t calculate_capacity(sample_rate sr, std::uint32_t latency_ms)
{
    return std::to_underlying(sr) * latency_ms / 1000;
}

template <class audio_type>
class audio_queue
{
    lockfree::queue<audio_type> queue;

    sample_rate    expected_rate    = sample_rate::SR44100;     // 44.1 KHz
    channel_layout expected_layout  = channel_layout::stereo;   // Stereo
    std::size_t    latency          = 200;                      // 200 ms

public :
    audio_queue() : queue(calculate_capacity(expected_rate,latency)){}
    
    audio_queue(sample_rate expected_sr, std::uint32_t expected_lat) 
        :   expected_rate(expected_sr),
            queue(calculate_capacity(expected_rate,expected_lat)) {}
    
    audio_queue(const audio_queue&) = delete;
    audio_queue(audio_queue&&)      = delete;
    
    ~audio_queue() = default;

    bool push_audio(audio_type* data, std::size_t frames, channel_layout input_layout, sample_rate input_rate)
    {
        bool need_resample = input_rate != expected_rate;
        bool need_mapping  = input_layout != expected_layout;

        const std::size_t data_size = frames * std::to_underlying(input_layout);
    }
};