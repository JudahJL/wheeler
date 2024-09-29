#pragma once
#include "TimeInterpolator/TimeFloatInterpolator.h"

class TimeTrapezoidInterpolator
{
    TimeFloatInterpolator point1Interpolator;
    TimeFloatInterpolator point2Interpolator;
    TimeFloatInterpolator point3Interpolator;
    TimeFloatInterpolator point4Interpolator;

public:
    TimeTrapezoidInterpolator() = default;

    TimeTrapezoidInterpolator(const float initialPoint1, const float initialPoint2, const float initialPoint3, const float initialPoint4):
        point1Interpolator(initialPoint1),
        point2Interpolator(initialPoint2),
        point3Interpolator(initialPoint3),
        point4Interpolator(initialPoint4) { }

    void InterpolateTo(float targetPoint1, float targetPoint2, float targetPoint3, float targetPoint4, float duration);

    [[nodiscard]] float GetPoint1() const;

    [[nodiscard]] float GetPoint2() const;

    [[nodiscard]] float GetPoint3() const;

    [[nodiscard]] float GetPoint4() const;
};
