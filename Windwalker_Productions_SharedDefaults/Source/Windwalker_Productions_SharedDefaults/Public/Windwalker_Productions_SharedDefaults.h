// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

class FWindwalker_Productions_SharedDefaultsModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

// Backward compatibility namespace - getter functions that forward to FWWTagLibrary
namespace WWDebugTags
{
    /** Debug - Root debug tag */
    static inline const FGameplayTag& Debug() { return FWWTagLibrary::Debug(); }
    
    /** Debug.AI - Debug layer for AI system */
    static inline const FGameplayTag& Debug_AI() { return FWWTagLibrary::Debug_AI(); }
    
    /** Debug.Camera - Debug layer for camera system */
    static inline const FGameplayTag& Debug_Camera() { return FWWTagLibrary::Debug_Camera(); }
    
    /** Debug.Character - Debug layer for character system */
    static inline const FGameplayTag& Debug_Character() { return FWWTagLibrary::Debug_Character(); }
    
    /** Debug.Combat - Debug layer for combat system */
    static inline const FGameplayTag& Debug_Combat() { return FWWTagLibrary::Debug_Combat(); }
    
    /** Debug.CQC - Debug layer for Close Quarters Combat */
    static inline const FGameplayTag& Debug_CQC() { return FWWTagLibrary::Debug_CQC(); }
    
    /** Debug.Interaction - Debug layer for interaction system */
    static inline const FGameplayTag& Debug_Interaction() { return FWWTagLibrary::Debug_Interaction(); }
    
    /** Debug.Inventory - Debug layer for inventory system */
    static inline const FGameplayTag& Debug_Inventory() { return FWWTagLibrary::Debug_Inventory(); }
    
    /** Debug.InventoryInteraction - Debug layer for inventory interaction */
    static inline const FGameplayTag& Debug_InventoryInteraction() { return FWWTagLibrary::Debug_InventoryInteraction(); }
    
    /** Debug.Movement - Debug layer for movement system */
    static inline const FGameplayTag& Debug_Movement() { return FWWTagLibrary::Debug_Movement(); }
    
    /** Debug.Parkour - Debug layer for parkour system */
    static inline const FGameplayTag& Debug_Parkour() { return FWWTagLibrary::Debug_Parkour(); }
    
    /** Debug.Pawn - Debug layer for pawn system */
    static inline const FGameplayTag& Debug_Pawn() { return FWWTagLibrary::Debug_Pawn(); }
    
    /** Debug.SaveSystem - Debug layer for save system */
    static inline const FGameplayTag& Debug_SaveSystem() { return FWWTagLibrary::Debug_SaveSystem(); }
    
    /** Debug.Widget - Debug layer for Widgets */
    static inline const FGameplayTag& Debug_Widget() { return FWWTagLibrary::Debug_Widget(); }
}

namespace  WWWidgetZOrder
{
    static constexpr int32 Z_ORDER_BASE = 0;
    static constexpr int32 Z_ORDER_HUD = 100;
    static constexpr int32 Z_ORDER_POPUP = 200;
    static constexpr int32 Z_ORDER_DRAG = 300;
    static constexpr int32 Z_ORDER_MODAL = 400;
}

namespace WWCameraDefaults
{
    /*Every Pawn controllable by a playercontroller
    * has to have "head" socket in order for a fps camera to be attached.
    */
    static constexpr FName CAMERA_FPS_SOCKET = "head";
    /*Every Pawn controllable by a playercontroller
    * has to have "" socket in order for a tps camera to be attached.
    */
    static constexpr FName CAMERA_TPS_Socket = "";
}

namespace WWPluginDirectories
{
    /*
     *In order for datatable auto population system to work, we must first
     *Have directory paths set as defaults
     * Every plugin that has this system has this workflow:
     *  *Json file in data for base population
     *  *DataTable Populated based on Json File
     *Actors setting their configuration based on data table.
     *Therefore, the path is PluginName/Data/ for the most convenient and
     *easiest way of setting it up.
     */
    
    // ============================================================
    // Modular Interaction System Interactable Path CONSTANTS
    // ============================================================
    
    static constexpr const TCHAR* PLUGIN_NAME_MODULARINTERACTIONSYSTEM = TEXT("ModularInteractionSystem");
    static constexpr const TCHAR* JSON_FILE_NAME_INTERACTABLES = TEXT("Interactables.json");
    static constexpr const TCHAR* DATATABLE_NAME_INTERACTABLES = TEXT("DT_Interactables");
    static constexpr const TCHAR* JSON_ARRAY_KEY_INTERACTABLES = TEXT("Interactables");
    static constexpr const TCHAR* ROW_NAME_FIELD_INTERACTABLE = TEXT("InteractableID");

    // ============================================================
    // Modular Inventory System Craftable Recipe Directory CONSTANTS
    // ============================================================
    
    static constexpr const TCHAR* PLUGIN_NAME_MODULARINVENTORYSYSTEM = TEXT("ModularInventorySystem");
    static constexpr const TCHAR* JSON_FILE_NAME_RECIPEITEMS = TEXT("RecipeItems.json");
    static constexpr const TCHAR* DATATABLE_NAME_RECIPEITEMS = TEXT("DT_RecipeItems");
    static constexpr const TCHAR* JSON_ARRAY_KEY_RECIPES = TEXT("Recipes");
    static constexpr const TCHAR* ROW_NAME_FIELD_RECIPE = TEXT("RecipeID");

    // ============================================================
    // Modular Inventory System Item Directory Constants
    // ============================================================

    /*Since the inventory and craftable system uses the same plugin,
     * There is no need redefinition here
     */
    //static constexpr const TCHAR* PLUGIN_NAME_MODULARINVENTORYSYSTEM = TEXT("ModularInventorySystem");
    static constexpr const TCHAR* JSON_FILE_NAME_INVENTORYITEMS = TEXT("InventoryItems.json");
    static constexpr const TCHAR* DATATABLE_NAME_INVENTORYITEMS = TEXT("DT_InventoryItems");
    static constexpr const TCHAR* JSON_ARRAY_KEY_ITEMS = TEXT("Items");
    static constexpr const TCHAR* ROW_NAME_FIELD_ITEMID = TEXT("ItemID");

    /*Crafters: Combination of capabilites and levels.
    *Basically a datacontainer used by craftercomponent
    */
    static constexpr const TCHAR* JSON_FILE_NAME_CRAFTER = TEXT("CRAFTER.json");
    static constexpr const TCHAR* DATATABLE_NAME_CRAFTER = TEXT("DT_CRAFTER");
    static constexpr const TCHAR* JSON_ARRAY_KEY_CRAFTERS = TEXT("Crafters");
    static constexpr const TCHAR* ROW_NAME_FIELD_CRAFTERID = TEXT("CrafterID");
}

namespace InteractionDefaultsConstants
{
    const FName PoolName_InteractionPrompts = FName("InteractionPrompts");
    const FName PoolName_PreInteractionDots = FName("PreInteractionDots");

    const FText Widget_DefaultInteractionText = FText::FromString(TEXT("Interact"));
    const FColor Widget_DefaultPreInteractionColor = FColor::White;

    /*TODO: define stencil color outline integers */
    static constexpr int32 OUTLINE_DEFAULT_COLOR = 1;
}

//Names space for both inventory and item 
namespace InventoryItemDefaultsConstants
{
    static constexpr int32 MAX_QUICKSLOTS_NUMBER = 10;
    static constexpr int32 MAX_WEARABLES_NUMBER = 8;
    static constexpr float DEFAULT_INVENTORY_ITEM_SPAWN_DISTANCE = 100.0f;

}