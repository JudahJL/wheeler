#pragma once

#include "Rendering/TextureManager.h"
#include <nlohmann/json.hpp>

class WheelItem
{
public:
    virtual ~WheelItem() = default;
             WheelItem() = default;
	
	/// <summary>
	/// Draw everything that's supposed to be in a wheel slot(entry)
	/// Currently, DrawSlot should draw an image of the item and its name.
	/// </summary>
	virtual void DrawSlot(ImVec2, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs&) {}
	
	/// <summary>
	/// Draw everything of the item that's supposed to be in the highlight region i.e. center of the wheel.
	/// Currently, DrawHighlight should draw an enlarged image of the item, item description, and item stats(if applicable).
	/// </summary>
	virtual void DrawHighlight(ImVec2, RE::TESObjectREFR::InventoryItemMap&, const DrawArgs&){}
	virtual constexpr bool IsActive(RE::TESObjectREFR::InventoryItemMap&) {return false;}

	/// <summary>
	/// Whether the item is available. 
	/// An item is unavailable when the player lacks skill to use it, or the item is not in the player's inventory.
	/// </summary>
	virtual constexpr bool IsAvailable(RE::TESObjectREFR::InventoryItemMap&){return false;}

	virtual void ActivateItemPrimary(){};
	virtual void ActivateItemSecondary(){};
	virtual void ActivateItemSpecial(){};

	virtual void SerializeIntoJsonObj(nlohmann::json& a_json);
	// static constexpr std::shared_ptr<WheelItem> SerializeFromJsonObj(nlohmann::json&){ return nullptr; }

	static inline auto ITEM_TYPE_STR{ "WheelItem" };


protected:
	Texture::Image _texture;
	Texture::Image _stat_texture;
	std::string _description;  // buffer for description.
	
	/// <summary>
    /// Draws stat icon and value of the item when the item is highlighted.
    /// Coordinates and scale of the icon texture and value text are determined by Config.
    /// </summary>
    static void drawItemHighlightStatIconAndValue(ImVec2 a_center, const Texture::Image& a_stat_icon, float a_stat_value, const DrawArgs& a_drawArgs);

    static void drawHighlightDescription(ImVec2 a_center, const char* a_text, const DrawArgs& a_drawArgs);
	void        drawHighlightTexture(ImVec2 a_center, const DrawArgs& a_drawArgs) const;
    static void drawHighlightText(ImVec2 a_center, const char* a_text, const DrawArgs& a_drawArgs);
	void        drawSlotTexture(ImVec2 a_center, const DrawArgs& a_drawArgs) const;
    static void drawSlotText(ImVec2 a_center, const char* a_text, const DrawArgs& a_drawArgs);
};
