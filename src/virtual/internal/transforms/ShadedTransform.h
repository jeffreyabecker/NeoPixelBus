#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <array>
#include <algorithm>

#include "../colors/Color.h"
#include "ITransformColorToBytes.h"
#include "IShader.h"

namespace npb
{

#ifndef NEOPIXELBUS_SHADED_TRANSFORM_SCRATCH_SIZE
#define NEOPIXELBUS_SHADED_TRANSFORM_SCRATCH_SIZE 32
#endif
static constexpr size_t DefaultShadedTransformScratchSize = NEOPIXELBUS_SHADED_TRANSFORM_SCRATCH_SIZE;

template<size_t V_SCRATCH_SIZE = DefaultShadedTransformScratchSize>
class ShadedTransform : public ITransformColorToBytes
{
public:
    ShadedTransform(ITransformColorToBytes& inner,
                    std::span<IShader* const> shaders)
        : _inner{inner}
        , _shaders{shaders}
    {
    }

    void apply(std::span<uint8_t> pixels,
               std::span<const Color> colors) override
    {
        // Zero-copy passthrough when no shaders are attached
        if (_shaders.empty())
        {
            _inner.apply(pixels, colors);
            return;
        }

        // Process in batches through a stack-allocated scratch buffer
        // to avoid mutating the caller's color data
        size_t remaining = colors.size();
        size_t srcOffset = 0;
        size_t dstOffset = 0;
        const size_t bytesPerPixel = _inner.bytesNeeded(1);

        while (remaining > 0)
        {
            const size_t batchSize = std::min(remaining, V_SCRATCH_SIZE);

            // Copy source colors into scratch buffer
            std::copy_n(colors.begin() + srcOffset, batchSize, _scratch.begin());

            // Apply shader chain in order
            std::span<Color> batchSpan(_scratch.data(), batchSize);
            for (auto* shader : _shaders)
            {
                shader->apply(batchSpan);
            }

            // Forward shaded batch to inner transform
            const size_t batchBytes = bytesPerPixel * batchSize;
            _inner.apply(
                pixels.subspan(dstOffset, batchBytes),
                batchSpan
            );

            srcOffset += batchSize;
            dstOffset += batchBytes;
            remaining -= batchSize;
        }
    }

    size_t bytesNeeded(size_t pixelCount) const override
    {
        return _inner.bytesNeeded(pixelCount);
    }

private:
    ITransformColorToBytes& _inner;
    std::span<IShader* const> _shaders;

    std::array<Color, V_SCRATCH_SIZE> _scratch;
};

} // namespace npb
