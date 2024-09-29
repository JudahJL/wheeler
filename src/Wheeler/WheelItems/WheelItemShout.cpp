// #include "Rendering/Drawer.h"
#include "Rendering/TextureManager.h"
#include "Wheeler/WheelItems/WheelItemShout.h"

WheelItemShout::WheelItemShout(RE::TESShout* a_shout) {
    this->_shout   = a_shout;
    this->_texture = Texture::GetIconImage(Texture::icon_image_type::shout, a_shout);
    RE::BSString descriptionBuf;
    this->_shout->GetDescription(descriptionBuf, nullptr);
    this->_description = descriptionBuf.c_str();
}

void WheelItemShout::DrawSlot(const ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) {
    WheelItemShout::drawSlotText(a_center, this->_shout->GetName(), a_drawArgs);
    this->drawSlotTexture(a_center, a_drawArgs);
}

void WheelItemShout::DrawHighlight(const ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) {
    WheelItemShout::drawHighlightText(a_center, this->_shout->GetName(), a_drawArgs);
    this->drawHighlightTexture(a_center, a_drawArgs);
    if(!this->_description.empty()) {
        WheelItemShout::drawHighlightDescription(a_center, this->_description.data(), a_drawArgs);
    }
}

bool WheelItemShout::IsActive(RE::TESObjectREFR::InventoryItemMap&) {
    const auto* pc{ RE::PlayerCharacter::GetSingleton() };
    if(!pc) [[unlikely]] {
        return false;
    }
    {
        RE::TESForm* selectedPower{ pc->GetActorRuntimeData().selectedPower };
        if(selectedPower) {
            return selectedPower->GetFormID() == this->_shout->GetFormID();
        }
    }

    return false;
}

//TODO: check if shout's been unlocked, block equipment if not unlocked.
void WheelItemShout::ActivateItemSecondary() {
    const auto [pc, aeMan]{
        std::pair{ RE::PlayerCharacter::GetSingleton(), RE::ActorEquipManager::GetSingleton() }
    };
    if(!pc || !aeMan) [[unlikely]] {
        return;
    }
    {
        RE::TESForm* selectedPower{ pc->GetActorRuntimeData().selectedPower };
        if(selectedPower && selectedPower->GetFormID() == this->_shout->GetFormID()) {
            aeMan->UnequipObject(pc, this->_shout->As<RE::TESBoundObject>());
        } else {
            aeMan->EquipShout(pc, this->_shout);
        }
    }
}

void WheelItemShout::ActivateItemPrimary() {
    const auto [pc, aeMan]{
        std::pair{ RE::PlayerCharacter::GetSingleton(), RE::ActorEquipManager::GetSingleton() }
    };
    if(!pc || !aeMan) [[unlikely]] {
        return;
    }
    {
        RE::TESForm* selectedPower = pc->GetActorRuntimeData().selectedPower;
        if(selectedPower && selectedPower->GetFormID() == this->_shout->GetFormID()) {
            aeMan->UnequipObject(pc, this->_shout->As<RE::TESBoundObject>());
        } else {
            aeMan->EquipShout(pc, this->_shout);
        }
    }
}

void WheelItemShout::SerializeIntoJsonObj(nlohmann::json& a_json) {
    a_json["type"]   = WheelItemShout::ITEM_TYPE_STR;
    a_json["formID"] = this->_shout->GetFormID();
}

