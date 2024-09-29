#pragma once

#include "WheelItem.h"

class WheelItemAmmo final: public WheelItem
{
public:
     WheelItemAmmo()          = delete;
    ~WheelItemAmmo() override = default;

    explicit WheelItemAmmo(RE::TESAmmo* a_ammo);

    void DrawSlot(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) override;
    void DrawHighlight(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) override;
    bool IsActive(RE::TESObjectREFR::InventoryItemMap& a_inv) override;
    bool IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv) override;
    void ActivateItemSecondary() override;
    void ActivateItemPrimary() override;

    void SerializeIntoJsonObj(nlohmann::json& a_json) override;

    static inline auto ITEM_TYPE_STR{ "WheelItemAmmo" };

private:
    RE::TESAmmo* _ammo;

    void toggleEquip();
};
