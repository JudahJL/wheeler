#include "Wheeler/WheelItems/WheelItemMutable.h"
#include "Wheeler/WheelItems/WheelItemMutableManager.h"

WheelItemMutable::~WheelItemMutable()
{
	WheelItemMutableManager::GetSingleton()->UnTrack(this);
}

uint16_t WheelItemMutable::GetUniqueID() const {
	std::lock_guard lock(_uniqueIDLock);
	return this->_uniqueID;
}

void WheelItemMutable::SetUniqueID(const uint16_t a_id)
{
	std::lock_guard lock(this->_uniqueIDLock);
	this->_uniqueID = a_id;
}

RE::FormID WheelItemMutable::GetFormID() const {
	return this->_obj->GetFormID();
}


std::pair<int, RE::ExtraDataList*> WheelItemMutable::GetItemExtraDataAndCount(RE::TESObjectREFR::InventoryItemMap& a_inv) const {
	try {
		std::pair<int, RE::ExtraDataList*> ret { 0, nullptr };

		std::unique_ptr<RE::InventoryEntryData>* pp{ nullptr };
		for (auto& [boundObj, data] : a_inv) {
			if (boundObj->formID == this->_obj->GetFormID()) {
				pp = &data.second;
			}
		}
		if (pp) {
			int cleanItemCount{};  // items not modified by enchantment, poison or tempering
            const uint16_t uniqueID{ this->GetUniqueID() };
			// iterate through the entry, searching for extradata
			bool targetClean { false };
			for (auto* extraList : *pp->get()->extraLists) {
				bool thisClean = false;
				if (!extraList->HasType(RE::ExtraDataType::kEnchantment) && !extraList->HasType(RE::ExtraDataType::kPoison) && !extraList->HasType(RE::ExtraDataType::kHealth)) {
					thisClean = true;  // itme is not modified
				}
				if (thisClean) {
					cleanItemCount++;
				}
				if (extraList->HasType(RE::ExtraDataType::kUniqueID)) {
					if (uniqueID == extraList->GetByType<RE::ExtraUniqueID>()->uniqueID) {
						ret.second = extraList;
						if (thisClean) {
							targetClean = true;
						} else {  // item with matching uniqueID isn't clean, item therefore can't stack.
							ret.first = 1;
							return ret;
						}
					}
				}
			}
			if (targetClean) {
				ret.first = cleanItemCount;
			}
		}
		return ret;
	} catch (const std::exception& e) {
		logger::error("Exception caught in WheelItemMutable::GetItemExtraDataAndCount: {}", e.what());
		return { 0, nullptr };
	}
}

void WheelItemMutable::GetItemEnchantment(RE::TESObjectREFR::InventoryItemMap& a_invMap, std::vector<RE::EnchantmentItem*>& r_enchantments) const {
	switch (this->_obj->GetFormType()) {
	case RE::FormType::Weapon:
		{
            const auto weapon { (this->_obj->As<RE::TESObjectWEAP>()) };
			if (weapon->formEnchanting) {
				r_enchantments.push_back(weapon->formEnchanting);
			}
			break;
		}
	case RE::FormType::Armor: {
            const auto armor { (this->_obj->As<RE::TESObjectARMO>()) };
            if(armor->formEnchanting) {
                r_enchantments.push_back(armor->formEnchanting);
            }
            break;
        }
        default:
            break;
    }
	std::unique_ptr<RE::InventoryEntryData>* pp{ nullptr };
	for (auto& [boundObj, data] : a_invMap) {
		if (boundObj->formID == this->_obj->GetFormID()) {
			pp = &data.second;
			break;
		}
	}
	if (pp) {
		for (auto* extraList : *pp->get()->extraLists) {
			if (extraList->HasType(RE::ExtraDataType::kUniqueID)) {
				if (this->GetUniqueID() == extraList->GetByType<RE::ExtraUniqueID>()->uniqueID) {
					if (extraList->HasType(RE::ExtraDataType::kEnchantment)) {
						r_enchantments.push_back(extraList->GetByType<RE::ExtraEnchantment>()->enchantment);
					}
					break;
				}
			}
		}
	}
}

void WheelItemMutable::GetItemEnchantment(const RE::InventoryEntryData* a_iData, std::vector<RE::EnchantmentItem*>& r_enchantments) const {
	switch (this->_obj->GetFormType()) {
	case RE::FormType::Weapon:
		{
			const auto weapon{ this->_obj->As<RE::TESObjectWEAP>() };
			if (weapon->formEnchanting) {
				r_enchantments.push_back(weapon->formEnchanting);
			}
			break;
		}
	case RE::FormType::Armor: {
            const auto armor { this->_obj->As<RE::TESObjectARMO>() };
            if(armor->formEnchanting) {
                r_enchantments.push_back(armor->formEnchanting);
            }
            break;
        }
        default:
            break;
    }

	if (a_iData) {
		for (RE::ExtraDataList* extraList : *a_iData->extraLists) {
			if (extraList->HasType(RE::ExtraDataType::kUniqueID)) {
				if (this->GetUniqueID() == extraList->GetByType<RE::ExtraUniqueID>()->uniqueID) {
					if (extraList->HasType(RE::ExtraDataType::kEnchantment)) {
						r_enchantments.push_back(extraList->GetByType<RE::ExtraEnchantment>()->enchantment);
					}
					break;
				}
			}
		}
	}
}

// static bool filterMutableItems(const RE::TESBoundObject& a_obj)
// {
// 	switch (a_obj.GetFormType()) {
// 	case RE::FormType::Weapon:
// 	case RE::FormType::Armor:
//         return true;
//     default:
//         return false;
//     }
// 	return false;
// }
