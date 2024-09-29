#include "Animation/TimeColorInterpolator.h"

void TimeColorInterpolator::InterpolateTo(const ImU32& targetColor, const float duration) {
    // assuming the targetColor is in ARGB format
    const uint32_t a{ targetColor >> IM_COL32_A_SHIFT & 0xFF };
    const uint32_t b{ targetColor >> IM_COL32_B_SHIFT & 0xFF };
    const uint32_t g{ targetColor >> IM_COL32_G_SHIFT & 0xFF };
    const uint32_t r{ targetColor >> IM_COL32_R_SHIFT & 0xFF };

    redInterpolator.InterpolateTo(r, duration);
    greenInterpolator.InterpolateTo(g, duration);
    blueInterpolator.InterpolateTo(b, duration);
    alphaInterpolator.InterpolateTo(a, duration);
}

uint32_t TimeColorInterpolator::GetRed() const {
    return redInterpolator.GetValue();
}

uint32_t TimeColorInterpolator::GetGreen() const {
    return greenInterpolator.GetValue();
}

uint32_t TimeColorInterpolator::GetBlue() const {
    return blueInterpolator.GetValue();
}

uint32_t TimeColorInterpolator::GetAlpha() const {
    return alphaInterpolator.GetValue();
}

uint32_t TimeColorInterpolator::GetColor() const {
    return IM_COL32(GetRed(), GetGreen(), GetBlue(), GetAlpha());
}
