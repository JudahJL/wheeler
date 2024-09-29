#pragma once

#include "WheelItem.h"

class WheelItemScroll final: public WheelItem
{
public:
     WheelItemScroll()          = delete;
    ~WheelItemScroll() override = default;

    explicit WheelItemScroll(RE::ScrollItem* a_scroll);

    void DrawSlot(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) override;
    void DrawHighlight(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) override;
    bool IsActive(RE::TESObjectREFR::InventoryItemMap&) override;
    bool IsAvailable(RE::TESObjectREFR::InventoryItemMap&) override;
    void ActivateItemSecondary() override;
    void ActivateItemPrimary() override;

    void SerializeIntoJsonObj(nlohmann::json& a_json) override;

    static inline auto ITEM_TYPE_STR{ "WheelItemScroll" };

private:
    RE::ScrollItem* _scroll{ nullptr };
};
