#include "Rendering/Drawer.h"
#include "Rendering/TextureManager.h"

#include "Wheeler/WheelItems/WheelItem.h"
#include "Wheeler/WheelItems/WheelItemFactory.h"
// #include "Wheeler/WheelItems/WheelItemMutable.h"
#include "Wheeler/WheelEntry.h"
#include "Config.h"

void WheelEntry::UpdateAnimation(const float innerSpacingRad, const bool hovered) {
    using namespace Config::Styling::Wheel;
    // bool active = this->IsActive(imap);
    if(hovered) {
        if(!_prevHovered) {
            const float expandSize{ Config::Animation::EntryHighlightExpandScale * (OuterCircleRadius - InnerCircleRadius) };
            _arcRadiusIncInterpolator.InterpolateTo(expandSize, Config::Animation::EntryHighlightExpandTime);
            _arcOuterAngleIncInterpolator.InterpolateTo(innerSpacingRad * (InnerCircleRadius / OuterCircleRadius), Config::Animation::EntryHighlightExpandTime);
            _arcInnerAngleIncInterpolator.InterpolateTo(innerSpacingRad, Config::Animation::EntryHighlightExpandTime);
            _prevHovered = true;
        }
    } else {
        if(_prevHovered != false) {
            _prevHovered = false;
            _arcRadiusIncInterpolator.InterpolateTo(0, Config::Animation::EntryHighlightRetractTime);
            _arcOuterAngleIncInterpolator.InterpolateTo(0, Config::Animation::EntryHighlightRetractTime);
            _arcInnerAngleIncInterpolator.InterpolateTo(0, Config::Animation::EntryHighlightRetractTime);
        }
    }
}

auto WheelEntry::DrawBackGround(
    const ImVec2 wheelCenter, const ImVec2 entryCenter,
    const float entryInnerAngleMin, const float entryInnerAngleMax,
    const float entryOuterAngleMin, const float entryOuterAngleMax,
    const bool hovered,
    const int numArcSegments, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawARGS) -> void {
    const bool active{ this->IsActive(a_imap) };
    //TODO:
    // 1. add separate config for background texture scaling
    // 2. decouple this function

    using namespace Config::Styling::Wheel;

    if(UseGeometricPrimitiveForBackgroundTexture) {
        const auto mainArcOuterBoundRadius{ static_cast<float>(OuterCircleRadius + _arcRadiusIncInterpolator.GetValue() + _arcRadiusBounceInterpolator.GetValue()) };

        const float entryInnerAngleMinUpdated{ static_cast<float>(entryInnerAngleMin - _arcInnerAngleIncInterpolator.GetValue() * 2) };
        const float entryInnerAngleMaxUpdated{ static_cast<float>(entryInnerAngleMax + _arcInnerAngleIncInterpolator.GetValue() * 2) };
        const float entryOuterAngleMinUpdated{ static_cast<float>(entryOuterAngleMin - _arcOuterAngleIncInterpolator.GetValue() * 2) };
        const float entryOuterAngleMaxUpdated{ static_cast<float>(entryOuterAngleMax + _arcOuterAngleIncInterpolator.GetValue() * 2) };

        Drawer::draw_arc_gradient(wheelCenter,
                                  InnerCircleRadius,
                                  mainArcOuterBoundRadius,
                                  entryInnerAngleMinUpdated, entryInnerAngleMaxUpdated,
                                  entryOuterAngleMinUpdated, entryOuterAngleMaxUpdated,
                                  hovered ? HoveredColorBegin : UnhoveredColorBegin,
                                  hovered ? HoveredColorEnd : UnhoveredColorEnd,
                                  numArcSegments, a_drawARGS);

        const ImU32 arcColorBegin{ active ? ActiveArcColorBegin : InActiveArcColorBegin };
        const ImU32 arcColorEnd{ active ? ActiveArcColorEnd : InActiveArcColorEnd };

        Drawer::draw_arc_gradient(wheelCenter,
                                  mainArcOuterBoundRadius,
                                  mainArcOuterBoundRadius + ActiveArcWidth,
                                  entryOuterAngleMinUpdated,
                                  entryOuterAngleMaxUpdated,
                                  entryOuterAngleMinUpdated,
                                  entryOuterAngleMaxUpdated,
                                  arcColorBegin,
                                  arcColorEnd,
                                  numArcSegments, a_drawARGS);
    } else {
        const Texture::icon_image_type backgroundImageType{ active ? Texture::icon_image_type::slot_active_background : (hovered ? Texture::icon_image_type::slot_highlighted_background : Texture::icon_image_type::slot_background) };
        const auto [texture, width, height]{ Texture::GetIconImage(backgroundImageType) };

        Drawer::draw_texture(texture, entryCenter, 0, 0,
                             ImVec2(width * Config::Styling::Item::Slot::BackgroundTexture::Scale, height * Config::Styling::Item::Slot::BackgroundTexture::Scale), C_SKYRIMWHITE, a_drawARGS);
    }
}

