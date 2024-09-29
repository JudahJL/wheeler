#pragma once

#include "WheelItem.h"

class WheelItemSpell final: public WheelItem // spell AND power
{
public:
	WheelItemSpell() = delete;
    ~WheelItemSpell() override = default;

	explicit WheelItemSpell(RE::SpellItem* a_spell);

    void DrawSlot(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) override;
	 void DrawHighlight(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) override;
	 bool IsActive(RE::TESObjectREFR::InventoryItemMap&) override;
	 bool IsAvailable(RE::TESObjectREFR::InventoryItemMap&) override;
	 void ActivateItemSecondary() override;
	 void ActivateItemPrimary() override;
	 void SerializeIntoJsonObj(nlohmann::json& a_json) override;
	 void ActivateItemSpecial() override;

	static inline auto ITEM_TYPE_STR{ "WheelItemSpell" };

private:
	bool isPower() const;
	RE::SpellItem* _spell{ nullptr };
};
