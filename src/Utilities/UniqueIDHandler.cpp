#include "Utilities/UniqueIDHandler.h"
#include "Utilities/Utils.h"

void UniqueIDHandler::EnsureXListUniquenessInPcInventory() {
    try {
        const auto pc{ RE::PlayerCharacter::GetSingleton() };
        if(!pc) [[unlikely]] {
            return;
        }
        for(auto& [boundObj, data] : pc->GetInventory()) {
            auto        rawCount{ data.first };
            const auto& entryData{ data.second };
            const auto  ft{ entryData->object->GetFormType() };
            if(ft != RE::FormType::Armor && ft != RE::FormType::Weapon) {
                continue;
            }
            if(entryData->extraLists) {
                for(auto* xList : *entryData->extraLists) {
                    if(xList) {
                        const auto count{ xList->GetCount() };
                        rawCount -= count;
                        try {
                            EnsureXListUniqueness(xList);
                        } catch(const std::exception& exception) {
                            logger::error("Error occured when ensuring extraDataList uniqueness: {}, item: {}",
                                          exception.what(), entryData->object ? entryData->object->GetName() : "unknown");
                        }
                    }
                }
            }
            RE::ExtraDataList* xList{ nullptr };  // extra data list to be added
            while(rawCount-- > 0) {
                // workaround: directly adding the extradatalist doesn't work. instead we remove the item with the removal target
                // set to pc, and the hook on addItem will append the extraDatalist.
                pc->RemoveItem(boundObj, 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, xList, pc);
            }
        }
    } catch(const std::exception& exception) {
        logger::error("Error occured when scanning player inventory extraDataList: {}", exception.what());
    }
}

void UniqueIDHandler::EnsureXListUniqueness(RE::ExtraDataList*& a_extraList) {
    const auto pc{ RE::PlayerCharacter::GetSingleton() };
    if(!pc) [[unlikely]] {
        return;
    }
    {
        auto invChanges{ pc->GetInventoryChanges() };
        if(!invChanges) {
            return;
        }

        if(a_extraList == nullptr) {
            a_extraList = static_cast<RE::ExtraDataList*>(Utils::Workaround::NiMemAlloc_1400F6B40(24));
            {  //this code block is straight from dtry's fork of CommonLibSSE-NG
                using func_t = RE::ExtraDataList* (*)(RE::ExtraDataList*);
                REL::Relocation<func_t>           func{ RELOCATION_ID(11'437, 11'583) };
                func(a_extraList);
            }
        }

        if(!a_extraList->HasType(RE::ExtraDataType::kUniqueID)) {
            const uint16_t nextID{ invChanges->GetNextUniqueID() };
            const auto     xID{ new RE::ExtraUniqueID(0x14, nextID) };
            a_extraList->Add(xID);
        }
    }
}
