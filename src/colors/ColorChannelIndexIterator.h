#pragma once

#include <cstddef>
#include <iterator>

#if __cplusplus >= 202002L
#include <compare>
#endif

namespace npb
{

    template <size_t NChannels>
    class ColorChannelIndexIterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using reference = value_type;

#if __cplusplus >= 202002L
        using iterator_concept = std::random_access_iterator_tag;
#endif

        constexpr ColorChannelIndexIterator() = default;

        explicit constexpr ColorChannelIndexIterator(size_t position)
            : _position(position)
        {
        }

        constexpr value_type operator*() const
        {
            return channelAt(_position);
        }

        constexpr value_type operator[](difference_type n) const
        {
            return channelAt(static_cast<size_t>(static_cast<difference_type>(_position) + n));
        }

        constexpr ColorChannelIndexIterator &operator++()
        {
            ++_position;
            return *this;
        }

        constexpr ColorChannelIndexIterator operator++(int)
        {
            auto tmp = *this;
            ++_position;
            return tmp;
        }

        constexpr ColorChannelIndexIterator &operator--()
        {
            --_position;
            return *this;
        }

        constexpr ColorChannelIndexIterator operator--(int)
        {
            auto tmp = *this;
            --_position;
            return tmp;
        }

        constexpr ColorChannelIndexIterator &operator+=(difference_type n)
        {
            _position = static_cast<size_t>(static_cast<difference_type>(_position) + n);
            return *this;
        }

        constexpr ColorChannelIndexIterator &operator-=(difference_type n)
        {
            _position = static_cast<size_t>(static_cast<difference_type>(_position) - n);
            return *this;
        }

        friend constexpr ColorChannelIndexIterator operator+(ColorChannelIndexIterator it, difference_type n)
        {
            it += n;
            return it;
        }

        friend constexpr ColorChannelIndexIterator operator+(difference_type n, ColorChannelIndexIterator it)
        {
            it += n;
            return it;
        }

        friend constexpr ColorChannelIndexIterator operator-(ColorChannelIndexIterator it, difference_type n)
        {
            it -= n;
            return it;
        }

        friend constexpr difference_type operator-(const ColorChannelIndexIterator &a,
                                                   const ColorChannelIndexIterator &b)
        {
            return static_cast<difference_type>(a._position) -
                   static_cast<difference_type>(b._position);
        }

        friend constexpr bool operator==(const ColorChannelIndexIterator &a,
                                         const ColorChannelIndexIterator &b)
        {
            return a._position == b._position;
        }

#if __cplusplus >= 202002L
        friend constexpr auto operator<=>(const ColorChannelIndexIterator &a,
                                          const ColorChannelIndexIterator &b)
        {
            return a._position <=> b._position;
        }
#else
        friend constexpr bool operator!=(const ColorChannelIndexIterator &a,
                                         const ColorChannelIndexIterator &b)
        {
            return !(a == b);
        }

        friend constexpr bool operator<(const ColorChannelIndexIterator &a,
                                        const ColorChannelIndexIterator &b)
        {
            return a._position < b._position;
        }

        friend constexpr bool operator<=(const ColorChannelIndexIterator &a,
                                         const ColorChannelIndexIterator &b)
        {
            return a._position <= b._position;
        }

        friend constexpr bool operator>(const ColorChannelIndexIterator &a,
                                        const ColorChannelIndexIterator &b)
        {
            return a._position > b._position;
        }

        friend constexpr bool operator>=(const ColorChannelIndexIterator &a,
                                         const ColorChannelIndexIterator &b)
        {
            return a._position >= b._position;
        }
#endif

        constexpr size_t position() const
        {
            return _position;
        }

        static constexpr size_t channelCount()
        {
            return (NChannels <= 5) ? NChannels : 5;
        }

        static constexpr value_type channelAt(size_t channelIndex)
        {
            switch (channelIndex)
            {
            case 0:
                return 'R';
            case 1:
                return 'G';
            case 2:
                return 'B';
            case 3:
                return (NChannels >= 4) ? 'W' : '\0';
            case 4:
                return (NChannels >= 5) ? 'C' : '\0';
            default:
                return '\0';
            }
        }

    private:
        size_t _position{0};
    };

    template <size_t NChannels>
    class ColorChannelIndexRange
    {
    public:
        using Iterator = ColorChannelIndexIterator<NChannels>;

        static constexpr size_t indexFromChannel(char channel)
        {
            switch (channel)
            {
            case 'R':
            case 'r':
                return 0;

            case 'G':
            case 'g':
                return 1;

            case 'B':
            case 'b':
                return 2;

            case 'W':
            case 'w':
                return (NChannels > 3) ? 3 : 0;

            case 'C':
            case 'c':
                return (NChannels > 4) ? 4 : 0;

            default:
                return 0;
            }
        }

        static constexpr bool isSupportedChannelTag(char channel)
        {
            switch (channel)
            {
            case 'R':
            case 'r':
            case 'G':
            case 'g':
            case 'B':
            case 'b':
                return true;

            case 'W':
            case 'w':
                return NChannels >= 4;

            case 'C':
            case 'c':
                return NChannels >= 5;

            default:
                return false;
            }
        }

        constexpr Iterator begin() const
        {
            return Iterator{0};
        }

        constexpr Iterator end() const
        {
            return Iterator{Iterator::channelCount()};
        }

        static constexpr size_t size()
        {
            return Iterator::channelCount();
        }
    };

    template <size_t NChannels>
    constexpr ColorChannelIndexRange<NChannels> makeColorChannelIndexRange()
    {
        return {};
    }

} // namespace npb
