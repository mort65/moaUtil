#include "Utility.h"
#include "Events.h"
#include "Hooks.h"
#include "Logging.h"
#include "Settings.h"

using namespace moaUtil;

    namespace
    {
        constexpr std::string_view moaClass = "moaUtil";
        enum amount_t
        {
            all    = -1,
            random = 0
        };

        std::vector<int> Get_Version(RE::StaticFunctionTag*)
        {
            std::vector<int> version{ 1, 0, 0 };
            return version;
        }

        std::vector<RE::TESForm*> GetItemsByType(RE::StaticFunctionTag*, RE::TESObjectREFR* a_container, std::vector<UINT32> a_types)
        {
            SKSE::log::info("GetItemsByType");
            std::vector<RE::TESForm*> filtered;

            if (!a_container || a_types.empty()) {
                return filtered;
            }

            bool bScroll     = (std::find(a_types.begin(), a_types.end(), (UINT32)RE::FormType::Scroll) != a_types.end());
            bool bArmor      = (std::find(a_types.begin(), a_types.end(), (UINT32)RE::FormType::Armor) != a_types.end());
            bool bBook       = (std::find(a_types.begin(), a_types.end(), (UINT32)RE::FormType::Book) != a_types.end());
            bool bIngredient = (std::find(a_types.begin(), a_types.end(), (UINT32)RE::FormType::Ingredient) != a_types.end());
            bool bMisc       = (std::find(a_types.begin(), a_types.end(), (UINT32)RE::FormType::Misc) != a_types.end());
            bool bWeapon     = (std::find(a_types.begin(), a_types.end(), (UINT32)RE::FormType::Weapon) != a_types.end());
            bool bAmmo       = (std::find(a_types.begin(), a_types.end(), (UINT32)RE::FormType::Ammo) != a_types.end());
            bool bKey        = (std::find(a_types.begin(), a_types.end(), (UINT32)RE::FormType::KeyMaster) != a_types.end());
            bool bpotion     = (std::find(a_types.begin(), a_types.end(), (UINT32)RE::FormType::AlchemyItem) != a_types.end());
            bool bSoulgem    = (std::find(a_types.begin(), a_types.end(), (UINT32)RE::FormType::SoulGem) != a_types.end());

            auto inventory = a_container->GetInventory();
            bool bAddItem  = false;
            for (const auto& [form, data] : inventory) {
                if (!form->GetPlayable() || form->GetName()[0] == '\0')
                    continue;

                bAddItem = false;
                switch (form->GetFormType()) {
                case RE::FormType::Scroll:
                    if (bScroll)
                        bAddItem = true;
                    break;
                case RE::FormType::Armor:
                    if (bArmor)
                        bAddItem = true;
                    break;
                case RE::FormType::Book:
                    if (bBook)
                        bAddItem = true;
                    break;
                case RE::FormType::Ingredient:
                    if (bIngredient)
                        bAddItem = true;
                    break;
                case RE::FormType::Misc:
                    if (bMisc)
                        bAddItem = true;
                    break;
                case RE::FormType::Weapon:
                    if (bWeapon)
                        bAddItem = true;
                    break;
                case RE::FormType::Ammo:
                    if (bAmmo)
                        bAddItem = true;
                    break;
                case RE::FormType::KeyMaster:
                    if (bKey)
                        bAddItem = true;
                    break;
                case RE::FormType::AlchemyItem:
                    if (bpotion)
                        bAddItem = true;
                    break;
                case RE::FormType::SoulGem:
                    if (bSoulgem)
                        bAddItem = true;
                    break;
                default:
                    break;
                }

                if (bAddItem) {
                    filtered.push_back(form);
                }
            }

            return filtered;
        }

        std::vector<RE::TESForm*> MergeFormArrays(RE::StaticFunctionTag*, std::vector<RE::TESForm*> a_vec1, std::vector<RE::TESForm*> a_vec2, bool a_bUnique = true)
        {
            if (a_vec1.empty()) {
                return a_vec2;
            }
            if (a_vec2.empty()) {
                return a_vec1;
            }
            std::vector<RE::TESForm*> result;
            if (a_bUnique) {
                std::set<RE::TESForm*> unique_elements;
                std::merge(a_vec1.begin(), a_vec1.end(), a_vec2.begin(), a_vec2.end(), std::inserter(unique_elements, unique_elements.end()));
                std::copy(unique_elements.begin(), unique_elements.end(), back_inserter(result));
            }
            else {
                std::merge(a_vec1.begin(), a_vec1.end(), a_vec2.begin(), a_vec2.end(), std::back_inserter(result));
            }
            return result;
        }

        long removeItemsByFilters(RE::StaticFunctionTag*, RE::TESObjectREFR* a_container_from, std::vector<UINT32> a_allowed_types, RE::BGSListForm* a_excludedForms_l,
                                  std::vector<RE::TESForm*> a_excludedForms_v, std::vector<RE::TESForm*> a_includedForms_v, std::vector<RE::BGSKeyword*> a_excluded_keywords,
                                  bool bExcludeQuestItems = true, bool bExcludeEnchanteds = false, INT32 a_number = amount_t::all, float a_MinWeight = 0.0, INT32 a_MinValue = 0,
                                  INT32 a_TotalValue = 0, RE::TESObjectREFR* a_container_to = nullptr)
        {
            if (!a_container_from) {
                return 0;
            }

            bool bCheckType  = (!a_allowed_types.empty());
            bool bScroll     = (!bCheckType || (std::find(a_allowed_types.begin(), a_allowed_types.end(), (UINT32)RE::FormType::Scroll) != a_allowed_types.end()));
            bool bArmor      = (!bCheckType || (std::find(a_allowed_types.begin(), a_allowed_types.end(), (UINT32)RE::FormType::Armor) != a_allowed_types.end()));
            bool bBook       = (!bCheckType || (std::find(a_allowed_types.begin(), a_allowed_types.end(), (UINT32)RE::FormType::Book) != a_allowed_types.end()));
            bool bIngredient = (!bCheckType || (std::find(a_allowed_types.begin(), a_allowed_types.end(), (UINT32)RE::FormType::Ingredient) != a_allowed_types.end()));
            bool bMisc       = (!bCheckType || (std::find(a_allowed_types.begin(), a_allowed_types.end(), (UINT32)RE::FormType::Misc) != a_allowed_types.end()));
            bool bWeapon     = (!bCheckType || (std::find(a_allowed_types.begin(), a_allowed_types.end(), (UINT32)RE::FormType::Weapon) != a_allowed_types.end()));
            bool bAmmo       = (!bCheckType || (std::find(a_allowed_types.begin(), a_allowed_types.end(), (UINT32)RE::FormType::Ammo) != a_allowed_types.end()));
            bool bKey        = (!bCheckType || (std::find(a_allowed_types.begin(), a_allowed_types.end(), (UINT32)RE::FormType::KeyMaster) != a_allowed_types.end()));
            bool bpotion     = (!bCheckType || (std::find(a_allowed_types.begin(), a_allowed_types.end(), (UINT32)RE::FormType::AlchemyItem) != a_allowed_types.end()));
            bool bSoulgem    = (!bCheckType || (std::find(a_allowed_types.begin(), a_allowed_types.end(), (UINT32)RE::FormType::SoulGem) != a_allowed_types.end()));

            auto inventory          = a_container_from->GetInventory();
            auto itemCounts         = a_container_from->GetInventoryCounts();
            auto iCount             = 0;
            long iValue             = 0;
            long iTotalValue        = 0;
            long iTotalRemovedItems = 0;
            for (const auto& [form, data] : inventory) {
                if (!form->GetPlayable() || form->GetName()[0] == '\0')
                    continue;

                if (!a_includedForms_v.empty()) {
                    if (std::find(a_includedForms_v.begin(), a_includedForms_v.end(), form) == a_includedForms_v.end())
                        continue;
                }

                if (bCheckType) {
                    bool bTypeOK = false;

                    switch (form->GetFormType()) {
                    case RE::FormType::Scroll:
                        if (bScroll)
                            bTypeOK = true;
                        break;
                    case RE::FormType::Armor:
                        if (bArmor)
                            bTypeOK = true;
                        break;
                    case RE::FormType::Book:
                        if (bBook)
                            bTypeOK = true;
                        break;
                    case RE::FormType::Ingredient:
                        if (bIngredient)
                            bTypeOK = true;
                        break;
                    case RE::FormType::Misc:
                        if (bMisc)
                            bTypeOK = true;
                        break;
                    case RE::FormType::Weapon:
                        if (bWeapon)
                            bTypeOK = true;
                        break;
                    case RE::FormType::Ammo:
                        if (bAmmo)
                            bTypeOK = true;
                        break;
                    case RE::FormType::KeyMaster:
                        if (bKey)
                            bTypeOK = true;
                        break;
                    case RE::FormType::AlchemyItem:
                        if (bpotion)
                            bTypeOK = true;
                        break;
                    case RE::FormType::SoulGem:
                        if (bSoulgem)
                            bTypeOK = true;
                        break;
                    default:
                        break;
                    }

                    if (!bTypeOK)
                        continue;
                }

                if ((a_excludedForms_l) && (a_excludedForms_l->forms.size() > 0) && a_excludedForms_l->HasForm(form))
                    continue;

                if (!a_excludedForms_v.empty()) {
                    if (std::find(a_excludedForms_v.begin(), a_excludedForms_v.end(), form) != a_excludedForms_v.end())
                        continue;
                }

                if (!a_excluded_keywords.empty()) {
                    if (form->HasKeywordInArray(a_excluded_keywords, false))
                        continue;
                }

                if (a_MinWeight >= 0) {
                    if (form->GetWeight() <= a_MinWeight)
                        continue;
                }

                if (a_MinValue > 0) {
                    if (form->GetGoldValue() < a_MinValue)
                        continue;
                }

                iCount = 0;
                if (a_number > 0)
                    iCount = std::min(a_number, itemCounts[form]);
                else if (a_number == amount_t::all)
                    iCount = itemCounts[form];
                else if (a_number == amount_t::random) {
                    // srand(time(0));
                    // iCount = (rand() % (itemCounts[form] + 1));

                    std::random_device              rd;                         // obtain a random number from hardware if possible
                    std::mt19937                    gen(rd());                  // seed the generator
                    std::uniform_int_distribution<> distr(0, itemCounts[form]); // define the range

                    iCount = distr(gen); // generate numbers
                }
                if (iCount < 1)
                    continue;

                iValue = 0;
                if (a_TotalValue > 0) {
                    iValue = form->GetGoldValue();
                    if ((iTotalValue + (iValue * iCount)) > a_TotalValue) {
                        while ((iCount > 0) && ((iTotalValue + (iValue * iCount)) > a_TotalValue)) {
                            iCount -= 1;
                        }
                    }
                    if (iCount < 1)
                        continue;
                }
                auto removeReason = (a_container_to) ? RE::ITEM_REMOVE_REASON::kStoreInContainer : RE::ITEM_REMOVE_REASON::kRemove;
                if (bExcludeQuestItems && data.second->IsQuestObject())
                    continue;
                if (bExcludeEnchanteds && (data.second->IsEnchanted()))
                    continue;
                a_container_from->RemoveItem(form, iCount, removeReason, nullptr, a_container_to);
                logger::info("removeItemsByFilters - {} item removed: {}", iCount, form->GetName());
                iTotalRemovedItems += 1;
                if (a_TotalValue > 0) {
                    iTotalValue += (iCount * iValue);
                    if (iTotalValue > a_TotalValue)
                        break;
                }
            }

            return iTotalRemovedItems;
        }

    } // namespace Utility

    bool Utility::RegisterFunctions(RE::BSScript::IVirtualMachine* vm)
    {
        vm->RegisterFunction("Get_Version", moaClass, Get_Version);
        vm->RegisterFunction("GetItemsByType", moaClass, GetItemsByType);
        vm->RegisterFunction("MergeFormArrays", moaClass, MergeFormArrays);
        vm->RegisterFunction("removeItemsByFilters", moaClass, removeItemsByFilters);
        return true;
    }
