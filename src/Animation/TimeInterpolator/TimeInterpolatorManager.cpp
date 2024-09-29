#include "Animation/TimeInterpolator/TimeInterpolatorManager.h"

void TimeFloatInterpolatorManager::RegisterInterpolator(TimeFloatInterpolator* interpolator)
{
	const std::lock_guard lock(mutex);
	interpolators.insert(interpolator);
}

void TimeFloatInterpolatorManager::UnregisterInterpolator(TimeFloatInterpolator* interpolator)
{
	const std::lock_guard lock(mutex);
	interpolators.erase(interpolator);
}

void TimeFloatInterpolatorManager::Update(const float dt)
{
	const std::lock_guard lock(mutex);
	for (auto it{ interpolators.begin() }; it != interpolators.end();) {
		if ((*it)->Update(dt)) {
			it = interpolators.erase(it);  // interpolator is done, remove it
		} else {
			++it;
		}
	}
	/*ImGui::Begin("INTERPOLATOR DEBUGGING");
	ImGui::Text("%i interpolators present.", interpolators.size());
	ImGui::End();*/
}
