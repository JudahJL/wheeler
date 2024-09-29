#pragma once

#include "TimeFloatInterpolator.h"

class TimeFloatInterpolator;

/**
 * Manages all instances of TimeFloatInterpolator. Constructor and destructor of 
 * TimeFloatInterpolator handles registering and unregistering itself from this class.
 * This class updates all instances of TimeFloatInterpolator every frame.
 * 
 * For this class to work, Update() function must be called every frame by another loop update function.
 * 
 */
class TimeFloatInterpolatorManager
{
    static inline std::unordered_set<TimeFloatInterpolator*> interpolators;
	static inline std::mutex mutex;

public:
	static void RegisterInterpolator(TimeFloatInterpolator* interpolator);

	static void UnregisterInterpolator(TimeFloatInterpolator* interpolator);

	static void Update(float dt);
};