void WheelEntry::DrawSlotAndHighlight(const ImVec2 a_wheelCenter, const ImVec2 a_entryCenter, const bool a_hovered, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) {
    if(a_hovered) {
        this->drawHighlight(a_wheelCenter, a_imap, a_drawArgs);
    }
    this->drawSlot(a_entryCenter, a_hovered, a_imap, a_drawArgs);
}

auto WheelEntry::drawSlot(const ImVec2 a_center, const bool, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) -> void {
    try {
        std::shared_lock lock(this->_lock);

        if(_items.empty()) {
            return;  // nothing to draw
        }
        _items[_selectedItem]->DrawSlot(a_center, a_imap, a_drawArgs);
    } catch(const std::exception& e) {
        logger::error("Exception in WheelEntry::drawSlot: {}", e.what());
    }
}

void WheelEntry::drawHighlight(const ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, const DrawArgs& a_drawArgs) {
    try {
        std::shared_lock lock(this->_lock);

        if(_items.empty()) {
            return;  // nothing to draw
        }
        _items[_selectedItem]->DrawHighlight(a_center, a_imap, a_drawArgs);
        if(_items.size() > 1) {
            Drawer::draw_text(
                a_center.x + Config::Styling::Entry::Highlight::Text::OffsetX,
                a_center.y + Config::Styling::Entry::Highlight::Text::OffsetY,
                std::format("{} / {}", _selectedItem + 1, _items.size()).data(),
                C_SKYRIMWHITE,
                Config::Styling::Entry::Highlight::Text::Size,
                a_drawArgs);
        }
    } catch(const std::exception& e) {
        logger::error("Exception in WheelEntry::drawHighlight: {}", e.what());
    }
}

float WheelEntry::GetRadiusMod() const {
    return this->_arcRadiusIncInterpolator.GetValue() + _arcRadiusBounceInterpolator.GetValue();
}

bool WheelEntry::IsActive(RE::TESObjectREFR::InventoryItemMap& a_inv) {
    std::shared_lock lock(this->_lock);

    if(_items.empty()) {
        return false;  // nothing to draw
    }
    return _items[_selectedItem]->IsActive(a_inv);
}

bool WheelEntry::IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv) {
    std::shared_lock lock(this->_lock);

    if(_items.empty()) {
        return false;  // nothing to draw
    }
    return _items[_selectedItem]->IsAvailable(a_inv);
}

void WheelEntry::ActivateItemSecondary(const bool editMode) {
    std::unique_lock lock(this->_lock);

    if(_items.empty()) {
        return;
    }
    if(!editMode) {
        _items[_selectedItem]->ActivateItemSecondary();
        _arcRadiusBounceInterpolator.InterpolateTo(Config::Animation::EntryInputBumpScale * (Config::Styling::Wheel::OuterCircleRadius - Config::Styling::Wheel::InnerCircleRadius), Config::Animation::EntryInputBumpTime);
    } else {
        // remove selected item
        std::shared_ptr<WheelItem> itemToDelete = _items[_selectedItem];
        _items.erase(_items.begin() + _selectedItem);
        // move _selecteditem to the item immediately before the erased item
        if(_selectedItem > 0) {
            _selectedItem--;
        }
    }
}

