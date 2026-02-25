// #include <unity.h>

// #include <array>

// #include <ArduinoFake.h>

// #include "VirtualNeoPixelBus.h"

// namespace
// {


//     static_assert(npb::TaggedTransportLike<npb::DebugOneWireTransport, npb::OneWireTransportTag>);
//     static_assert(npb::SettingsConstructibleTransportLike<npb::DebugOneWireTransport>);
//     static_assert(npb::ProtocolPixelSettingsConstructible<npb::DebugProtocol<Rgbcw8Color>>);
//     static_assert(npb::ProtocolSettingsTransportBindable<npb::DebugProtocol<Rgbcw8Color>>);

//     void test_make_ws2812x_bus_with_debug_transport_and_aggregate_shaders_smoke(void)
//     {
//         using DebugTransport = npb::DebugOneWireTransport;
//             using 

//         auto bus = npb::factory::makeWs2812xBus<DebugTransport, Rgbcw8Color>(
//             8,
//             "GRBW",
//             npb::factory::makeAggregateShader(
//                 npb::factory::makeGammaShader<Rgbcw8Color>(),
//                 npb::factory::makeWhiteBalanceShader<Rgbcw8Color>(),
//                 npb::factory::makeCurrentLimiterShader<Rgbcw8Color>(npb::CurrentLimiterShaderSettings<Rgbcw8Color>{
//                     .maxMilliamps = 1000,
//                     .milliampsPerChannel = std::array<uint16_t, Rgbcw8Color::ChannelCount>{20, 20, 20, 20,20},
//                     .controllerMilliamps = 50,
//                     .standbyMilliampsPerPixel = 1,
//                     .rgbwDerating = true,
//                 })),
//             typename DebugTransport::TransportSettingsType{.output = nullptr, .invert = false});

//         bus.begin();
//         TEST_ASSERT_TRUE(bus.canShow());

//         bus.setPixelColor(0, Rgbcw8Color{10, 20, 30, 40, 50});
//         bus.setPixelColor(1, Rgbcw8Color{1, 2, 3, 4, 5});
//         bus.show();

//         TEST_ASSERT_EQUAL_UINT32(8U, static_cast<uint32_t>(bus.pixelCount()));
//     }
// }

// void setUp(void)
// {
//     ArduinoFakeReset();
// }

// void tearDown(void)
// {
// }

// int main(int argc, char **argv)
// {
//     (void)argc;
//     (void)argv;

//     UNITY_BEGIN();
//     RUN_TEST(test_make_ws2812x_bus_with_debug_transport_and_aggregate_shaders_smoke);
//     return UNITY_END();
// }
