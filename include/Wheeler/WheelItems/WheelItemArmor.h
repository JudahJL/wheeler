#pragma once
#include "WheelItemMutable.h"

class TimeColorInterpolator;

class WheelItemArmor final: public WheelItemMutable
{
public:
    void DrawSlot(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) override;
    void DrawHighlight(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) override;
    bool IsActive(RE::TESObjectREFR::InventoryItemMap& a_inv) override;
    bool IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv) override;

     WheelItemArmor(RE::TESBoundObject* a_armor, uint16_t a_uniqueID);
     WheelItemArmor() = delete;
    ~WheelItemArmor() override = default;  // wheelitemmutable's destructor will remove it from the manager

    void ActivateItemSecondary() override;
    void ActivateItemPrimary() override;

    void SerializeIntoJsonObj(nlohmann::json& a_json) override;

    static inline auto ITEM_TYPE_STR{ "WheelItemArmor" };

private:
    void toggleEquip();
    void equipArmor() const;
    void unequipArmor() const;
};
