#include "Utilities/Utils.h"
#include "Wheeler/WheelItems/WheelItemScroll.h"

WheelItemScroll::WheelItemScroll(RE::ScrollItem* a_scroll) {
    this->_scroll = a_scroll;
    RE::BSString descriptionBuf;
    this->_scroll->GetDescription(descriptionBuf, nullptr);
    this->_description = descriptionBuf;
    this->_texture     = Texture::GetIconImage(Texture::icon_image_type::scroll, a_scroll);
}

void WheelItemScroll::DrawSlot(const ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) {
    int               itemCount{ a_imap.contains(this->_scroll) ? a_imap[this->_scroll].first : 0 };
    const std::string text{ std::format("{}{}", this->_scroll->GetName(), itemCount > 0 ? std::format(" ({})", itemCount) : "") };

    WheelItemScroll::drawSlotText(a_center, text.data(), a_drawArgs);
    this->drawSlotTexture(a_center, a_drawArgs);
}

void WheelItemScroll::DrawHighlight(const ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) {
    WheelItemScroll::drawHighlightText(a_center, this->_scroll->GetName(), a_drawArgs);
    this->drawHighlightTexture(a_center, a_drawArgs);

    std::string descriptionBuf;
    if(this->_description.empty()) {
        Utils::Magic::GetMagicItemDescription(this->_scroll, descriptionBuf);
    }

    WheelItemScroll::drawHighlightDescription(a_center, descriptionBuf.data(), a_drawArgs);
}

bool WheelItemScroll::IsActive(RE::TESObjectREFR::InventoryItemMap&) {
    const auto pc{ RE::PlayerCharacter::GetSingleton() };
    if(!pc) [[unlikely]] {
        return false;
    }
    {
        RE::TESForm* lhs{ pc->GetEquippedObject(true) };
        if(lhs) {
            if(lhs->GetFormID() == this->_scroll->GetFormID()) {
                return true;
            }
        }
        RE::TESForm* rhs{ pc->GetEquippedObject(false) };
        if(rhs) {
            if(rhs->GetFormID() == this->_scroll->GetFormID()) {
                return true;
            }
        }
    }
    return false;
}

bool WheelItemScroll::IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv) {
    return a_inv.contains(this->_scroll) && a_inv[this->_scroll].first > 0;
}

void WheelItemScroll::ActivateItemSecondary() {
    RE::PlayerCharacter*   pc{ RE::PlayerCharacter::GetSingleton() };
    RE::ActorEquipManager* aeMan{ RE::ActorEquipManager::GetSingleton() };
    if(!pc || !aeMan) [[unlikely]] {
        return;
    }
    RE::TESObjectREFR::InventoryItemMap iMap{ pc->GetInventory() };
    if(!this->IsAvailable(iMap)) {
        return;
    }
    aeMan->EquipObject(pc, this->_scroll, nullptr, 1, Utils::Slot::GetLeftHandSlot());
}

void WheelItemScroll::ActivateItemPrimary() {
    RE::PlayerCharacter*   pc{ RE::PlayerCharacter::GetSingleton() };
    RE::ActorEquipManager* aeMan{ RE::ActorEquipManager::GetSingleton() };
    if(!pc || !aeMan) [[unlikely]] {
        return;
    }
    RE::TESObjectREFR::InventoryItemMap iMap{ pc->GetInventory() };
    if(!this->IsAvailable(iMap)) {
        return;
    }
    aeMan->EquipObject(pc, this->_scroll, nullptr, 1, Utils::Slot::GetRightHandSlot());
}

void WheelItemScroll::SerializeIntoJsonObj(nlohmann::json& a_json) {
    a_json["type"]   = WheelItemScroll::ITEM_TYPE_STR;
    a_json["formID"] = this->_scroll->GetFormID();
}
