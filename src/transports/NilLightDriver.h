#pragma once

#include "transports/ILightDriver.h"

namespace lw::transports
{

struct NilLightDriverSettings : LightDriverSettingsBase
{
    static NilLightDriverSettings normalize(NilLightDriverSettings settings) { return settings; }
};

template <typename TColor> class NilLightDriver : public ILightDriver<TColor>
{
  public:
    using ColorType = TColor;
    using LightDriverSettingsType = NilLightDriverSettings;

    explicit NilLightDriver(LightDriverSettingsType = {}) {}

    void begin() override {}

    bool isReadyToUpdate() const override { return true; }

    void write(const ColorType&) override {}
};

} // namespace lw::transports
