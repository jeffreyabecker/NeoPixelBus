#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <initializer_list>

#include "buses/ConcatBus.h"
#include "core/Compat.h"

namespace npb
{
namespace factory
{

    template <typename TBus>
    using BusColorType = decltype(_deduceBusColor(static_cast<npb::remove_cvref_t<TBus> *>(nullptr)));

    template <typename TColor,
              typename... TBuses>
    class StaticConcatBusT : public IPixelBus<TColor>
    {
    public:
        static_assert(ConcatBusCompatibleBuses<TColor, TBuses...>,
                      "All owned buses must be compatible with IPixelBus<TColor>");

        using OwnedTuple = std::tuple<npb::remove_cvref_t<TBuses>...>;

        explicit StaticConcatBusT(TBuses &&...buses)
            : _ownedBuses(std::forward<TBuses>(buses)...)
            , _busList(makeBusList(_ownedBuses))
            , _concat(std::vector<IPixelBus<TColor> *>{_busList})
        {
        }

        StaticConcatBusT(const StaticConcatBusT &) = delete;
        StaticConcatBusT &operator=(const StaticConcatBusT &) = delete;
        StaticConcatBusT(StaticConcatBusT &&) = delete;
        StaticConcatBusT &operator=(StaticConcatBusT &&) = delete;

        void begin() override
        {
            _concat.begin();
        }

        void show() override
        {
            _concat.show();
        }

        bool canShow() const override
        {
            return _concat.canShow();
        }

        size_t pixelCount() const override
        {
            return _concat.pixelCount();
        }

        void setPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
        {
            _concat.setPixelColors(offset, first, last);
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
        {
            _concat.getPixelColors(offset, first, last);
        }

    private:
        static std::vector<IPixelBus<TColor> *> makeBusList(OwnedTuple &ownedBuses)
        {
            std::vector<IPixelBus<TColor> *> buses{};
            buses.reserve(sizeof...(TBuses));
            std::apply(
                [&](auto &...bus)
                {
                    (buses.emplace_back(&bus), ...);
                },
                ownedBuses);
            return buses;
        }

        OwnedTuple _ownedBuses;
        std::vector<IPixelBus<TColor> *> _busList;
        ConcatBus<TColor> _concat;
    };

    template <typename TColor,
              typename... TBuses>
    class RootOwnedConcatBusT : public IPixelBus<TColor>
    {
    public:
        static_assert(ConcatBusCompatibleBuses<TColor, TBuses...>,
                      "All owned buses must be compatible with IPixelBus<TColor>");

        using OwnedTuple = std::tuple<npb::remove_cvref_t<TBuses>...>;

        explicit RootOwnedConcatBusT(std::initializer_list<uint16_t> segmentLengths,
                                     TBuses &&...buses)
            : _ownedBuses(std::forward<TBuses>(buses)...)
            , _busList(makeBusList(_ownedBuses))
        {
            initializeSegments(segmentLengths);
        }

        RootOwnedConcatBusT(const RootOwnedConcatBusT &) = delete;
        RootOwnedConcatBusT &operator=(const RootOwnedConcatBusT &) = delete;
        RootOwnedConcatBusT(RootOwnedConcatBusT &&) = delete;
        RootOwnedConcatBusT &operator=(RootOwnedConcatBusT &&) = delete;

        void begin() override
        {
            for (auto *bus : _busList)
            {
                if (bus != nullptr)
                {
                    bus->begin();
                }
            }
        }

        void show() override
        {
            if (!_valid)
            {
                return;
            }

            if (_dirty)
            {
                for (size_t index = 0; index < _busList.size(); ++index)
                {
                    auto *bus = _busList[index];
                    if (bus == nullptr)
                    {
                        continue;
                    }

                    auto offset = _segmentOffsets[index];
                    auto count = _segmentLengths[index];
                    span<const TColor> segment{_colors.data() + offset, count};
                    bus->setPixelColors(0, segment);
                }

                _dirty = false;
            }

            for (auto *bus : _busList)
            {
                if (bus != nullptr)
                {
                    bus->show();
                }
            }
        }

        bool canShow() const override
        {
            if (!_valid)
            {
                return false;
            }

            return std::all_of(_busList.begin(),
                               _busList.end(),
                               [](const IPixelBus<TColor> *bus)
                               {
                                   return bus != nullptr && bus->canShow();
                               });
        }

        size_t pixelCount() const override
        {
            return _colors.size();
        }

