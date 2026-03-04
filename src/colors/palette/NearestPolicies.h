#pragma once

#include <cstddef>

namespace lw
{
    struct NearestTieStable
    {
        static constexpr bool pickCandidate(size_t,
                                            size_t)
        {
            return false;
        }
    };

    struct NearestTieLeft
    {
        static constexpr bool pickCandidate(size_t candidateIndex,
                                            size_t currentIndex)
        {
            return candidateIndex < currentIndex;
        }
    };

    struct NearestTieRight
    {
        static constexpr bool pickCandidate(size_t candidateIndex,
                                            size_t currentIndex)
        {
            return candidateIndex > currentIndex;
        }
    };

} // namespace lw
