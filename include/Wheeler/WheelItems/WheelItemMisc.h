#pragma once

#include "WheelItem.h"

class WheelItemMisc final: public WheelItem
{
public:
     WheelItemMisc()          = delete;
    ~WheelItemMisc() override = default;

    explicit WheelItemMisc(RE::TESObjectMISC* a_miscItem);

    void DrawSlot(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) override;
    void DrawHighlight(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) override;

    constexpr bool IsActive(RE::TESObjectREFR::InventoryItemMap&) override { return false; }

    bool IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv) override;
    void ActivateItemSecondary() override;
    void ActivateItemPrimary() override;

    void SerializeIntoJsonObj(nlohmann::json& a_json) override;

    static inline auto ITEM_TYPE_STR{ "WheelItemMisc" };

private:
    RE::TESObjectMISC* _miscItem;

    void useItem() const;
};
