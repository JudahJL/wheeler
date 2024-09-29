#include "Rendering/TextureManager.h"
// #include "Rendering/Drawer.h"
#include "Wheeler/WheelItems/WheelItemMisc.h"

WheelItemMisc::WheelItemMisc(RE::TESObjectMISC* a_miscItem)
{
	this->_miscItem = a_miscItem;
	this->_texture = Texture::GetIconImage(Texture::icon_image_type::icon_default, a_miscItem);
}

void WheelItemMisc::DrawSlot(const ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs)
{
	WheelItemMisc::drawSlotText(a_center, this->_miscItem->GetName(), a_drawArgs);
	this->drawSlotTexture(a_center, a_drawArgs);
}

void WheelItemMisc::DrawHighlight(const ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs)
{
	WheelItemMisc::drawHighlightText(a_center, this->_miscItem->GetName(), a_drawArgs);
	this->drawHighlightTexture(a_center, a_drawArgs);
}

bool WheelItemMisc::IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv)
{
	return a_inv.contains(this->_miscItem) && a_inv[this->_miscItem].first > 0;
}

void WheelItemMisc::ActivateItemSecondary()
{
	this->useItem();
}

void WheelItemMisc::ActivateItemPrimary()
{
	this->useItem();
}

void WheelItemMisc::SerializeIntoJsonObj(nlohmann::json& a_json)
{
	a_json["type"] = WheelItemMisc::ITEM_TYPE_STR;
	a_json["formID"] = this->_miscItem->GetFormID();
}

void WheelItemMisc::useItem() const {
    const auto [pc, aeMan]{
        std::pair{ RE::PlayerCharacter::GetSingleton(), RE::ActorEquipManager::GetSingleton() }
    };
    if(!pc || !aeMan) [[unlikely]] {
        return;
    }
	aeMan->EquipObject(pc, this->_miscItem);
}
