// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Centralized Gameplay Tag Library for Windwalker Productions
 * Provides static accessor functions for all native gameplay tags
 * This is the SINGLE SOURCE OF TRUTH for gameplay tags across all plugins
 */
class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWWTagLibrary
{
public:
    // ================================================================
    // MOVEMENT TAGS
    // ================================================================
    
    /** Movement.Stance.Standing - Character is in standing position */
    static const FGameplayTag& Movement_Stance_Standing();
    
    /** Movement.Stance.Crouch - Character is crouched */
    static const FGameplayTag& Movement_Stance_Crouch();
    
    /** Movement.Stance.Prone - Character is lying flat */
    static const FGameplayTag& Movement_Stance_Prone();
    
    /** Movement.Speed.Idle - Character is not moving */
    static const FGameplayTag& Movement_Speed_Idle();
    
    /** Movement.Speed.Walking - Character is walking */
    static const FGameplayTag& Movement_Speed_Walking();
    
    /** Movement.Speed.Running - Character is running */
    static const FGameplayTag& Movement_Speed_Running();
    
    /** Movement.Speed.Sprinting - Character is sprinting */
    static const FGameplayTag& Movement_Speed_Sprinting();
    
    // ================================================================
    // INPUT TAGS
    // ================================================================
    
    /** Input.Mode.Crouch.Toggle - Crouch uses toggle mode */
    static const FGameplayTag& Input_Mode_Crouch_Toggle();
    
    /** Input.Mode.Crouch.Hold - Crouch uses hold mode */
    static const FGameplayTag& Input_Mode_Crouch_Hold();
    
    /** Input.Mode.Sprint.Toggle - Sprint uses toggle mode */
    static const FGameplayTag& Input_Mode_Sprint_Toggle();
    
    /** Input.Mode.Sprint.Hold - Sprint uses hold mode */
    static const FGameplayTag& Input_Mode_Sprint_Hold();
    
    /** Input.Mode.Aim.Toggle - Aim uses toggle mode */
    static const FGameplayTag& Input_Mode_Aim_Toggle();
    
    /** Input.Mode.Aim.Hold - Aim uses hold mode */
    static const FGameplayTag& Input_Mode_Aim_Hold();
    
    /** Input.State.Crouch.Pressed - Crouch button was pressed */
    static const FGameplayTag& Input_State_Crouch_Pressed();
    
    /** Input.State.Crouch.Held - Crouch button is being held */
    static const FGameplayTag& Input_State_Crouch_Held();
    
    /** Input.State.Sprint.Pressed - Sprint button was pressed */
    static const FGameplayTag& Input_State_Sprint_Pressed();
    
    /** Input.State.Sprint.Held - Sprint button is being held */
    static const FGameplayTag& Input_State_Sprint_Held();
    
    /** Input.State.Jump.Pressed - Jump button was pressed */
    static const FGameplayTag& Input_State_Jump_Pressed();
    
    // ================================================================
    // CHARACTER STATE TAGS
    // ================================================================
    
    /** Character.State.Alive - Character is alive */
    static const FGameplayTag& Character_State_Alive();
    
    /** Character.State.Dead - Character is dead */
    static const FGameplayTag& Character_State_Dead();
    
    /** Character.State.InCombat - Character is in combat */
    static const FGameplayTag& Character_State_InCombat();
    
    /** Character.State.Interacting - Character is interacting with object */
    static const FGameplayTag& Character_State_Interacting();
    
    /** Character.State.Stunned - Character is stunned */
    static const FGameplayTag& Character_State_Stunned();
    
    /** Character.State.Ragdoll - Character is ragdolling */
    static const FGameplayTag& Character_State_Ragdoll();
    
    /** Character.State.Invulnerable - Character cannot take damage */
    static const FGameplayTag& Character_State_Invulnerable();
    
    // ================================================================
    // CAMERA TAGS
    // ================================================================
    
    /** Camera.Mode.FirstPerson - First person camera */
    static const FGameplayTag& Camera_Mode_FirstPerson();

