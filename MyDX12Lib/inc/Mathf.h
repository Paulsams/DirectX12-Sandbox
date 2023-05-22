#pragma once
#include <algorithm>
#include <cmath>

namespace math
{
    inline float Repeat(const float t, const float length)
    {
        return std::clamp(t - std::floor(t / length) * length, 0.0f, length);
    }

    inline float PingPong(float t, const float length)
    {
        t = Repeat(t, length * 2.0f);
        return length - std::abs(t - length);
    }
}