        span<TColor> pixelBuffer() override
        {
            return span<TColor>{_colors.data(), _colors.size()};
        }

        span<const TColor> pixelBuffer() const override
        {
            return span<const TColor>{_colors.data(), _colors.size()};
        }

        void setPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count = std::min(requested, available);

            auto src = first;
            auto dest = _colors.begin() + offset;
            for (std::ptrdiff_t index = 0; index < count; ++index, ++src, ++dest)
            {
                *dest = *src;
            }

            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count = std::min(requested, available);

            auto src = _colors.cbegin() + offset;
            auto dest = first;
            for (std::ptrdiff_t index = 0; index < count; ++index, ++src, ++dest)
            {
                *dest = *src;
            }
        }

        void setPixelColors(size_t offset,
                            span<const TColor> pixelData) override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = _colors.size() - offset;
            auto count = std::min(pixelData.size(), available);
            std::copy_n(pixelData.begin(), count, _colors.begin() + offset);
            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            span<TColor> pixelData) const override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = _colors.size() - offset;
            auto count = std::min(pixelData.size(), available);
            std::copy_n(_colors.cbegin() + offset, count, pixelData.begin());
        }

        void setPixelColor(size_t index, const TColor &color) override
        {
            if (index < _colors.size())
            {
                _colors[index] = color;
                _dirty = true;
            }
        }

        TColor getPixelColor(size_t index) const override
        {
            if (index < _colors.size())
            {
                return _colors[index];
            }

            return TColor{};
        }

    private:
        static std::vector<IPixelBus<TColor> *> makeBusList(OwnedTuple &ownedBuses)
        {
            std::vector<IPixelBus<TColor> *> buses{};
            buses.reserve(sizeof...(TBuses));
            std::apply(
                [&](auto &...bus)
                {
                    (buses.emplace_back(&bus), ...);
                },
                ownedBuses);
            return buses;
        }

        void initializeSegments(std::initializer_list<uint16_t> segmentLengths)
        {
            if (segmentLengths.size() != _busList.size())
            {
                _valid = false;
                return;
            }

            _segmentLengths.clear();
            _segmentOffsets.clear();
            _segmentLengths.reserve(segmentLengths.size());
            _segmentOffsets.reserve(segmentLengths.size());

            size_t running = 0;
            for (auto len : segmentLengths)
            {
                _segmentOffsets.push_back(running);
                auto count = static_cast<size_t>(len);
                _segmentLengths.push_back(count);
                running += count;
            }

            _colors.resize(running);
            _valid = true;
            _dirty = true;
        }

        OwnedTuple _ownedBuses;
        std::vector<IPixelBus<TColor> *> _busList;
        std::vector<size_t> _segmentOffsets;
        std::vector<size_t> _segmentLengths;
        std::vector<TColor> _colors;
        bool _valid{false};
        bool _dirty{false};
    };

    template <typename TFirstBus,
              typename... TOtherBuses,
              typename TColor = BusColorType<TFirstBus>,
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<TFirstBus> *, IPixelBus<TColor> *>::value &&
                                          std::conjunction<std::is_convertible<npb::remove_cvref_t<TOtherBuses> *, IPixelBus<TColor> *>...>::value>>
    auto makeStaticConcatBus(TFirstBus &&firstBus,
                             TOtherBuses &&...otherBuses)
        -> StaticConcatBusT<TColor, TFirstBus, TOtherBuses...>
    {
        return StaticConcatBusT<TColor, TFirstBus, TOtherBuses...>{
            std::forward<TFirstBus>(firstBus),
            std::forward<TOtherBuses>(otherBuses)...};
    }

    template <typename TFirstBus,
              typename... TOtherBuses,
              typename TColor = BusColorType<TFirstBus>,
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<TFirstBus> *, IPixelBus<TColor> *>::value &&
                                          std::conjunction<std::is_convertible<npb::remove_cvref_t<TOtherBuses> *, IPixelBus<TColor> *>...>::value>>
    auto makeRootOwnedConcatBus(std::initializer_list<uint16_t> segmentLengths,
                                TFirstBus &&firstBus,
                                TOtherBuses &&...otherBuses)
        -> RootOwnedConcatBusT<TColor, TFirstBus, TOtherBuses...>
    {
        return RootOwnedConcatBusT<TColor, TFirstBus, TOtherBuses...>{
            segmentLengths,
            std::forward<TFirstBus>(firstBus),
            std::forward<TOtherBuses>(otherBuses)...};
    }

} // namespace factory
} // namespace npb