	/*Camera.Mode.TrueFPS - True FPS Cam*/
	static const FGameplayTag& Camera_Mode_TrueFPS();
    
    /** Camera.Mode.ThirdPerson - Third person camera */
    static const FGameplayTag& Camera_Mode_ThirdPerson();
    
    /** Camera.Mode.TopDown - Top down camera */
    static const FGameplayTag& Camera_Mode_TopDown();
	
	/*Camera.Mode.Vehicle - VehicleCam*/
	static const FGameplayTag& Camera_Mode_Vehicle();
	static const FGameplayTag& Camera_Mode_Vehicle_Chase();
	static const FGameplayTag& Camera_Mode_Vehicle_Cockpit();
	static const FGameplayTag& Camera_Mode_Vehicle_Hood();
	static const FGameplayTag& Camera_Mode_Vehicle_Bumper();
	static const FGameplayTag& Camera_Mode_Vehicle_Orbit();
	static const FGameplayTag& Camera_Mode_Vehicle_Cinematic();

	/*Camera.Mode.Station - Station Based Camera
	* A station is whenever player crafts something on an actor:
	* A bench, a station, a vehicle for example
	* Station_Default is whatever default camera mode: TPS 
	 */
	static const FGameplayTag& Camera_Mode_Station_Default();
	static const FGameplayTag& Camera_Mode_Station_Generic();
	static const FGameplayTag& Camera_Mode_Station_Numpad();
	static const FGameplayTag& Camera_Mode_Station_Lockpick();
	static const FGameplayTag& Camera_Mode_Station_Assembly();
	static const FGameplayTag& Camera_Mode_Station_Cooking();


	/*Camera.Mode.Custom - Set of custom camera orientations and positions
	 *based on gameplay state tags*/
	static const FGameplayTag& Camera_Mode_Custom();
	static const FGameplayTag& Camera_Mode_Custom_Default();
	static const FGameplayTag& Camera_Mode_Custom_Killcam();
	static const FGameplayTag& Camera_Mode_Custom_Stealth();
	static const FGameplayTag& Camera_Mode_Custom_Cinematic();

	
    /** Camera.Shoulder - Shoulder camera */
    static const FGameplayTag& Camera_Shoulder_Center();

	static const FGameplayTag& Camera_Shoulder_Left();
	
	static const FGameplayTag& Camera_Shoulder_Right();
	/** Camera.Aim - Camera Aiming State */
	
	/** Camera.Aim.None - Not aiming (default in inpersonated games) */
	static const FGameplayTag& Camera_Aim_None();
	/*Camera.Aim.Aiming - Aiming with the intent of aim (only aiming button active in inpersonated games)*/
	static const FGameplayTag& Camera_Aim_Aiming();

	/** Camera.Aim.Hip - Hip fire, usualy firing without aim input */
	static const FGameplayTag& Camera_Aim_Hip();
	/** Camera.Aim.Scope - Aiming with scope */
	static const FGameplayTag& Camera_Aim_Scope();


    /** Camera.State.Zooming - Camera is zooming */
    static const FGameplayTag& Camera_State_Zooming();
    
    /** Camera.State.Transitioning - Camera is transitioning */
    static const FGameplayTag& Camera_State_Transitioning();
    
    /** Camera.State.LockedOn - Camera is locked onto target */
    static const FGameplayTag& Camera_State_LockedOn();

	/*CameraShake Tags*/
	static const FGameplayTag& CameraShake_Combat_Explosion();
	static const FGameplayTag& CameraShake_Combat_Punch();
	static const FGameplayTag& CameraShake_Combat_Grenade();


    
    // ================================================================
    // INTERACTION TAGS
    // ================================================================
    
    /** Interaction.Action.Pickup - Picking up an item */
    static const FGameplayTag& Interaction_Action_Pickup();
    
    /** Interaction.Action.Drop - Dropping an item */
    static const FGameplayTag& Interaction_Action_Drop();
    
    /** Interaction.Action.Use - Using an item */
    static const FGameplayTag& Interaction_Action_Use();
    
