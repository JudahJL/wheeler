#include <nlohmann/json.hpp>

#include "Wheeler/WheelItems/WheelItemFactory.h"
#include "Utilities/Utils.h"
#include "Wheeler/WheelItems/WheelItem.h"
#include "Wheeler/WheelItems/WheelItemSpell.h"
#include "Wheeler/WheelItems/WheelItemWeapon.h"
#include "Wheeler/WheelItems/WheelItemArmor.h"
#include "Wheeler/WheelItems/WheelItemShout.h"
#include "Wheeler/WheelItems/WheelItemLight.h"
#include "Wheeler/WheelItems/WheelItemAmmo.h"
#include "Wheeler/WheelItems/WheelItemAlchemy.h"
#include "Wheeler/WheelItems/WheelItemScroll.h"
#include "Wheeler/WheelItems/WheelItemMisc.h"

std::shared_ptr<WheelItem> WheelItemFactory::MakeWheelItemFromMenuHovered() {
    const auto [pc, ui]{
        std::pair{ RE::PlayerCharacter::GetSingleton(), RE::UI::GetSingleton() }
    };
    if(!pc || !pc->Is3DLoaded() || !ui) [[unlikely]] {
        return nullptr;
    }
    try {
        if(ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME)) {
            auto* invMenu{ skyrim_cast<RE::InventoryMenu*>(ui->GetMenu(RE::InventoryMenu::MENU_NAME).get()) };
            if(!invMenu) {
                return nullptr;
            }
            RE::InventoryEntryData* invEntry{ Utils::Inventory::GetSelectedItemIninventory(invMenu) };
            if(!invEntry) {
                return nullptr;
            }
            RE::TESBoundObject* boundObj{ invEntry->object };
            if(!boundObj) {
                return nullptr;
            }

            // if is weapon or armor, get the item's unique ID. If there's no unique id, abort.
            uint16_t           uniqueID{};
            const RE::FormType formType{ boundObj->GetFormType() };
            switch(formType) {
                case RE::FormType::Weapon:
                case RE::FormType::Armor:  {
                    if(!invEntry->extraLists) {
                        return nullptr;
                    }
                    for(auto* extraList : *invEntry->extraLists) {
                        if(extraList->HasType(RE::ExtraDataType::kUniqueID)) {
                            auto* uniqueIDExtra{ extraList->GetByType<RE::ExtraUniqueID>() };
                            if(uniqueIDExtra) {
                                uniqueID = uniqueIDExtra->uniqueID;
                                break;
                            }
                        }
                    }
                    if(uniqueID == 0) {
                        return nullptr;
                    }
                } break;
                default:
                    break;
            }

            switch(formType) {
                case RE::FormType::Weapon: {
                    auto wheelItemweap{ WheelItemMutable::CreateWheelItemMutable<WheelItemWeapon>(boundObj, uniqueID) };
                    return wheelItemweap;
                }
                case RE::FormType::Armor: {
                    auto wheelItemArmo{ WheelItemMutable::CreateWheelItemMutable<WheelItemArmor>(boundObj, uniqueID) };
                    return wheelItemArmo;
                }
                case RE::FormType::Light: {
                    auto* light{ boundObj->As<RE::TESObjectLIGH>() };
                    if(!light || !light->CanBeCarried()) {
                        return nullptr;
                    }
                    auto wheelItemLight{ std::make_shared<WheelItemLight>(light) };
                    return wheelItemLight;
                }
                case RE::FormType::Ammo: {
                    auto* ammo{ boundObj->As<RE::TESAmmo>() };
                    if(!ammo) {
                        return nullptr;
                    }
                    auto wheelItemAmmo{ std::make_shared<WheelItemAmmo>(ammo) };
                    return wheelItemAmmo;
                }
                case RE::FormType::AlchemyItem:  // can be food, drink, poison, potion
                {
                    auto* alchemyItem{ boundObj->As<RE::AlchemyItem>() };
                    if(!alchemyItem) {
                        return nullptr;
                    }
                    auto wheelItemAlchemy{ std::make_shared<WheelItemAlchemy>(alchemyItem) };
                    return wheelItemAlchemy;
                }
                case RE::FormType::Scroll: {
                    auto scrollItem{ boundObj->As<RE::ScrollItem>() };
                    if(!scrollItem) {
                        return nullptr;
                    }
                    auto wheelItemScroll{ std::make_shared<WheelItemScroll>(scrollItem) };
                    return wheelItemScroll;
                }
                case RE::FormType::Misc: {
                    auto miscItem{ boundObj->As<RE::TESObjectMISC>() };
                    if(!miscItem) {
                        return nullptr;
                    }
                    auto wheelItemMisc{ std::make_shared<WheelItemMisc>(miscItem) };
                    return wheelItemMisc;
                }
                default:
                    break;
            }
        } else if(ui->IsMenuOpen(RE::MagicMenu::MENU_NAME)) {
            auto* magMenu{ skyrim_cast<RE::MagicMenu*>(ui->GetMenu(RE::MagicMenu::MENU_NAME).get()) };
            if(!magMenu) {
                return nullptr;
            }
            RE::TESForm* form{ Utils::Inventory::GetSelectedFormInMagicMenu(magMenu) };
            if(!form) {
                return nullptr;
            }
            switch(form->GetFormType()) {
                case RE::FormType::Spell: {
                    auto wheelItemSpell{ std::make_shared<WheelItemSpell>(form->As<RE::SpellItem>()) };
                    return wheelItemSpell;
                }
                case RE::FormType::Shout: {
                    auto wheelItemShout{ std::make_shared<WheelItemShout>(form->As<RE::TESShout>()) };
                    return wheelItemShout;
                }
                default:
                    break;
            }
        }
    } catch(const std::exception& exception) {
        logger::info("Exception: {}", exception.what());
    }

    return nullptr;
}

