#include <unity.h>

#include <array>

#include "colors/palette/PaletteCodec.h"

namespace
{
    using Stop = lw::PaletteStop<lw::Rgb8Color>;

    lw::Palette<lw::Rgb8Color> makeThreeStopPalette(const std::array<Stop, 3> &stops)
    {
        return lw::Palette<lw::Rgb8Color>(lw::span<const Stop>(stops.data(), stops.size()));
    }

    void test_7_3_1_binary_round_trip_rgb8(void)
    {
        const std::array<Stop, 3> srcStops = {
            Stop{0, lw::Rgb8Color(1, 2, 3)},
            Stop{64, lw::Rgb8Color(10, 20, 30)},
            Stop{255, lw::Rgb8Color(200, 210, 220)}};

        const auto srcPalette = makeThreeStopPalette(srcStops);

        std::array<uint8_t, 64> encoded{};
        size_t written = 0;
        const auto encodeErr = lw::encodePaletteBinary(srcPalette, lw::span<uint8_t>(encoded.data(), encoded.size()), written);

        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::PaletteCodecError::None), static_cast<int>(encodeErr));
        TEST_ASSERT_EQUAL_UINT32(23, static_cast<uint32_t>(written));

        std::array<Stop, 8> decodedStops{};
        lw::Palette<lw::Rgb8Color> decodedPalette;
        size_t decodedCount = 0;

        const auto decodeErr = lw::decodePaletteBinary(
            lw::span<const uint8_t>(encoded.data(), written),
            lw::span<Stop>(decodedStops.data(), decodedStops.size()),
            decodedPalette,
            decodedCount);

        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::PaletteCodecError::None), static_cast<int>(decodeErr));
        TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(decodedCount));
        TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(decodedPalette.size()));

        const auto roundTripStops = decodedPalette.stops();
        for (size_t i = 0; i < srcStops.size(); ++i)
        {
            TEST_ASSERT_EQUAL_UINT8(srcStops[i].index, roundTripStops[i].index);
            TEST_ASSERT_EQUAL_UINT8(srcStops[i].color['R'], roundTripStops[i].color['R']);
            TEST_ASSERT_EQUAL_UINT8(srcStops[i].color['G'], roundTripStops[i].color['G']);
            TEST_ASSERT_EQUAL_UINT8(srcStops[i].color['B'], roundTripStops[i].color['B']);
        }
    }

    void test_7_3_2_decode_rejects_unsupported_version(void)
    {
        const std::array<Stop, 3> srcStops = {
            Stop{0, lw::Rgb8Color(1, 2, 3)},
            Stop{64, lw::Rgb8Color(10, 20, 30)},
            Stop{255, lw::Rgb8Color(200, 210, 220)}};

        const auto srcPalette = makeThreeStopPalette(srcStops);

        std::array<uint8_t, 64> encoded{};
        size_t written = 0;
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::PaletteCodecError::None),
                              static_cast<int>(lw::encodePaletteBinary(srcPalette, lw::span<uint8_t>(encoded.data(), encoded.size()), written)));

        encoded[4] = 0x02;
        encoded[5] = 0x00;

        std::array<Stop, 8> decodedStops{};
        lw::Palette<lw::Rgb8Color> decodedPalette;
        size_t decodedCount = 0;

        const auto decodeErr = lw::decodePaletteBinary(
            lw::span<const uint8_t>(encoded.data(), written),
            lw::span<Stop>(decodedStops.data(), decodedStops.size()),
            decodedPalette,
            decodedCount);

        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::PaletteCodecError::UnsupportedVersion), static_cast<int>(decodeErr));
    }

    void test_7_3_3_decode_rejects_length_mismatch(void)
    {
        const std::array<Stop, 3> srcStops = {
            Stop{0, lw::Rgb8Color(1, 2, 3)},
            Stop{64, lw::Rgb8Color(10, 20, 30)},
            Stop{255, lw::Rgb8Color(200, 210, 220)}};

        const auto srcPalette = makeThreeStopPalette(srcStops);

        std::array<uint8_t, 64> encoded{};
        size_t written = 0;
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::PaletteCodecError::None),
                              static_cast<int>(lw::encodePaletteBinary(srcPalette, lw::span<uint8_t>(encoded.data(), encoded.size()), written)));

        encoded[2] = static_cast<uint8_t>(encoded[2] - 1u);

        std::array<Stop, 8> decodedStops{};
        lw::Palette<lw::Rgb8Color> decodedPalette;
        size_t decodedCount = 0;

        const auto decodeErr = lw::decodePaletteBinary(
            lw::span<const uint8_t>(encoded.data(), written),
            lw::span<Stop>(decodedStops.data(), decodedStops.size()),
            decodedPalette,
            decodedCount);

        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::PaletteCodecError::InvalidLength), static_cast<int>(decodeErr));
    }

    void test_7_3_4_decode_rejects_checksum_mismatch(void)
    {
        const std::array<Stop, 3> srcStops = {
            Stop{0, lw::Rgb8Color(1, 2, 3)},
            Stop{64, lw::Rgb8Color(10, 20, 30)},
            Stop{255, lw::Rgb8Color(200, 210, 220)}};

        const auto srcPalette = makeThreeStopPalette(srcStops);

        std::array<uint8_t, 64> encoded{};
        size_t written = 0;
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::PaletteCodecError::None),
                              static_cast<int>(lw::encodePaletteBinary(srcPalette, lw::span<uint8_t>(encoded.data(), encoded.size()), written)));

        encoded[10] ^= 0x01u;

        std::array<Stop, 8> decodedStops{};
        lw::Palette<lw::Rgb8Color> decodedPalette;
        size_t decodedCount = 0;

        const auto decodeErr = lw::decodePaletteBinary(
            lw::span<const uint8_t>(encoded.data(), written),
            lw::span<Stop>(decodedStops.data(), decodedStops.size()),
            decodedPalette,
            decodedCount);

        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::PaletteCodecError::InvalidChecksum), static_cast<int>(decodeErr));
    }

    void test_7_3_5_encode_rejects_non_monotonic_stops(void)
    {
        const std::array<Stop, 3> invalidStops = {
            Stop{0, lw::Rgb8Color(1, 2, 3)},
            Stop{100, lw::Rgb8Color(10, 20, 30)},
            Stop{80, lw::Rgb8Color(200, 210, 220)}};

        const auto palette = makeThreeStopPalette(invalidStops);

        std::array<uint8_t, 64> encoded{};
        size_t written = 0;

        const auto err = lw::encodePaletteBinary(palette, lw::span<uint8_t>(encoded.data(), encoded.size()), written);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lw::PaletteCodecError::InvalidStopOrder), static_cast<int>(err));
    }
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();
    RUN_TEST(test_7_3_1_binary_round_trip_rgb8);
    RUN_TEST(test_7_3_2_decode_rejects_unsupported_version);
    RUN_TEST(test_7_3_3_decode_rejects_length_mismatch);
    RUN_TEST(test_7_3_4_decode_rejects_checksum_mismatch);
    RUN_TEST(test_7_3_5_encode_rejects_non_monotonic_stops);
    return UNITY_END();
}