    /** Interaction.Action.Equip - Equipping an item */
    static const FGameplayTag& Interaction_Action_Equip();
    
    /** Interaction.Action.Unequip - Unequipping an item */
    static const FGameplayTag& Interaction_Action_Unequip();
    
    /** Interaction.Action.Talk - Talking to NPC */
    static const FGameplayTag& Interaction_Action_Talk();
    
    /** Interaction.Action.Open - Opening container/door */
    static const FGameplayTag& Interaction_Action_Open();
    
    /** Interaction.Action.Close - Closing container/door */
    static const FGameplayTag& Interaction_Action_Close();
    
    /** Interaction.Action.Examine - Examining object */
    static const FGameplayTag& Interaction_Action_Examine();
    
    /** Interaction.State.Available - Can interact */
    static const FGameplayTag& Interaction_State_Available();
    
    /** Interaction.State.InProgress - Currently interacting */
    static const FGameplayTag& Interaction_State_InProgress();
    
    /** Interaction.State.Holding - Hold interaction active */
    static const FGameplayTag& Interaction_State_Holding();
    
    /** Interaction.State.Mashing - Mash interaction active */
    static const FGameplayTag& Interaction_State_Mashing();
    
    /** Interaction.State.Cooldown - Interaction on cooldown */
    static const FGameplayTag& Interaction_State_Cooldown();
    
    // ================================================================
    // INVENTORY TAGS
    // ================================================================
	
	
	/** Inventory.Item.Type.Weapon - Weapon item */
	static const FGameplayTag& Inventory_Item_Type_Attachment();

	/** Inventory.Item.Type.Weapon - Weapon item */
	static const FGameplayTag& Inventory_Item_Type_Wearable();

	/** Inventory.Item.Type.Weapon - Weapon item */
	static const FGameplayTag& Inventory_Item_Type_Throwable();

    /** Inventory.Item.Type.Weapon - Weapon item */
    static const FGameplayTag& Inventory_Item_Type_Weapon();
    
    /** Inventory.Item.Type.Armor - Armor item */
    static const FGameplayTag& Inventory_Item_Type_Armor();
    
    /** Inventory.Item.Type.Consumable - Consumable item */
    static const FGameplayTag& Inventory_Item_Type_Consumable();
    
    /** Inventory.Item.Type.KeyItem - Key item */
    static const FGameplayTag& Inventory_Item_Type_KeyItem();
    
    /** Inventory.Item.Type.Crafting - Crafting material */
    static const FGameplayTag& Inventory_Item_Type_Crafting();
    
    /** Inventory.Item.Type.Quest - Quest item */
    static const FGameplayTag& Inventory_Item_Type_Quest();
    
    /** Inventory.Item.Rarity.Common - Common rarity */
    static const FGameplayTag& Inventory_Item_Rarity_Common();
    
    /** Inventory.Item.Rarity.Uncommon - Uncommon rarity */
    static const FGameplayTag& Inventory_Item_Rarity_Uncommon();
    
    /** Inventory.Item.Rarity.Rare - Rare rarity */
    static const FGameplayTag& Inventory_Item_Rarity_Rare();
    
    /** Inventory.Item.Rarity.Epic - Epic rarity */
    static const FGameplayTag& Inventory_Item_Rarity_Epic();
    
    /** Inventory.Item.Rarity.Legendary - Legendary rarity */
    static const FGameplayTag& Inventory_Item_Rarity_Legendary();
// ================================================================
// SLOT TAGS
// ================================================================
	
    
    /** Inventory.Slot.MainHand - Main hand weapon slot */
    static const FGameplayTag& Inventory_Slot_MainHand();
    
    /** Inventory.Slot.OffHand - Off hand weapon slot */
    static const FGameplayTag& Inventory_Slot_OffHand();
    
	/** Inventory.Slot.QuickSlot.0 - Quick slot 0 */
	static const FGameplayTag& Inventory_Slot_QuickSlot_0();

