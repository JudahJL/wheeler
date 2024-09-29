#pragma once
#include "Wheeler/WheelItems/WheelItemMutable.h"
#include "Animation/TimeColorInterpolator.h"

class TimeColorInterpolator;

class WheelItemWeapon final: public WheelItemMutable
{
public:
    WheelItemWeapon() = delete;

    void DrawSlot(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) override;
    void DrawHighlight(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) override;
    bool IsActive(RE::TESObjectREFR::InventoryItemMap& a_inv) override;
    bool IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv) override;

     WheelItemWeapon(RE::TESBoundObject* a_weapon, uint16_t a_uniqueID);
    ~WheelItemWeapon() override = default;  // wheelitemmutable's destructor will remove it from the manager

    void ActivateItemSecondary() override;
    void ActivateItemPrimary() override;

    void SerializeIntoJsonObj(nlohmann::json& a_json) override;

    static inline auto ITEM_TYPE_STR{ "WheelItemWeapon" };

private:
    void equipItem(bool a_toRight = true) const;
    void unequipItem(const RE::BGSEquipSlot* a_slot) const;
};
