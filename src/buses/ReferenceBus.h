#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "colors/IShader.h"
#include "core/IPixelBus.h"
#include "protocols/IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

    template <typename TColor>
    class ReferenceBus : public IPixelBus<TColor>
    {
    public:
        ReferenceBus(PixelCount pixelCount,
                     TColor *rootBuffer,
                     IProtocol<TColor> *protocol,
                     uint8_t *protocolBuffer,
                     ITransport *transport,
                     IShader<TColor> *shader,
                     TColor *shaderBuffer,
                     bool owns)
            : _pixelCount(pixelCount)
            , _rootBuffer(rootBuffer)
            , _protocol(protocol)
            , _protocolBuffer(protocolBuffer)
            , _transport(transport)
            , _shader(shader)
            , _shaderBuffer(shaderBuffer)
            , _pixelViewChunks{makePixelChunk(_rootBuffer, _pixelCount)}
            , _pixels(span<span<TColor>>{_pixelViewChunks.data(), _pixelViewChunks.size()})
            , _owns(owns)
        {
        }

        ~ReferenceBus() override
        {
            if (!_owns)
            {
                return;
            }

            delete _protocol;
            delete _transport;
            delete _shader;
            delete[] _rootBuffer;
            delete[] _protocolBuffer;
            delete[] _shaderBuffer;
        }

        void begin() override
        {
            if (_transport != nullptr)
            {
                _transport->begin();
            }

            if (_protocol != nullptr)
            {
                _protocol->begin();
            }
        }

        void show() override
        {
            if (_protocol == nullptr || _transport == nullptr)
            {
                return;
            }

            if (!_dirty && !_protocol->alwaysUpdate())
            {
                return;
            }

            if (!_transport->isReadyToUpdate())
            {
                return;
            }

            span<const TColor> protocolInput{};
            if (_rootBuffer != nullptr && _pixelCount > 0)
            {
                if (_shader != nullptr && _shaderBuffer != nullptr)
                {
                    std::copy_n(_rootBuffer, _pixelCount, _shaderBuffer);
                    span<TColor> shaderSpan{_shaderBuffer, _pixelCount};
                    _shader->apply(shaderSpan);
                    protocolInput = shaderSpan;
                }
                else if (_shader != nullptr)
                {
                    span<TColor> rootSpan{_rootBuffer, _pixelCount};
                    _shader->apply(rootSpan);
                    protocolInput = rootSpan;
                }
                else
                {
                    protocolInput = span<const TColor>{_rootBuffer, _pixelCount};
                }
            }

            span<uint8_t> protocolBytes{};
            const size_t requiredSize = _protocol->requiredBufferSizeBytes();
            if (_protocolBuffer != nullptr && requiredSize > 0)
            {
                protocolBytes = span<uint8_t>{_protocolBuffer, requiredSize};
            }

            _protocol->update(protocolInput, protocolBytes);

            if (!protocolBytes.empty())
            {
                _transport->beginTransaction();
                _transport->transmitBytes(protocolBytes);
                _transport->endTransaction();
            }

            _dirty = false;
        }

        bool isReadyToUpdate() const override
        {
            if (_transport == nullptr)
            {
                return false;
            }

            return _transport->isReadyToUpdate();
        }

        PixelView<TColor> &pixels() override
        {
            _dirty = true;
            return _pixels;
        }

        const PixelView<TColor> &pixels() const override
        {
            return _pixels;
        }

        PixelCount pixelCount() const
        {
            return _pixelCount;
        }

        IProtocol<TColor> *protocol()
        {
            return _protocol;
        }

        const IProtocol<TColor> *protocol() const
        {
            return _protocol;
        }

        ITransport *transport()
        {
            return _transport;
        }

        const ITransport *transport() const
        {
            return _transport;
        }

        IShader<TColor> *shader()
        {
            return _shader;
        }

        const IShader<TColor> *shader() const
        {
            return _shader;
        }

    private:
        static span<TColor> makePixelChunk(TColor *buffer,
                                           PixelCount pixelCount)
        {
            if (buffer == nullptr || pixelCount == 0)
            {
                return span<TColor>{};
            }

            return span<TColor>{buffer, pixelCount};
        }

        PixelCount _pixelCount{0};
        TColor *_rootBuffer{nullptr};
        IProtocol<TColor> *_protocol{nullptr};
        uint8_t *_protocolBuffer{nullptr};
        ITransport *_transport{nullptr};
        IShader<TColor> *_shader{nullptr};
        TColor *_shaderBuffer{nullptr};
        std::array<span<TColor>, 1> _pixelViewChunks;
        PixelView<TColor> _pixels;
        bool _owns{false};
        bool _dirty{true};
    };

} // namespace lw