	/** Inventory.Slot.QuickSlot.1 - Quick slot 1 */
	static const FGameplayTag& Inventory_Slot_QuickSlot_1();

	/** Inventory.Slot.QuickSlot.2 - Quick slot 2 */
	static const FGameplayTag& Inventory_Slot_QuickSlot_2();

	/** Inventory.Slot.QuickSlot.3 - Quick slot 3 */
	static const FGameplayTag& Inventory_Slot_QuickSlot_3();

	/** Inventory.Slot.QuickSlot.4 - Quick slot 4 */
	static const FGameplayTag& Inventory_Slot_QuickSlot_4();

	/** Inventory.Slot.QuickSlot.5 - Quick slot 5 */
	static const FGameplayTag& Inventory_Slot_QuickSlot_5();

	/** Inventory.Slot.QuickSlot.6 - Quick slot 6 */
	static const FGameplayTag& Inventory_Slot_QuickSlot_6();

	/** Inventory.Slot.QuickSlot.7 - Quick slot 7 */
	static const FGameplayTag& Inventory_Slot_QuickSlot_7();

	/** Inventory.Slot.QuickSlot.8 - Quick slot 8 */
	static const FGameplayTag& Inventory_Slot_QuickSlot_8();

	/** Inventory.Slot.QuickSlot.9 - Quick slot 9 */
	static const FGameplayTag& Inventory_Slot_QuickSlot_9();

	//Wearables
	
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Head();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Torso();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Gloves();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Legs();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Shoes();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Armor_Top();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Armor_Bottom();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Armor_Head();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Armor_Gloves();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Undearwear();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Accessory_Chest();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Accessory_Ear_Left();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Accessory_Ear_Right();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Accessory_Arm_Left();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Accessory_Arm_Right();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Accessory_Ring_Left();
	/** Inventory.Slot.OffHand - Off hand weapon slot */
	static const FGameplayTag& Inventory_Slot_Wearable_Accessory_Ring_Right();

	
    // ================================================================
    // INVENTORY BEHAVIOR TAGS (Item use behaviors)
    // ================================================================

    /** Inventory.Item.Behavior.Equip - Equip item to hand (weapons/tools) */
    static const FGameplayTag& Inventory_Item_Behavior_Equip();

    /** Inventory.Item.Behavior.Consume - Instant use consumable (drugs/food) */
    static const FGameplayTag& Inventory_Item_Behavior_Consume();

	/** Inventory.Item.Behavior.Wear - Determiner for wearables */
	static const FGameplayTag& Inventory_Item_Behavior_Wear();

    /** Inventory.Item.Behavior.Throw - Throwable item (grenades) */
    static const FGameplayTag& Inventory_Item_Behavior_Throw();

    /** Inventory.Item.Behavior.Activate - Activate item (phone/key) */
    static const FGameplayTag& Inventory_Item_Behavior_Activate();

    /** Inventory.Item.Behavior.Toggle - Toggle item (flashlight) */
    static const FGameplayTag& Inventory_Item_Behavior_Toggle();
    
    // ================================================================
    // INVENTORY BEHAVIOR TAGS (Context Menu Actions)
    // ================================================================

    static const FGameplayTag& Inventory_Item_Behavior_Unequip();
    static const FGameplayTag& Inventory_Item_Behavior_Use();
    static const FGameplayTag& Inventory_Item_Behavior_Split();
    static const FGameplayTag& Inventory_Item_Behavior_Drop();


    
    // ================================================================
    // INVENTORY FLAG TAGS (Item restrictions)
    // ================================================================

    /** Inventory.Item.Flags.NoTrade - Cannot be traded/sold */
    static const FGameplayTag& Inventory_Item_Flags_NoTrade();

    /** Inventory.Item.Flags.NoDrop - Cannot be dropped */
    static const FGameplayTag& Inventory_Item_Flags_NoDrop();

    /** Inventory.Item.Flags.Stolen - Marked as stolen (police interaction) */
    static const FGameplayTag& Inventory_Item_Flags_Stolen();

