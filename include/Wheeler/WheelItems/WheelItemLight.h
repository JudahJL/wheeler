#pragma once

#include "WheelItem.h"

class WheelItemLight final: public WheelItem
{
public:
	WheelItemLight() = delete;
    explicit WheelItemLight(RE::TESObjectLIGH* a_light);

    void DrawSlot(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) override;
    void DrawHighlight(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs& a_drawArgs) override;
    bool IsActive(RE::TESObjectREFR::InventoryItemMap& a_inv) override;
    bool IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv) override;
    void ActivateItemSecondary() override;
    void ActivateItemPrimary() override;

    void SerializeIntoJsonObj(nlohmann::json& a_json) override;

	static inline auto ITEM_TYPE_STR{ "WheelItemLight" };

private:
	RE::TESObjectLIGH* _light;

	void toggleEquip();
};
