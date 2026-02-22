#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>
#include <vector>

#include <Print.h>

#include "IEmitPixels.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "ColorOrderTransform.h"

namespace npb
{

    struct PrintEmitterSettings
    {
        Print& output;
        ColorOrderTransformConfig colorConfig;
    };

    class PrintEmitter : public IEmitPixels
    {
    public:
        PrintEmitter(uint16_t pixelCount,
                     ResourceHandle<IShader> shader,
                     PrintEmitterSettings settings)
            : _output{settings.output}
            , _shader{std::move(shader)}
            , _transform{settings.colorConfig}
            , _scratchColors(pixelCount)
            , _byteBuffer(_transform.bytesNeeded(pixelCount))
        {
        }

        void initialize() override
        {
            // no-op — no hardware to set up
        }

        void update(std::span<const Color> colors) override
        {
            static constexpr char HexDigits[] = "0123456789ABCDEF";

            // Apply shaders in batch
            std::span<const Color> source = colors;
            if (nullptr != _shader)
            {
                std::copy(colors.begin(), colors.end(), _scratchColors.begin());
                _shader->apply(_scratchColors);
                source = _scratchColors;
            }

            // Transform all pixels to bytes in one batch call
            _transform.apply(_byteBuffer, source);

            // Print each pixel's bytes as hex
            const size_t bpp = _transform.bytesNeeded(1);
            for (size_t i = 0; i < source.size(); ++i)
            {
                size_t offset = i * bpp;
                for (size_t b = 0; b < bpp; ++b)
                {
                    _output.print(HexDigits[_byteBuffer[offset + b] >> 4]);
                    _output.print(HexDigits[_byteBuffer[offset + b] & 0x0F]);
                }
                _output.print(' ');
            }
            _output.println();
        }

        bool isReadyToUpdate() const override
        {
            return true;
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

    private:
        Print& _output;
        ResourceHandle<IShader> _shader;
        ColorOrderTransform _transform;
        std::vector<Color> _scratchColors;       // pre-allocated at construction
        std::vector<uint8_t> _byteBuffer;        // pre-allocated at construction
    };

} // namespace npb