    /** Inventory.Item.Flags.QuestItem - Quest-related item */
    static const FGameplayTag& Inventory_Item_Flags_QuestItem();

    // ================================================================
    // INVENTORY TYPE TAGS (Slot location types)
    // ================================================================

    /** Inventory.Type.PlayerInventory - Main player backpack */
    static const FGameplayTag& Inventory_Type_PlayerInventory();

    /** Inventory.Type.Equipment - Equipment/hotbar slots */
    static const FGameplayTag& Inventory_Type_Equipment();

    /** Inventory.Type.Container - Container/chest inventory */
    static const FGameplayTag& Inventory_Type_Container();

	// ================================================================
	// SIMULATOR FRAMEWORK TAGS
	// ================================================================
	
	/** Device State Tag Accessors */
	
	static const FGameplayTag& Simulator_Device_State_Off();
	static const FGameplayTag& Simulator_Device_State_Idle();
	static const FGameplayTag& Simulator_Device_State_InUse();
	static const FGameplayTag& Simulator_Device_State_Broken();
	static const FGameplayTag& Simulator_Device_State_Maintenance();
	static const FGameplayTag& Simulator_Device_State_NoPower();

	/* Application State Tag Accessors*/
	static const FGameplayTag& Simulator_Application_State_Closed();
	static const FGameplayTag& Simulator_Application_State_Opening();
	static const FGameplayTag& Simulator_Application_State_Open();
	static const FGameplayTag& Simulator_Application_State_Minimized();
	static const FGameplayTag& Simulator_Application_State_Closing();

	
	/*  MINI-GAME TAGS (V2.11)*/

	// --- Types ---
	static const FGameplayTag& Simulator_MiniGame_Type_Manipulation();
	static const FGameplayTag& Simulator_MiniGame_Type_Lockpick();
	static const FGameplayTag& Simulator_MiniGame_Type_Sequence();
	static const FGameplayTag& Simulator_MiniGame_Type_Timing();
	static const FGameplayTag& Simulator_MiniGame_Type_Calibration();
	static const FGameplayTag& Simulator_MiniGame_Type_Temperature();

	// --- Objectives ---
	static const FGameplayTag& Simulator_MiniGame_Objective_ItemSnapped();
	static const FGameplayTag& Simulator_MiniGame_Objective_AllPartsAssembled();
	static const FGameplayTag& Simulator_MiniGame_Objective_LockOpened();
	static const FGameplayTag& Simulator_MiniGame_Objective_CodeEntered();
	static const FGameplayTag& Simulator_MiniGame_Objective_SequenceComplete();
	static const FGameplayTag& Simulator_MiniGame_Objective_TemperatureMaintained();
	static const FGameplayTag& Simulator_MiniGame_Objective_CalibrationHeld();
	static const FGameplayTag& Simulator_MiniGame_Objective_TimingHit();

	// --- MiniGame IDs ---
	static const FGameplayTag& Simulator_MiniGame_ID_Sequence_VaultNumpad();
	static const FGameplayTag& Simulator_MiniGame_ID_Sequence_Keypad();
	static const FGameplayTag& Simulator_MiniGame_ID_Lockpick_Standard();
	static const FGameplayTag& Simulator_MiniGame_ID_Lockpick_Advanced();
	static const FGameplayTag& Simulator_MiniGame_ID_Lockpick_Master();
	static const FGameplayTag& Simulator_MiniGame_ID_Assembly_CarEngine();
	static const FGameplayTag& Simulator_MiniGame_ID_Assembly_Weapon();
	static const FGameplayTag& Simulator_MiniGame_ID_Temperature_CookingSteak();
	static const FGameplayTag& Simulator_MiniGame_ID_Temperature_Smithing();
	static const FGameplayTag& Simulator_MiniGame_ID_Timing_SafeDial();
	static const FGameplayTag& Simulator_MiniGame_ID_Timing_Tumbler();
	static const FGameplayTag& Simulator_MiniGame_ID_Calibration_Scope();
	static const FGameplayTag& Simulator_MiniGame_ID_Calibration_Radio();

