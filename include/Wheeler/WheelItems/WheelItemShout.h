#pragma once

#include "WheelItem.h"

class WheelItemShout final: public WheelItem
{
public:
     WheelItemShout()          = delete;
    ~WheelItemShout() override = default;

    explicit WheelItemShout(RE::TESShout* a_shout);

    void DrawSlot(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) override;
    void DrawHighlight(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) override;
    bool IsActive(RE::TESObjectREFR::InventoryItemMap&) override;

    constexpr bool IsAvailable(RE::TESObjectREFR::InventoryItemMap&) override { return false; };

    void ActivateItemSecondary() override;
    void ActivateItemPrimary() override;
    void SerializeIntoJsonObj(nlohmann::json& a_json) override;

    void ActivateItemSpecial() override { }

    static inline auto ITEM_TYPE_STR{ "WheelItemShout" };

private:
    RE::TESShout* _shout{ nullptr };
};
