#pragma once
using EventResult = RE::BSEventNotifyControl;

class ModCallbackEventHandler final: public RE::BSTEventSink<SKSE::ModCallbackEvent>
{
public:
    EventResult ProcessEvent(const SKSE::ModCallbackEvent* a_event, RE::BSTEventSource<SKSE::ModCallbackEvent>*) override;

	static bool Register();
};
