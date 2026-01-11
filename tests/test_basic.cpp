#include "audio_queue.h"

#include <catch2/catch_all.hpp>

#include <iostream>
#include <numeric>
#include <vector>

using Catch::Matchers::WithinAbs;

// Utility: generate a simple ramp waveform for predictable verification
template <typename T>
std::vector<T> generate_ramp(
    size_t frame_count,
    size_t channels,
    T      start = T(0),
    T      step  = T(1))
{
    std::vector<T> data(frame_count * channels);
    std::iota(data.begin(), data.end(), start);
    for (auto& v : data) v *= step;
    return data;
}

// Utility: compute RMS difference between two buffers
template <typename T>
float rms_diff(
    const std::vector<T>& a,
    const std::vector<T>& b)
{
    REQUIRE(a.size() == b.size());
    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i)
    {
        float diff  = static_cast<float>(a[i]) - static_cast<float>(b[i]);
        sum        += diff * diff;
    }
    return std::sqrt(sum / static_cast<float>(a.size()));
}

TEST_CASE(
    "audio_queue basic push/pop (float)",
    "[audio_queue]")
{
    audio_context      ctx{sample_rate::SR48000, "Stereo"};
    audio_queue<float> q(ctx);

    auto input = generate_ramp<float>(256, ctx.m_channel_num, 0.0f, 0.001f);
    REQUIRE(q.push_audio(ctx, input.data(), 256));

    std::vector<float> output(256 * ctx.m_channel_num, 0.0f);
    bool               full = q.pop_audio(ctx, output.data(), 256);

    REQUIRE(output.size() == input.size());
    REQUIRE(rms_diff(input, output) < 1e-6f);
}

TEST_CASE(
    "audio_queue push/pop (int16_t)",
    "[audio_queue]")
{
    audio_context        ctx{sample_rate::SR44100, "Mono"};
    audio_queue<int16_t> q(ctx);
    auto                 input = generate_ramp<int16_t>(256, 1, 0, 10);
    REQUIRE(q.push_audio(ctx, input.data(), 256));

    std::vector<int16_t> output(256, 0);
    bool                 full = q.pop_audio(ctx, output.data(), 256);

    REQUIRE(output.size() == input.size());
    REQUIRE(rms_diff(input, output) < 2.0f);
}

TEST_CASE(
    "audio_queue pop mixes instead of overwriting",
    "[audio_queue]")
{
    audio_context      ctx{sample_rate::SR48000, "Stereo"};
    audio_queue<float> q(ctx);

    // Push only half of required samples
    auto input = generate_ramp<float>(64, ctx.m_channel_num, 0.0f, 0.001f);
    REQUIRE(q.push_audio(ctx, input.data(), 64));

    std::vector<float> output(128 * ctx.m_channel_num, 0.1f);
    bool               full = q.pop_audio(ctx, output.data(), 128);

    // Still considered "not full" since we popped less than required
    REQUIRE_FALSE(full);

    // First half should be mixed and clamped, second half unchanged
    for (size_t i = 0; i < input.size(); ++i)
    {
        float expected = std::clamp(0.1f + input[i], -1.0f, 1.0f);
        REQUIRE_THAT(output[i], WithinAbs(expected, 1e-5f));
    }

    for (size_t i = input.size(); i < output.size(); ++i) REQUIRE_THAT(output[i], WithinAbs(0.1f, 1e-5f));
}

TEST_CASE(
    "audio_queue mixing behavior clamps output",
    "[audio_queue]")
{
    audio_context      ctx{sample_rate::SR48000, "Stereo"};
    audio_queue<float> q(ctx);

    auto input = generate_ramp<float>(64, 2, 0.0f, 0.02f);
    REQUIRE(q.push_audio(ctx, input.data(), 64));

    std::vector<float> output(64 * 2, 0.5f);
    q.pop_audio(ctx, output.data(), 64);

    for (auto s : output) REQUIRE(s <= 1.0f);
}

TEST_CASE(
    "audio_queue round-trip conversion precision",
    "[audio_queue]")
{
    audio_context ctx{sample_rate::SR48000, "Mono"};

    SECTION("int16_t <-> float conversion")
    {
        auto [to_float, from_float] = make_audio_converters<int16_t>();
        int16_t src                 = 16384;
        float   f                   = to_float(src);
        auto    back                = from_float(f);

        std::cout << "src: " << src << '\n';
        std::cout << "to float: " << f << '\n';
        std::cout << "back: " << back << '\n';
        std::cout << "difference: " << std::abs(src - back) << '\n';
        REQUIRE(std::abs(src - back) <= 2);
    }

    SECTION("uint8_t <-> float conversion")
    {
        auto [to_float, from_float] = make_audio_converters<uint8_t>();
        uint8_t src                 = 200;
        float   f                   = to_float(src);
        auto    back                = from_float(f);
        REQUIRE(std::abs(src - back) <= 2);
    }
}