std::shared_ptr<WheelItem> WheelItemFactory::MakeWheelItemFromJsonObject(nlohmann::json a_json, SKSE::SerializationInterface* a_intfc) {
    if(!a_json.contains("type")) {
        throw std::exception("WheelItemFactory::MakeWheelItemFromJsonObject: Json object does not contain a \"type\" field.");
    }
    if(!a_json.contains("formID")) {
        throw std::exception("WheelItemFactory::MakeWheelItemFromJsonObject: Json object does not contain a \"formID\" field.");
    }
    const std::string type{ a_json["type"] };
    RE::FormID        formID{ a_json["formID"].get<RE::FormID>() };
    if(!a_intfc->ResolveFormID(formID, formID)) {
        return nullptr;
    }
    try {
        if(type == WheelItemWeapon::ITEM_TYPE_STR) {
            auto* weap{ RE::TESForm::LookupByID(formID)->As<RE::TESObjectWEAP>() };
            if(!weap) {
                return nullptr;
            }
            const uint16_t uniqueID{ a_json["uniqueID"].get<uint16_t>() };
            auto           wheelItemweap{ WheelItemMutable::CreateWheelItemMutable<WheelItemWeapon>(weap, uniqueID) };
            return wheelItemweap;
        }
        if(type == WheelItemArmor::ITEM_TYPE_STR) {
            auto* armor{ RE::TESForm::LookupByID(formID)->As<RE::TESObjectARMO>() };
            if(!armor) {
                return nullptr;
            }
            const uint16_t uniqueID{ a_json["uniqueID"].get<uint16_t>() };
            auto           wheelItemArmo{ WheelItemMutable::CreateWheelItemMutable<WheelItemArmor>(armor, uniqueID) };
            return wheelItemArmo;
        }
        if(type == WheelItemSpell::ITEM_TYPE_STR) {
            auto spell{ (RE::TESForm::LookupByID(formID)->As<RE::SpellItem>()) };
            if(!spell) {
                return nullptr;
            }
            auto wheelItemSpell{ std::make_shared<WheelItemSpell>(spell) };
            return wheelItemSpell;
        }
        if(type == WheelItemShout::ITEM_TYPE_STR) {
            auto shout{ (RE::TESForm::LookupByID(formID)->As<RE::TESShout>()) };
            if(!shout) {
                return nullptr;
            }
            auto wheelItemShout{ std::make_shared<WheelItemShout>(shout) };
            return wheelItemShout;
        }
        if(type == WheelItemLight::ITEM_TYPE_STR) {
            auto light{ (RE::TESForm::LookupByID(formID)->As<RE::TESObjectLIGH>()) };
            if(!light) {
                return nullptr;
            }
            auto wheelItemLight{ std::make_shared<WheelItemLight>(light) };
            return wheelItemLight;
        }
        if(type == WheelItemAmmo::ITEM_TYPE_STR) {
            auto ammo{ (RE::TESForm::LookupByID(formID)->As<RE::TESAmmo>()) };
            if(!ammo) {
                return nullptr;
            }
            auto wheelItemAmmo{ std::make_shared<WheelItemAmmo>(ammo) };
            return wheelItemAmmo;
        }
        if(type == WheelItemAlchemy::ITEM_TYPE_STR) {
            auto alchemyItem{ (RE::TESForm::LookupByID(formID)->As<RE::AlchemyItem>()) };
            if(!alchemyItem) {
                return nullptr;
            }
            auto wheelItemAlchemy{ std::make_shared<WheelItemAlchemy>(alchemyItem) };
            return wheelItemAlchemy;
        }
        if(type == WheelItemScroll::ITEM_TYPE_STR) {
            auto scrollItem{ (RE::TESForm::LookupByID(formID)->As<RE::ScrollItem>()) };
            if(!scrollItem) {
                return nullptr;
            }
            auto wheelItemScroll{ std::make_shared<WheelItemScroll>(scrollItem) };
            return wheelItemScroll;
        }
        if(type == WheelItemMisc::ITEM_TYPE_STR) {
            auto miscObj{ (RE::TESForm::LookupByID(formID)->As<RE::TESObjectMISC>()) };
            if(!miscObj) {
                return nullptr;
            }
            auto wheelItemMisc{ std::make_shared<WheelItemMisc>(miscObj) };
            return wheelItemMisc;
        }
    } catch(const std::exception& exception) {
        logger::info("Exception when de-serializing wheel item: {}", exception.what());
    }

    return nullptr;
}
