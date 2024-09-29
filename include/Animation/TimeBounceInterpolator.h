#pragma once

#include "TimeInterpolator/TimeFloatInterpolator.h"

class TimeBounceInterpolator
{
    TimeFloatInterpolator _interpolator;
    double                _original;
    double                _target{};
    float                 _duration{};
    std::function<void()> _interpolateBackToOriginal;
    bool                  _isInterpolatingBackToOriginal = false;

public:
     TimeBounceInterpolator() = delete;
    ~TimeBounceInterpolator() = default;

    explicit TimeBounceInterpolator(double originalValue);

    void InterpolateTo(double target, double duration);

    [[nodiscard]] double GetValue() const;

    void ForceFinish();

    void SetValue(double value);

    void ForceValue(double value);
};
