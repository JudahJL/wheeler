#include "Wheeler/WheelItems/WheelItem.h"
#include "Rendering/Drawer.h"
#include "Config.h"

void WheelItem::SerializeIntoJsonObj(nlohmann::json& a_json)
{
	a_json["type"] = ITEM_TYPE_STR;
	a_json["WARNING"] = "This is a placeholder item. It should not be used.";
}

void WheelItem::drawItemHighlightStatIconAndValue(const ImVec2 a_center, const Texture::Image& a_stat_icon, const float a_stat_value, const DrawArgs& a_drawArgs)
{
	Drawer::draw_text(
		a_center.x + Config::Styling::Item::Highlight::StatText::OffsetX,
		a_center.y + Config::Styling::Item::Highlight::StatText::OffsetY,
		std::format(": {}", static_cast<int>(std::ceil(a_stat_value))).data(),
		Config::Styling::Wheel::TextColor,
		Config::Styling::Item::Highlight::StatText::Size,
		a_drawArgs,
		false);
	
	Drawer::draw_texture(
		a_stat_icon.texture,
		a_center,
		Config::Styling::Item::Highlight::StatIcon::OffsetX,
		Config::Styling::Item::Highlight::StatIcon::OffsetY,
		ImVec2(a_stat_icon.width * Config::Styling::Item::Highlight::StatIcon::Scale, a_stat_icon.height * Config::Styling::Item::Highlight::StatIcon::Scale),
		C_SKYRIMWHITE,
		a_drawArgs);
}

void WheelItem::drawHighlightDescription(const ImVec2 a_center, const char* a_text, const DrawArgs& a_drawArgs)
{
    const std::string buf{a_text};
	Drawer::draw_text_block(a_center.x + Config::Styling::Item::Highlight::Desc::OffsetX, a_center.y + Config::Styling::Item::Highlight::Desc::OffsetY,
                            buf, Config::Styling::Wheel::TextColor, Config::Styling::Item::Highlight::Desc::Size, Config::Styling::Item::Highlight::Desc::LineSpacing, Config::Styling::Item::Highlight::Desc::LineLength, a_drawArgs);
}

void WheelItem::drawHighlightTexture(const ImVec2 a_center, const DrawArgs& a_drawArgs) const {
	Drawer::draw_texture(_texture.texture,
		ImVec2(a_center.x, a_center.y),
		Config::Styling::Item::Highlight::Texture::OffsetX,
		Config::Styling::Item::Highlight::Texture::OffsetY,
		ImVec2(_texture.width * Config::Styling::Item::Highlight::Texture::Scale, _texture.height * Config::Styling::Item::Highlight::Texture::Scale),
		C_SKYRIMWHITE, a_drawArgs);
}

void WheelItem::drawHighlightText(const ImVec2 a_center, const char* a_text, const DrawArgs& a_drawArgs)
{
	using namespace Config::Styling::Item::Highlight;
	Drawer::draw_text(a_center.x + Text::OffsetX, a_center.y + Text::OffsetY,
		a_text, Config::Styling::Wheel::TextColor, Text::Size, a_drawArgs);
}

void WheelItem::drawSlotTexture(const ImVec2 a_center, const DrawArgs& a_drawArgs) const {
	using namespace Config::Styling::Item::Slot;	
	Drawer::draw_texture(_texture.texture,
		ImVec2(a_center.x, a_center.y),
		Config::Styling::Item::Slot::Texture::OffsetX,
		Config::Styling::Item::Slot::Texture::OffsetY,
		ImVec2(_texture.width * Config::Styling::Item::Slot::Texture::Scale, _texture.height * Config::Styling::Item::Slot::Texture::Scale),
		C_SKYRIMWHITE, a_drawArgs);
}

void WheelItem::drawSlotText(const ImVec2 a_center, const char* a_text, const DrawArgs& a_drawArgs)
{
	using namespace Config::Styling::Item::Slot;
	Drawer::draw_text(a_center.x + Text::OffsetX, a_center.y + Text::OffsetY,
		a_text, Config::Styling::Wheel::TextColor, Text::Size, a_drawArgs);
}

