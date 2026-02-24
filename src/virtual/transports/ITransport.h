#pragma once

#include <concepts>
#include <cstdint>
#include <cstddef>
#include <span>

namespace npb
{

    struct TransportTag
    {
    };
    struct ClockDataTransportTag : public TransportTag
    {
    };

    struct SelfClockingTransportTag : public TransportTag
    {
    };

    class ITransport
    {
    public:
        virtual ~ITransport() = default;

        virtual void begin() = 0;

        virtual void beginTransaction()
        {
        }

        virtual void transmitBytes(std::span<const uint8_t> data) = 0;

        virtual void endTransaction()
        {
        }

        virtual bool isReadyToUpdate() const
        {
            return true;
        }
    };

    template <typename TTransport>
    concept TransportLike = std::derived_from<TTransport, ITransport> &&
                            requires {
                                typename TTransport::TransportCategory;
                                typename TTransport::TransportConfigType;
                            };

    template <typename TTransport, typename TTag>
    concept TaggedTransportLike = TransportLike<TTransport> &&
                                  std::same_as<typename TTransport::TransportCategory, TTag>;

} // namespace npb