	// --- Input: Numpad ---
	static const FGameplayTag& Input_Numpad_0();
	static const FGameplayTag& Input_Numpad_1();
	static const FGameplayTag& Input_Numpad_2();
	static const FGameplayTag& Input_Numpad_3();
	static const FGameplayTag& Input_Numpad_4();
	static const FGameplayTag& Input_Numpad_5();
	static const FGameplayTag& Input_Numpad_6();
	static const FGameplayTag& Input_Numpad_7();
	static const FGameplayTag& Input_Numpad_8();
	static const FGameplayTag& Input_Numpad_9();
	static const FGameplayTag& Input_Numpad_Clear();
	static const FGameplayTag& Input_Numpad_Enter();

	// --- Input: QTE ---
	static const FGameplayTag& Input_QTE_North();
	static const FGameplayTag& Input_QTE_South();
	static const FGameplayTag& Input_QTE_East();
	static const FGameplayTag& Input_QTE_West();
	
    // ================================================================
    // UI MODE TAGS
    // ================================================================

    /*UI Mode is for tracking and setting in which mode the player is currently in.
     *This will be used to change the input mode in ModularPlayerController_Master module.
     */
    /** UI.Mode.Game - Normal gameplay mode (no UI overlays) */
    static const FGameplayTag& UI_Mode_Game();

    /** UI.Mode.Inventory - Inventory UI is open */
    static const FGameplayTag& UI_Mode_Inventory();

    /** UI.Mode.Crafting - Crafting UI is open */
    static const FGameplayTag& UI_Mode_Crafting();

    /** UI.Mode.Shop - Shop/Trading UI is open */
    static const FGameplayTag& UI_Mode_Shop();

    /** UI.Mode.Menu - Pause menu is open */
    static const FGameplayTag& UI_Mode_Menu();

    // ============================================================================
    // UI WIDGET CATEGORIES (for ESC priority system)
    // ============================================================================

    //Desc: Used for tracking the top ui and destroy them with ESC_ContextAction in MPC_Character_Master. 
    /** Context menus (right-click, split, drop) - Priority: 100 */
    UFUNCTION(BlueprintPure, Category = "Tags|UI|Widget")
    static FGameplayTag UI_Widget_Category_ContextMenu();

    /** Modal dialogs (confirmations, warnings) - Priority: 90 */
    UFUNCTION(BlueprintPure, Category = "Tags|UI|Widget")
    static FGameplayTag UI_Widget_Category_Modal(); 

    /** Windows (inventory, shop, exchange) - Priority: 50 */
    UFUNCTION(BlueprintPure, Category = "Tags|UI|Widget")
    static FGameplayTag UI_Widget_Category_Window();
  

    /** Pause menu - Priority: 10 */
    UFUNCTION(BlueprintPure, Category = "Tags|UI|Widget")
    static FGameplayTag UI_Widget_Category_PauseMenu(); 
  

    /** HUD elements (health, stamina) - Priority: 0 (never closes) */
    UFUNCTION(BlueprintPure, Category = "Tags|UI|Widget")
    static FGameplayTag UI_Widget_Category_HUD();

    // ============================================================================
    // UI - WIDGET STATE MACHINE STATES
    // ============================================================================

    /** Widget is not visible, not in viewport */
    static const FGameplayTag& UI_Widget_State_Closed();

    /** Widget is transitioning to visible (animate-in) */
    static const FGameplayTag& UI_Widget_State_AnimatingIn();

    /** Widget is fully visible and interactive */
    static const FGameplayTag& UI_Widget_State_Visible();

    /** Widget is hidden by higher-priority widget, will resume */
    static const FGameplayTag& UI_Widget_State_Paused();

    /** Widget is transitioning to closed (animate-out) */
    static const FGameplayTag& UI_Widget_State_AnimatingOut();

    // ============================================================================
    // UI - DOCK ZONE TAGS (parent hierarchy for user-defined zones)
    // ============================================================================

