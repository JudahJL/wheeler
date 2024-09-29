#pragma once
#include <imgui.h>
#include "Animation/TimeInterpolator/TimeFloatInterpolator.h"

#include <cstdint>

/**
 * Interpolates ImU32 color value over time through 4 instances of TimeUintInterpolator.
 */
class TimeColorInterpolator
{
    TimeFloatInterpolator redInterpolator{};
	TimeFloatInterpolator greenInterpolator{};
	TimeFloatInterpolator blueInterpolator{};
	TimeFloatInterpolator alphaInterpolator{};

public:
	TimeColorInterpolator() = default;

    explicit TimeColorInterpolator(const ImU32& targetColor) :
    redInterpolator((targetColor >> IM_COL32_R_SHIFT) & 0xFF),
    greenInterpolator((targetColor >> IM_COL32_G_SHIFT) & 0xFF),
    blueInterpolator((targetColor >> IM_COL32_B_SHIFT) & 0xFF),
    alphaInterpolator((targetColor >> IM_COL32_A_SHIFT) & 0xFF) {}

	void InterpolateTo(const ImU32& targetColor, float duration);

	uint32_t GetRed() const;
	uint32_t GetGreen() const;
	uint32_t GetBlue() const;
	uint32_t GetAlpha() const;
	uint32_t GetColor() const;
};
