#include "Animation/TimeInterpolator/TimeFloatInterpolator.h"
#include "Animation/TimeInterpolator/TimeInterpolatorManager.h"

TimeFloatInterpolator::~TimeFloatInterpolator()
{
	TimeFloatInterpolatorManager::UnregisterInterpolator(this);
}

void TimeFloatInterpolator::InterpolateTo(const double targetValue, const double interpolDuration)
{
	target = targetValue;
	duration = interpolDuration;
	elapsedTime = 0.0f;
	TimeFloatInterpolatorManager::RegisterInterpolator(this);
}

void TimeFloatInterpolator::PushCallback(const std::function<void()>& callback)
{
	this->_callbacks.push_back(callback);
}

bool TimeFloatInterpolator::Update(const double dt)
{
	if (elapsedTime < duration) {
		elapsedTime   += dt;
        const double t = std::min(elapsedTime / duration, 1.0);
		value = value + (target - value) * t;
		if (elapsedTime >= duration) { // we're done
			for (auto& callback : this->_callbacks) {
				// we use std::jthread here because without it, the callback could try to call this* InterpolateTo(), which leads to a lock race in TimeFloatInterpolatorManager.
				std::jthread(callback).detach();
			}
			return true;
		}
	}
	return false;
}

double TimeFloatInterpolator::GetValue() const
{
	return value;
}

void TimeFloatInterpolator::ForceFinish(const bool wantCallback)
{
	this->elapsedTime.store(this->duration);
	this->value.store(this->target);
	if (wantCallback) {
		for (auto& callback : this->_callbacks) {
			std::jthread(callback).detach();
		}
	}
	TimeFloatInterpolatorManager::UnregisterInterpolator(this);
}

void TimeFloatInterpolator::SetValue(const double value)
{
	this->value = value;
}

void TimeFloatInterpolator::ForceValue(const double value)
{
	this->target = value;
	this->value = value;
	this->duration = 0;
	this->elapsedTime = 0;
}
