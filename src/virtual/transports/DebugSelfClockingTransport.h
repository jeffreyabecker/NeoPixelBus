#pragma once

#include <cstdint>
#include <utility>

#include <Print.h>

#include "IClockDataTransport.h"
#include "DebugClockDataTransport.h"
#include "EncodedClockDataSelfClockingTransport.h"

namespace npb
{

    struct DebugSelfClockingTransportConfig : EncodedClockDataSelfClockingTransportConfig<DebugClockDataTransportConfig>
    {
        DebugSelfClockingTransportConfig() = default;

        DebugSelfClockingTransportConfig(OneWireTiming timingConfig,
                                         uint32_t clockDataBitRate,
                                         bool manageTx,
                                         EncodedClockDataBitPattern pattern)
        {
            timing = timingConfig;
            clockDataBitRateHz = clockDataBitRate;
            manageTransaction = manageTx;
            bitPattern = pattern;
        }
    };

    class DebugSelfClockingTransport : public SelfClockingWrapperTransport<DebugClockDataTransport>
    {
    public:
        using Base = SelfClockingWrapperTransport<DebugClockDataTransport>;

        explicit DebugSelfClockingTransport(DebugSelfClockingTransportConfig config)
            : Base(std::move(config))
        {
        }


    };

} // namespace npb
