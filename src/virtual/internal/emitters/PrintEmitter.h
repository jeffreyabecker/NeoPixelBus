#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>

#include <Print.h>

#include "IEmitPixels.h"
#include "../shaders/IShader.h"
#include "ColorOrderTransform.h"
namespace npb
{

    class PrintEmitter : public IEmitPixels
    {
    public:
        PrintEmitter(Print &output, std::unique_ptr<IShader> shader, npb::ColorOrderTransformConfig config)
            : _output{output}, _shader{std::move(shader)}, _transform{config}
        {
        }

        void initialize() override
        {
            // no-op — no hardware to set up
        }

        void update(std::span<const Color> colors) override
        {
            static constexpr char HexDigits[] = "0123456789ABCDEF";

            if (_shader)
            {
                _shader->begin(colors);
            }

            for (uint16_t i = 0; i < colors.size(); ++i)
            {
                Color shaded = _shader ? _shader->apply(i, colors[i]) : colors[i];
                _transform.apply(std::span<uint8_t>(_singlePixelBuffer), std::span<const Color>(&shaded, 1));
                // WS2812 
                _output.print(HexDigits[_singlePixelBuffer[0] >> 4]);
                _output.print(HexDigits[_singlePixelBuffer[0] & 0x0F]);
                _output.print(HexDigits[_singlePixelBuffer[1] >> 4]);
                _output.print(HexDigits[_singlePixelBuffer[1] & 0x0F]);
                _output.print(HexDigits[_singlePixelBuffer[2] >> 4]);
                _output.print(HexDigits[_singlePixelBuffer[2] & 0x0F]);
                _output.print(HexDigits[_singlePixelBuffer[3] >> 4]);
                _output.print(HexDigits[_singlePixelBuffer[3] & 0x0F]);
                _output.print(HexDigits[_singlePixelBuffer[4] >> 4]);
                _output.print(HexDigits[_singlePixelBuffer[4] & 0x0F]);
                _output.print(' ');
            }
            _output.println();
            if (_shader)
            {
                _shader->end();
            }
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
        Print &_output;
        std::unique_ptr<IShader> _shader;
        ColorOrderTransform _transform;
        std::array<uint8_t, Color::ChannelCount> _singlePixelBuffer{0, 0, 0, 0, 0}; // default Black
    };

} // namespace npb
