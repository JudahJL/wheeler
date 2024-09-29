#pragma once

#include "WheelItem.h"

class WheelItemAlchemy final: public WheelItem
{
public:
    WheelItemAlchemy() = delete;

    explicit WheelItemAlchemy(RE::AlchemyItem* a_alchemyItem);

    ~WheelItemAlchemy() override = default;

    void DrawSlot(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) override;
    void DrawHighlight(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) override;

    constexpr bool IsActive(RE::TESObjectREFR::InventoryItemMap&) override { return false; }

    bool IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv) override;

    void ActivateItemPrimary() override;
    void ActivateItemSecondary() override;

    void ActivateItemSpecial() override { }

    void SerializeIntoJsonObj(nlohmann::json& a_json) override;

    static inline auto ITEM_TYPE_STR{ "WheelItemAlchemy" };

private:
    enum class WheelItemAlchemyType {
        kFood,
        kPotion,
        kPoison,
        kNone
    };
    RE::AlchemyItem*     _alchemyItem{ nullptr };
    WheelItemAlchemyType _alchemyItemType{ WheelItemAlchemyType::kNone };

    void consume() const;
    void applyPoison() const;
};
