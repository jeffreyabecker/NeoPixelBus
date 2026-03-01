#pragma once

namespace lw
{
    class HueBlendBase
    {
    protected:
        static constexpr float fixWrap(float value)
        {
            if (value < 0.0f)
            {
                value += 1.0f;
            }
            else if (value > 1.0f)
            {
                value -= 1.0f;
            }

            return value;
        }
    };

    class HueBlendShortestDistance : HueBlendBase
    {
    public:
        static constexpr float HueBlend(float left, float right, float progress)
        {
            float delta = right - left;
            float base = left;
            if (delta > 0.5f)
            {
                base = right;
                delta = 1.0f - delta;
                progress = 1.0f - progress;
            }
            else if (delta < -0.5f)
            {
                delta = 1.0f + delta;
            }

            return fixWrap(base + (delta * progress));
        }
    };

    class HueBlendLongestDistance : HueBlendBase
    {
    public:
        static constexpr float HueBlend(float left, float right, float progress)
        {
            float delta = right - left;
            float base = left;
            if (delta < 0.5f && delta >= 0.0f)
            {
                base = right;
                delta = 1.0f - delta;
                progress = 1.0f - progress;
            }
            else if (delta > -0.5f && delta < 0.0f)
            {
                delta = 1.0f + delta;
            }

            return fixWrap(base + (delta * progress));
        }
    };

    class HueBlendClockwiseDirection : HueBlendBase
    {
    public:
        static constexpr float HueBlend(float left, float right, float progress)
        {
            float delta = right - left;
            if (delta < 0.0f)
            {
                delta = 1.0f + delta;
            }

            return fixWrap(left + (delta * progress));
        }
    };

    class HueBlendCounterClockwiseDirection : HueBlendBase
    {
    public:
        static constexpr float HueBlend(float left, float right, float progress)
        {
            float delta = right - left;
            if (delta > 0.0f)
            {
                delta -= 1.0f;
            }

            return fixWrap(left + (delta * progress));
        }
    };
}

