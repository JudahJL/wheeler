#include "Wheeler/WheelItems/WheelItemMutableManager.h"
#include "Wheeler/WheelItems/WheelItemMutable.h"

using EventResult = RE::BSEventNotifyControl;

void WheelItemMutableManager::Track(WheelItemMutable* a_mutable) {
    std::unique_lock lock(this->_lock);
    this->_mutables.push_back(a_mutable);
}

void WheelItemMutableManager::UnTrack(const WheelItemMutable* a_mutable) {
    std::unique_lock lock(this->_lock);
    for(auto it = this->_mutables.begin(); it != this->_mutables.end(); ++it) {
        if(*it == a_mutable) {
            this->_mutables.erase(it);
            break;
        }
    }
}

void WheelItemMutableManager::Clear() {
    std::unique_lock lock(this->_lock);
    this->_mutables.clear();
}

EventResult WheelItemMutableManager::ProcessEvent(const RE::TESUniqueIDChangeEvent* a_event, RE::BSTEventSource<RE::TESUniqueIDChangeEvent>*) {
    std::shared_lock lock(this->_lock);
    if(!a_event) {
        return EventResult::kContinue;
    }
    if(constexpr auto playerRefID{ 0x14 }; (a_event->oldBaseID != playerRefID && a_event->newBaseID != playerRefID) || a_event->oldUniqueID == a_event->newUniqueID) {
        return EventResult::kContinue;
    }
    const auto form{ RE::TESForm::LookupByID(a_event->objectID) };
    if(!form) {
        return EventResult::kContinue;
    }
    uint16_t oldUniqueID{ a_event->oldUniqueID };
    uint16_t newUniqueID{ a_event->newUniqueID };
    for(auto* item : this->_mutables) {
        if(item->GetFormID() == form->GetFormID()) {
            if(item->GetUniqueID() == oldUniqueID && newUniqueID != 0) {
                logger::info("{}'s new unique id changed from {} to {} due to external changes.", form->GetName(), oldUniqueID, newUniqueID);
                item->SetUniqueID(newUniqueID);  // update the uniqueID of the item
            }
        }
    }
    return EventResult::kContinue;
}