void WheelEntry::ActivateItemPrimary(const bool editMode) {
    std::unique_lock lock(this->_lock);

    if(!editMode) {
        if(_items.empty()) {
            return;  // nothing to do
        }
        _items[_selectedItem]->ActivateItemPrimary();
        _arcRadiusBounceInterpolator.InterpolateTo(Config::Animation::EntryInputBumpScale * (Config::Styling::Wheel::OuterCircleRadius - Config::Styling::Wheel::InnerCircleRadius), Config::Animation::EntryInputBumpTime);
    } else {  // append item to after _selectedItem index
        auto newItem{ WheelItemFactory::MakeWheelItemFromMenuHovered() };
        if(newItem) {
            _items.insert(_items.begin() + _selectedItem, newItem);
        }
    }
}

void WheelEntry::ActivateItemSpecial(bool editMode) {
    std::unique_lock lock(this->_lock);
    if(editMode || _items.empty()) {
        return;  // nothing to do
    }
    _items[_selectedItem]->ActivateItemSpecial();
}

void WheelEntry::PrevItem() {
    std::unique_lock lock(this->_lock);

    _selectedItem--;
    if(_selectedItem < 0) {
        if(_items.size() > 1) {
            _selectedItem = _items.size() - 1;
        } else {
            _selectedItem = 0;
        }
    }
    if(_items.size() > 1) {
        RE::PlaySound(Config::Sound::SD_ITEMSWITCH);
    }
}

void WheelEntry::NextItem() {
    std::unique_lock lock(this->_lock);

    _selectedItem++;
    if(_selectedItem >= _items.size()) {
        _selectedItem = 0;
    }
    if(_items.size() > 1) {
        RE::PlaySound(Config::Sound::SD_ITEMSWITCH);
    }
}

bool WheelEntry::IsEmpty() {
    std::shared_lock lock(this->_lock);
    return this->_items.empty();
}

int WheelEntry::GetNumItems() {
    std::shared_lock lock(this->_lock);
    return this->_items.size();
}

WheelEntry::WheelEntry() {
    _selectedItem = 0;
}

void WheelEntry::PushItem(const std::shared_ptr<WheelItem>& item) {
    std::unique_lock lock(this->_lock);
    this->_items.push_back(item);
}

WheelEntry::~WheelEntry() {
    _items.clear();
}

int WheelEntry::GetSelectedItemIndex() {
    std::shared_lock lock(this->_lock);
    return this->_selectedItem;
}

void WheelEntry::SerializeIntoJsonObj(nlohmann::json& j_entry) {
    // setup for entry
    // j_entry["items"] = nlohmann::json::array();
    for(const auto& item : this->_items) {
        nlohmann::json j_item;
        item->SerializeIntoJsonObj(j_item);
        j_entry["items"].push_back(std::move(j_item));
    }
    j_entry["selecteditem"] = this->_selectedItem;
}

std::unique_ptr<WheelEntry> WheelEntry::SerializeFromJsonObj(const nlohmann::json& j_entry, SKSE::SerializationInterface* a_intfc) {
    auto entry{ std::make_unique<WheelEntry>() };

    const auto& j_items{ j_entry["items"] };
    try {
        for(const auto& j_item : j_items) {
            const auto item{ WheelItemFactory::MakeWheelItemFromJsonObject(j_item, a_intfc) };
            if(item) {
                entry->PushItem(item);
            }
        }
    } catch(const std::exception& exception) {
        logger::info("Exception serializing wheel entry: {}", exception.what());
    }
    const int selectedItem = std::clamp(j_entry["selecteditem"].get<int>(), 0, entry->GetNumItems());
    entry->SetSelectedItem(selectedItem);

    return std::move(entry);
}

void WheelEntry::ResetAnimation() {
    _arcInnerAngleIncInterpolator.ForceFinish();
    _arcOuterAngleIncInterpolator.ForceFinish();
    _arcRadiusIncInterpolator.ForceFinish();
    _arcInnerAngleIncInterpolator.ForceValue(0);
    _arcOuterAngleIncInterpolator.ForceValue(0);
    _arcRadiusIncInterpolator.ForceValue(0);
    // don't need call interpolateto() because bounce interpolator's forceFinish() goes back to its fixed starting value (which is where we want it to be at)
    _arcRadiusBounceInterpolator.ForceFinish();
    this->_prevHovered = false;
}

void WheelEntry::SetSelectedItem(const int a_selected) {
    std::unique_lock lock(this->_lock);
    this->_selectedItem = a_selected;
}

