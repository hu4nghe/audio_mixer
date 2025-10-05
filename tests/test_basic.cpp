#include <catch2/catch_all.hpp>
#include "audio_queue.h"

TEST_CASE("AudioMixer basic passthrough", "[audio_queue]") 
{
    audio_ctx in_ctx{sample_rate::SR44100, "stereo"};
    audio_ctx out_ctx{sample_rate::SR44100, "stereo"};

    audio_queue<int16_t> mixer(out_ctx);

    std::vector<int16_t> input_data = { 32767, -32768, 10000, -10000 }; // 2 frames, stereo
    REQUIRE(mixer.push_audio(in_ctx, input_data.data(), 2));
}

TEST_CASE("AudioMixer resample", "[audio_queue]") 
{
    audio_ctx in_ctx{sample_rate::SR48000, "stereo"};
    audio_ctx out_ctx{sample_rate::SR96000, "stereo"};
    audio_queue<int16_t> mixer(out_ctx);

    std::vector<int16_t> input_data(480); // 240 stereo frames of silence
    REQUIRE(mixer.push_audio(in_ctx, input_data.data(), 240));
}

TEST_CASE("AudioMixer mono to stereo conversion", "[audio_queue]") 
{
    audio_ctx in_ctx{sample_rate::SR44100, "mono"};
    audio_ctx out_ctx{sample_rate::SR44100, "stereo"};
    audio_queue<int16_t> mixer(out_ctx);

    std::vector<int16_t> input_data = { 32767, -32768 }; // 2 frames mono
    REQUIRE(mixer.push_audio(in_ctx, input_data.data(), 2));
}