    /** UI.Dock.Zone - Parent tag for all dock zones */
    static const FGameplayTag& UI_Dock_Zone();

// ============================================================================
// UI - INVENTORY SORT TAGS
// ============================================================================

/**
 * Sort mode tags for inventory search/sort widget
 * These tags represent different sort modes that can be cycled through
 */

// Custom sort (original/manual order - default state)
    static FGameplayTag UI_Inventory_Sort_Custom();

    // Name sorting
    static FGameplayTag UI_Inventory_Sort_Name_Ascending();

    static FGameplayTag UI_Inventory_Sort_Name_Descending();

    // Rarity sorting
    static FGameplayTag UI_Inventory_Sort_Rarity_Ascending();

    static FGameplayTag UI_Inventory_Sort_Rarity_Descending();

    // Type sorting
    static FGameplayTag UI_Inventory_Sort_Type_Ascending();

    static FGameplayTag UI_Inventory_Sort_Type_Descending();

    // Weight sorting
    static FGameplayTag UI_Inventory_Sort_Weight_Ascending();

    static FGameplayTag UI_Inventory_Sort_Weight_Descending();

    // Value sorting
    static FGameplayTag UI_Inventory_Sort_Value_Ascending();

    static FGameplayTag UI_Inventory_Sort_Value_Descending();

    // Quantity sorting
    static FGameplayTag UI_Inventory_Sort_Quantity_Ascending();
    static FGameplayTag UI_Inventory_Sort_Quantity_Descending();

    // Search mode (auto-activated when search is active, not in cycle)
    static FGameplayTag UI_Inventory_Sort_Search();
    


    // ================================================================
    // CHEAT TAGS
    // ================================================================
    
    /** Cheat.Permission.None - No cheat permission */
    static const FGameplayTag& Cheat_Permission_None();
    
    /** Cheat.Permission.User - User level cheats */
    static const FGameplayTag& Cheat_Permission_User();
    
    /** Cheat.Permission.Developer - Developer level cheats */
    static const FGameplayTag& Cheat_Permission_Developer();
    
    /** Cheat.Permission.Admin - Admin level cheats */
    static const FGameplayTag& Cheat_Permission_Admin();
    
    /** Cheat.Permission.GodMode - God mode cheats */
    static const FGameplayTag& Cheat_Permission_GodMode();
    
    // ================================================================
    // DEBUG TAGS
    // ================================================================
    
    /** Debug - Root debug tag */
    static const FGameplayTag& Debug();

    /** Debug - Debug_InputTag */
    static const FGameplayTag& Debug_Input();
    
    /** Debug.AI - Debug layer for AI system */
    static const FGameplayTag& Debug_AI();
    
    /** Debug.Camera - Debug layer for camera system */
    static const FGameplayTag& Debug_Camera();
    
    /** Debug.Character - Debug layer for character system */
    static const FGameplayTag& Debug_Character();
    
    /** Debug.Combat - Debug layer for combat system */
    static const FGameplayTag& Debug_Combat();
    
    /** Debug.CQC - Debug layer for Close Quarters Combat */
    static const FGameplayTag& Debug_CQC();
    
    /** Debug.Interaction - Debug layer for interaction system */
    static const FGameplayTag& Debug_Interaction();
    
    /** Debug.Inventory - Debug layer for inventory system */
    static const FGameplayTag& Debug_Inventory();
    
    /** Debug.InventoryInteraction - Debug layer for inventory interaction */
    static const FGameplayTag& Debug_InventoryInteraction();
    
    /** Debug.Movement - Debug layer for movement system */
    static const FGameplayTag& Debug_Movement();
    
    /** Debug.Parkour - Debug layer for parkour system */
    static const FGameplayTag& Debug_Parkour();
    
    /** Debug.Pawn - Debug layer for pawn system */
    static const FGameplayTag& Debug_Pawn();
    
    /** Debug.SaveSystem - Debug layer for save system */
    static const FGameplayTag& Debug_SaveSystem();
    
    /** Debug.Widget - Debug layer for Widgets */
    static const FGameplayTag& Debug_Widget();
    
};


