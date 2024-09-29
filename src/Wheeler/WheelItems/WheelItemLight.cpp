#include "Wheeler/WheelItems/WheelItemLight.h"
#include "Rendering/TextureManager.h"
#include "Rendering/Drawer.h"

WheelItemLight::WheelItemLight(RE::TESObjectLIGH* a_light) {
    if(!a_light->CanBeCarried()) {
        throw std::invalid_argument("WheelItemLight::ctor: Light must be carryable");
    }
    this->_light   = a_light;
    // load texture
    this->_texture = Texture::GetIconImage(Texture::icon_image_type::torch, a_light);
}

void WheelItemLight::DrawSlot(const ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) {
    WheelItemLight::drawSlotText(a_center, _light->GetName(), a_drawArgs);
    this->drawSlotTexture(a_center, a_drawArgs);
}

void WheelItemLight::DrawHighlight(const ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) {
    WheelItemLight::drawHighlightText(a_center, _light->GetName(), a_drawArgs);
    this->drawHighlightTexture(a_center, a_drawArgs);
}

bool WheelItemLight::IsActive(RE::TESObjectREFR::InventoryItemMap& a_inv) {
    if(!a_inv.contains(this->_light)) {
        return false;
    }
    auto entry{ a_inv.find(this->_light)->second.second.get() };
    return entry && entry->IsWorn();
}

bool WheelItemLight::IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv) {
    return a_inv.contains(this->_light);
}

void WheelItemLight::ActivateItemSecondary() {
    toggleEquip();
}

void WheelItemLight::ActivateItemPrimary() {
    toggleEquip();
}

void WheelItemLight::SerializeIntoJsonObj(nlohmann::json& a_json) {
    a_json["type"]   = WheelItemLight::ITEM_TYPE_STR;
    a_json["formID"] = this->_light->GetFormID();
}

void WheelItemLight::toggleEquip() {
    const auto [pc, aeMan]{
        std::pair{ RE::PlayerCharacter::GetSingleton(), RE::ActorEquipManager::GetSingleton() }
    };
    if(!pc || !aeMan) [[unlikely]] {
        return;
    }
    {
        RE::TESObjectREFR::InventoryItemMap invMap{ pc->GetInventory() };
        if(this->IsAvailable(invMap)) {
            if(this->IsActive(invMap)) {
                aeMan->UnequipObject(pc, this->_light);
            } else {
                aeMan->EquipObject(pc, this->_light);
            }
        }
    }
}
