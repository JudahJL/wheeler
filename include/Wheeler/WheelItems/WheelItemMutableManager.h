#pragma once

class WheelItemMutable;
/// <summary>
/// Tracker of all instances of WheelItemMutable through direct reference to their raw pointers.
/// This class should be dealt with extreme caution as WheelItemMutables are passed around as smart pointers.
/// Instances of WheelItemMutable are responsible for adding themselves to the manager right after initialization, and removing themselves on right before destruction.
/// </summary>
class WheelItemMutableManager final: public RE::BSTEventSink<RE::TESUniqueIDChangeEvent>
{
public:
	void Register()
	{
		auto events{ RE::ScriptEventSourceHolder::GetSingleton() };
		events->AddEventSink(this);
	}

	static WheelItemMutableManager* GetSingleton()
	{
		static WheelItemMutableManager singleton;
		return &singleton;
	}

	/// <summary>
	/// Track a mutable wheel item. Must be called following initialization of a WheelItemMutable
	/// </summary>
	void Track(WheelItemMutable* a_mutable);

	void UnTrack(const WheelItemMutable* a_mutable);

	/// <summary>
	/// Clear the tracked items. Right before reloading the wheels.
	/// </summary>
	void Clear();
	
private:
	std::vector<WheelItemMutable*> _mutables;
	
	using EventResult = RE::BSEventNotifyControl;
    EventResult ProcessEvent(const RE::TESUniqueIDChangeEvent* a_event, RE::BSTEventSource<RE::TESUniqueIDChangeEvent>* a_dispatcher) override;
	mutable std::shared_mutex _lock;
};
