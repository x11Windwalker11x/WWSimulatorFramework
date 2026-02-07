// Copyright Windwalker Productions. All Rights Reserved.

#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "NativeGameplayTags.h"

// Define all native tags in an internal namespace
// These are private to this translation unit
namespace WW_Internal
{
    // MOVEMENT TAGS
    UE_DEFINE_GAMEPLAY_TAG(Movement_Stance_Standing, "Movement.Stance.Standing");
    UE_DEFINE_GAMEPLAY_TAG(Movement_Stance_Crouch, "Movement.Stance.Crouch");
    UE_DEFINE_GAMEPLAY_TAG(Movement_Stance_Prone, "Movement.Stance.Prone");
    UE_DEFINE_GAMEPLAY_TAG(Movement_Speed_Idle, "Movement.Speed.Idle");
    UE_DEFINE_GAMEPLAY_TAG(Movement_Speed_Walking, "Movement.Speed.Walking");
    UE_DEFINE_GAMEPLAY_TAG(Movement_Speed_Running, "Movement.Speed.Running");
    UE_DEFINE_GAMEPLAY_TAG(Movement_Speed_Sprinting, "Movement.Speed.Sprinting");
    
    // INPUT TAGS
    UE_DEFINE_GAMEPLAY_TAG(Input_Mode_Crouch_Toggle, "Input.Mode.Crouch.Toggle");
    UE_DEFINE_GAMEPLAY_TAG(Input_Mode_Crouch_Hold, "Input.Mode.Crouch.Hold");
    UE_DEFINE_GAMEPLAY_TAG(Input_Mode_Sprint_Toggle, "Input.Mode.Sprint.Toggle");
    UE_DEFINE_GAMEPLAY_TAG(Input_Mode_Sprint_Hold, "Input.Mode.Sprint.Hold");
    UE_DEFINE_GAMEPLAY_TAG(Input_Mode_Aim_Toggle, "Input.Mode.Aim.Toggle");
    UE_DEFINE_GAMEPLAY_TAG(Input_Mode_Aim_Hold, "Input.Mode.Aim.Hold");
    UE_DEFINE_GAMEPLAY_TAG(Input_State_Crouch_Pressed, "Input.State.Crouch.Pressed");
    UE_DEFINE_GAMEPLAY_TAG(Input_State_Crouch_Held, "Input.State.Crouch.Held");
    UE_DEFINE_GAMEPLAY_TAG(Input_State_Sprint_Pressed, "Input.State.Sprint.Pressed");
    UE_DEFINE_GAMEPLAY_TAG(Input_State_Sprint_Held, "Input.State.Sprint.Held");
    UE_DEFINE_GAMEPLAY_TAG(Input_State_Jump_Pressed, "Input.State.Jump.Pressed");
    
    // CHARACTER STATE TAGS
    UE_DEFINE_GAMEPLAY_TAG(Character_State_Alive, "Character.State.Alive");
    UE_DEFINE_GAMEPLAY_TAG(Character_State_Dead, "Character.State.Dead");
    UE_DEFINE_GAMEPLAY_TAG(Character_State_InCombat, "Character.State.InCombat");
    UE_DEFINE_GAMEPLAY_TAG(Character_State_Interacting, "Character.State.Interacting");
    UE_DEFINE_GAMEPLAY_TAG(Character_State_Stunned, "Character.State.Stunned");
    UE_DEFINE_GAMEPLAY_TAG(Character_State_Ragdoll, "Character.State.Ragdoll");
    UE_DEFINE_GAMEPLAY_TAG(Character_State_Invulnerable, "Character.State.Invulnerable");
    
    // CAMERA TAGS
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Default, "Camera.Mode.Default");
    
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_FirstPerson, "Camera.Mode.FirstPerson");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_ThirdPerson, "Camera.Mode.ThirdPerson");
    
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_TrueFPS, "Camera.Mode.TrueFPS");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_TopDown, "Camera.Mode.TopDown");
    
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Vehicle, "Camera.Mode.Vehicle");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Vehicle_Chase, "Camera.Mode.Vehicle.Chase");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Vehicle_Cockpit, "Camera.Mode.Vehicle.Cockpit");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Vehicle_Hood, "Camera.Mode.Vehicle.Hood");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Vehicle_Bumper, "Camera.Mode.Vehicle.Bumper");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Vehicle_Orbit, "Camera.Mode.Vehicle.Orbit");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Vehicle_Cinematic, "Camera.Mode.Vehicle.Cinematic");
    
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Station_Generic, "Camera.Mode.Station.Generic");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Station_Default, "Camera.Mode.Station.Generic");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Station_Numpad, "Camera.Mode.Station.Numpad");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Station_Lockpick, "Camera.Mode.Station.Lockpick");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Station_Assembly, "Camera.Mode.Station.Assembly");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Station_Cooking, "Camera.Mode.Station.Cooking");

    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Custom, "Camera.Mode.Custom");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Custom_Default, "Camera.Mode.Custom.Default");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Custom_KillCam, "Camera.Mode.Custom.KillCam");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Custom_Stealth, "Camera.Mode.Custom.Stealth");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Mode_Custom_Cinematic, "Camera.Mode.Custom.Cinematic");
    
    UE_DEFINE_GAMEPLAY_TAG(Camera_Shoulder_Center, "Camera.Shoulder.Center");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Shoulder_Left, "Camera.Shoulder.Left");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Shoulder_Right, "Camera.Shoulder.Right");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Aim_None, "Camera.Aim.None");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Aim_Hip, "Camera.Aim.Hip");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Aim_Scope, "Camera.Aim.Scope");
    UE_DEFINE_GAMEPLAY_TAG(Camera_Aim_Aiming, "Camera.Aim.Aiming");


    UE_DEFINE_GAMEPLAY_TAG(Camera_State_Zooming, "Camera.State.Zooming");
    UE_DEFINE_GAMEPLAY_TAG(Camera_State_Transitioning, "Camera.State.Transitioning");
    UE_DEFINE_GAMEPLAY_TAG(Camera_State_LockedOn, "Camera.State.LockedOn");

    UE_DEFINE_GAMEPLAY_TAG(CameraShake_Combat_Explosion, "CameraShake.Combat.Explosion");
    UE_DEFINE_GAMEPLAY_TAG(CameraShake_Combat_Punch, "CameraShake.Combat.Punch");
    UE_DEFINE_GAMEPLAY_TAG(CameraShake_Combat_Grenade, "CameraShake.Combat.Grenade");



    
    
    // INTERACTION TAGS
    UE_DEFINE_GAMEPLAY_TAG(Interaction_Action_Pickup, "Interaction.Action.Pickup");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_Action_Drop, "Interaction.Action.Drop");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_Action_Use, "Interaction.Action.Use");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_Action_Equip, "Interaction.Action.Equip");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_Action_Unequip, "Interaction.Action.Unequip");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_Action_Talk, "Interaction.Action.Talk");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_Action_Open, "Interaction.Action.Open");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_Action_Close, "Interaction.Action.Close");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_Action_Examine, "Interaction.Action.Examine");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_State_Available, "Interaction.State.Available");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_State_InProgress, "Interaction.State.InProgress");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_State_Holding, "Interaction.State.Holding");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_State_Mashing, "Interaction.State.Mashing");
    UE_DEFINE_GAMEPLAY_TAG(Interaction_State_Cooldown, "Interaction.State.Cooldown");
    
    // ITEM TAGS (Pickupable world items)
    UE_DEFINE_GAMEPLAY_TAG(Item_Type_Weapon_Ranged, "Item.Type.Weapon.Ranged");
    UE_DEFINE_GAMEPLAY_TAG(Item_Type_Weapon_Melee, "Item.Type.Weapon.Melee");
    UE_DEFINE_GAMEPLAY_TAG(Item_Type_Throwable, "Item.Type.Throwable");
    UE_DEFINE_GAMEPLAY_TAG(Item_Type_Armor, "Item.Type.Armor");
    UE_DEFINE_GAMEPLAY_TAG(Item_Type_Consumable, "Item.Type.Consumable");
    UE_DEFINE_GAMEPLAY_TAG(Item_Rarity_Common, "Item.Rarity.Common");
    UE_DEFINE_GAMEPLAY_TAG(Item_Rarity_Uncommon, "Item.Rarity.Uncommon");
    UE_DEFINE_GAMEPLAY_TAG(Item_Rarity_Rare, "Item.Rarity.Rare");
    UE_DEFINE_GAMEPLAY_TAG(Item_Rarity_Epic, "Item.Rarity.Epic");
    UE_DEFINE_GAMEPLAY_TAG(Item_Rarity_Legendary, "Item.Rarity.Legendary");

    // INVENTORY TAGS
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Type_Attachment, "Inventory.Item.Type.Attachment");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Type_Wearable, "Inventory.Item.Type.Wearable");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Type_Throwable, "Inventory.Item.Type.Throwable");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Type_Weapon, "Inventory.Item.Type.Weapon");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Type_Armor, "Inventory.Item.Type.Armor");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Type_Consumable, "Inventory.Item.Type.Consumable");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Type_KeyItem, "Inventory.Item.Type.KeyItem");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Type_Crafting, "Inventory.Item.Type.Crafting");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Type_Quest, "Inventory.Item.Type.Quest");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Rarity_Common, "Inventory.Item.Rarity.Common");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Rarity_Uncommon, "Inventory.Item.Rarity.Uncommon");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Rarity_Rare, "Inventory.Item.Rarity.Rare");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Rarity_Epic, "Inventory.Item.Rarity.Epic");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Rarity_Legendary, "Inventory.Item.Rarity.Legendary");
    //Slot Tags
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_MainHand, "Inventory.Slot.MainHand");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_OffHand, "Inventory.Slot.OffHand");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_QuickSlot_0, "Inventory.Slot.QuickSlot.0");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_QuickSlot_1, "Inventory.Slot.QuickSlot.1");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_QuickSlot_2, "Inventory.Slot.QuickSlot.2");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_QuickSlot_3, "Inventory.Slot.QuickSlot.3");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_QuickSlot_4, "Inventory.Slot.QuickSlot.4");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_QuickSlot_5, "Inventory.Slot.QuickSlot.5");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_QuickSlot_6, "Inventory.Slot.QuickSlot.6");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_QuickSlot_7, "Inventory.Slot.QuickSlot.7");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_QuickSlot_8, "Inventory.Slot.QuickSlot.8");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_QuickSlot_9, "Inventory.Slot.QuickSlot.9");
	//Wearables
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable, "Inventory.Slot.Wearable");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Head, "Inventory.Slot.Wearable.Head");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Torso, "Inventory.Slot.Wearable.Torso");
    /** Inventory.Slot.Wearable.Gloves - Slot for hand protection */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Gloves, "Inventory.Slot.Wearable.Gloves");

    /** Inventory.Slot.Wearable.Legs - Slot for leg clothing */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Legs, "Inventory.Slot.Wearable.Legs");

    /** Inventory.Slot.Wearable.Shoes - Slot for footwear */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Shoes, "Inventory.Slot.Wearable.Shoes");

    /** Inventory.Slot.Wearable.Armor.Top - Slot for chest/torso armor */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Armor_Top, "Inventory.Slot.Wearable.Armor.Top");

    /** Inventory.Slot.Wearable.Armor.Bottom - Slot for lower body armor */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Armor_Bottom, "Inventory.Slot.Wearable.Armor.Bottom");

    /** Inventory.Slot.Wearable.Armor.Head - Slot for head protection */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Armor_Head, "Inventory.Slot.Wearable.Armor.Head");

    /** Inventory.Slot.Wearable.Armor.Gloves - Slot for specialized armored gauntlets */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Armor_Gloves, "Inventory.Slot.Wearable.Armor.Gloves");

    /** Inventory.Slot.Wearable.Underwear - Slot for base layer clothing */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Underwear, "Inventory.Slot.Wearable.Underwear");

    /** Inventory.Slot.Wearable.Accessory.Chest - Slot for necklaces or chest trinkets */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Accessory_Chest, "Inventory.Slot.Wearable.Accessory.Chest");

    /** Inventory.Slot.Wearable.Accessory.Ear.Left - Slot for left ear accessory */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Accessory_Ear_Left, "Inventory.Slot.Wearable.Accessory.Ear.Left");

    /** Inventory.Slot.Wearable.Accessory.Ear.Right - Slot for right ear accessory */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Accessory_Ear_Right, "Inventory.Slot.Wearable.Accessory.Ear.Right");

    /** Inventory.Slot.Wearable.Accessory.Arm.Left - Slot for left arm bands or bracelets */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Accessory_Arm_Left, "Inventory.Slot.Wearable.Accessory.Arm.Left");

    /** Inventory.Slot.Wearable.Accessory.Arm.Right - Slot for right arm bands or bracelets */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Accessory_Arm_Right, "Inventory.Slot.Wearable.Accessory.Arm.Right");

    /** Inventory.Slot.Wearable.Accessory.Ring.Left - Slot for left hand rings */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Accessory_Ring_Left, "Inventory.Slot.Wearable.Accessory.Ring.Left");

    /** Inventory.Slot.Wearable.Accessory.Ring.Right - Slot for right hand rings */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Slot_Wearable_Accessory_Ring_Right, "Inventory.Slot.Wearable.Accessory.Ring.Right");
    
    
    // INVENTORY TYPE TAGS
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Type_PlayerInventory, "Inventory.Type.PlayerInventory");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Type_Equipment, "Inventory.Type.Equipment");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Type_Container, "Inventory.Type.Container");
    
    // INVENTORY BEHAVIOR TAGS
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Behavior_Equip, "Inventory.Item.Behavior.Equip");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Behavior_Consume, "Inventory.Item.Behavior.Consume");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Behavior_Throw, "Inventory.Item.Behavior.Throw");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Behavior_Activate, "Inventory.Item.Behavior.Activate");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Behavior_Toggle, "Inventory.Item.Behavior.Toggle");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Behavior_Wear, "Inventory.Item.Behavior.Wear");

    //INVENTORY BEHAVIOR CONTEXT MENU TAGS
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Behavior_Unequip, "Inventory.Item.Behavior.Unequip");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Behavior_Use, "Inventory.Item.Behavior.Use");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Behavior_Drop, "Inventory.Item.Behavior.Drop");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Behavior_Split, "Inventory.Item.Behavior.Split");

    
    // INVENTORY FLAG TAGS
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Flags_NoTrade, "Inventory.Item.Flags.NoTrade");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Flags_NoDrop, "Inventory.Item.Flags.NoDrop");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Flags_Stolen, "Inventory.Item.Flags.Stolen");
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Item_Flags_QuestItem, "Inventory.Item.Flags.QuestItem");

    //SIMULATOR FRAMEWORK DEVICE STATE TAGS
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Device_State_Off, "Simulator.Device.State.Off");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Device_State_Idle, "Simulator.Device.State.Idle");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Device_State_InUse, "Simulator.Device.State.InUse");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Device_State_Broken, "Simulator.Device.State.Broken");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Device_State_Maintenance, "Simulator.Device.State.Maintenance");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Device_State_NoPower, "Simulator.Device.State.NoPower");

    //SIMULATOR FRAMEWORK APPLICATION STATE TAGS
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Application_State_Closed, "Simulator.Application.State.Closed");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Application_State_Opening, "Simulator.Application.State.Opening");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Application_State_Open, "Simulator.Application.State.Open");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Application_State_Minimized, "Simulator.Application.State.Minimized");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_Application_State_Closing, "Simulator.Application.State.Closing");

    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Type_Manipulation, "Simulator.MiniGame.Type.Manipulation");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Type_Lockpick, "Simulator.MiniGame.Type.Lockpick");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Type_Sequence, "Simulator.MiniGame.Type.Sequence");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Type_Timing, "Simulator.MiniGame.Type.Timing");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Type_Calibration, "Simulator.MiniGame.Type.Calibration");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Type_Temperature, "Simulator.MiniGame.Type.Temperature");

    // SIMULATOR FRAMEWORK OBJECTIVES
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Objective_ItemSnapped, "Simulator.MiniGame.Objective.ItemSnapped");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Objective_AllPartsAssembled, "Simulator.MiniGame.Objective.AllPartsAssembled");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Objective_LockOpened, "Simulator.MiniGame.Objective.LockOpened");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Objective_CodeEntered, "Simulator.MiniGame.Objective.CodeEntered");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Objective_SequenceComplete, "Simulator.MiniGame.Objective.SequenceComplete");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Objective_TemperatureMaintained, "Simulator.MiniGame.Objective.TemperatureMaintained");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Objective_CalibrationHeld, "Simulator.MiniGame.Objective.CalibrationHeld");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_Objective_TimingHit, "Simulator.MiniGame.Objective.TimingHit");

    // SIMULATOR FRAMEWORK MiniGame IDs
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Sequence_VaultNumpad, "Simulator.MiniGame.ID.Sequence.VaultNumpad");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Sequence_Keypad, "Simulator.MiniGame.ID.Sequence.Keypad");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Lockpick_Standard, "Simulator.MiniGame.ID.Lockpick.Standard");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Lockpick_Advanced, "Simulator.MiniGame.ID.Lockpick.Advanced");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Lockpick_Master, "Simulator.MiniGame.ID.Lockpick.Master");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Assembly_CarEngine, "Simulator.MiniGame.ID.Assembly.CarEngine");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Assembly_Weapon, "Simulator.MiniGame.ID.Assembly.Weapon");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Temperature_CookingSteak, "Simulator.MiniGame.ID.Temperature.CookingSteak");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Temperature_Smithing, "Simulator.MiniGame.ID.Temperature.Smithing");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Timing_SafeDial, "Simulator.MiniGame.ID.Timing.SafeDial");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Timing_Tumbler, "Simulator.MiniGame.ID.Timing.Tumbler");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Calibration_Scope, "Simulator.MiniGame.ID.Calibration.Scope");
    UE_DEFINE_GAMEPLAY_TAG(Simulator_MiniGame_ID_Calibration_Radio, "Simulator.MiniGame.ID.Calibration.Radio");

    // SIMULATOR FRAMEWORK Input: Numpad
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_0, "Input.Numpad.0");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_1, "Input.Numpad.1");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_2, "Input.Numpad.2");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_3, "Input.Numpad.3");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_4, "Input.Numpad.4");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_5, "Input.Numpad.5");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_6, "Input.Numpad.6");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_7, "Input.Numpad.7");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_8, "Input.Numpad.8");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_9, "Input.Numpad.9");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_Clear, "Input.Numpad.Clear");
    UE_DEFINE_GAMEPLAY_TAG(Input_Numpad_Enter, "Input.Numpad.Enter");

    // SIMULATOR FRAMEWORK Input: QTE
    UE_DEFINE_GAMEPLAY_TAG(Input_QTE_North, "Input.QTE.North");
    UE_DEFINE_GAMEPLAY_TAG(Input_QTE_South, "Input.QTE.South");
    UE_DEFINE_GAMEPLAY_TAG(Input_QTE_East, "Input.QTE.East");
    UE_DEFINE_GAMEPLAY_TAG(Input_QTE_West, "Input.QTE.West");

    // UI WIDGET CATEGORIES
    UE_DEFINE_GAMEPLAY_TAG(UI_WIDGET_CATEGORY_CONTEXTMENU, "UI.WIDGET.CATEGORY.CONTEXTMENU");
    UE_DEFINE_GAMEPLAY_TAG(UI_WIDGET_CATEGORY_MODAL, "UI.WIDGET.CATEGORY.MODAL");
    UE_DEFINE_GAMEPLAY_TAG(UI_WIDGET_CATEGORY_WINDOW, "UI.WIDGET.CATEGORY.WINDOW");
    UE_DEFINE_GAMEPLAY_TAG(UI_WIDGET_CATEGORY_PAUSEMENU, "UI.WIDGET.CATEGORY.PAUSEMENU");
    UE_DEFINE_GAMEPLAY_TAG(UI_WIDGET_CATEGORY_HUD, "UI.WIDGET.CATEGORY.HUD");
    
    // CHEAT TAGS
    UE_DEFINE_GAMEPLAY_TAG(Cheat_Permission_None, "Cheat.Permission.None");
    UE_DEFINE_GAMEPLAY_TAG(Cheat_Permission_User, "Cheat.Permission.User");
    UE_DEFINE_GAMEPLAY_TAG(Cheat_Permission_Developer, "Cheat.Permission.Developer");
    UE_DEFINE_GAMEPLAY_TAG(Cheat_Permission_Admin, "Cheat.Permission.Admin");
    UE_DEFINE_GAMEPLAY_TAG(Cheat_Permission_GodMode, "Cheat.Permission.GodMode");
    
    // DEBUG TAGS
    UE_DEFINE_GAMEPLAY_TAG(Debug, "Debug");
    UE_DEFINE_GAMEPLAY_TAG(Debug_AI, "Debug.AI");
    UE_DEFINE_GAMEPLAY_TAG(Debug_Input, "Debug.Input");
    UE_DEFINE_GAMEPLAY_TAG(Debug_Camera, "Debug.Camera");
    UE_DEFINE_GAMEPLAY_TAG(Debug_Character, "Debug.Character");
    UE_DEFINE_GAMEPLAY_TAG(Debug_Combat, "Debug.Combat");
    UE_DEFINE_GAMEPLAY_TAG(Debug_CQC, "Debug.CQC");
    UE_DEFINE_GAMEPLAY_TAG(Debug_Interaction, "Debug.Interaction");
    UE_DEFINE_GAMEPLAY_TAG(Debug_Inventory, "Debug.Inventory");
    UE_DEFINE_GAMEPLAY_TAG(Debug_InventoryInteraction, "Debug.InventoryInteraction");
    UE_DEFINE_GAMEPLAY_TAG(Debug_Movement, "Debug.Movement");
    UE_DEFINE_GAMEPLAY_TAG(Debug_Parkour, "Debug.Parkour");
    UE_DEFINE_GAMEPLAY_TAG(Debug_Pawn, "Debug.Pawn");
    UE_DEFINE_GAMEPLAY_TAG(Debug_SaveSystem, "Debug.SaveSystem");
    UE_DEFINE_GAMEPLAY_TAG(Debug_Widget, "Debug.Widget");

    // UI WIDGET STATE MACHINE STATES
    UE_DEFINE_GAMEPLAY_TAG(UI_Widget_State_Closed, "UI.Widget.State.Closed");
    UE_DEFINE_GAMEPLAY_TAG(UI_Widget_State_AnimatingIn, "UI.Widget.State.AnimatingIn");
    UE_DEFINE_GAMEPLAY_TAG(UI_Widget_State_Visible, "UI.Widget.State.Visible");
    UE_DEFINE_GAMEPLAY_TAG(UI_Widget_State_Paused, "UI.Widget.State.Paused");
    UE_DEFINE_GAMEPLAY_TAG(UI_Widget_State_AnimatingOut, "UI.Widget.State.AnimatingOut");

    // UI DOCK ZONE TAGS
    UE_DEFINE_GAMEPLAY_TAG(UI_Dock_Zone, "UI.Dock.Zone");

    // SPAWN SYSTEM TAGS
    UE_DEFINE_GAMEPLAY_TAG(Spawn_Type_Pickup, "Spawn.Type.Pickup");
    UE_DEFINE_GAMEPLAY_TAG(Spawn_Type_AI, "Spawn.Type.AI");
    UE_DEFINE_GAMEPLAY_TAG(Spawn_Type_Prop, "Spawn.Type.Prop");
    UE_DEFINE_GAMEPLAY_TAG(Spawn_Cleanup_Lifetime, "Spawn.Cleanup.Lifetime");
    UE_DEFINE_GAMEPLAY_TAG(Spawn_Cleanup_Immediate, "Spawn.Cleanup.Immediate");
}

// Implement public accessor functions
// These are exported properly with WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API

// MOVEMENT TAGS
const FGameplayTag& FWWTagLibrary::Movement_Stance_Standing() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Movement_Stance_Standing; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Movement_Stance_Crouch() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Movement_Stance_Crouch; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Movement_Stance_Prone() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Movement_Stance_Prone; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Movement_Speed_Idle() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Movement_Speed_Idle; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Movement_Speed_Walking() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Movement_Speed_Walking; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Movement_Speed_Running() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Movement_Speed_Running; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Movement_Speed_Sprinting() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Movement_Speed_Sprinting; 
    return Tag; 
}

// INPUT TAGS
const FGameplayTag& FWWTagLibrary::Input_Mode_Crouch_Toggle() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_Mode_Crouch_Toggle; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Input_Mode_Crouch_Hold() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_Mode_Crouch_Hold; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Input_Mode_Sprint_Toggle() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_Mode_Sprint_Toggle; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Input_Mode_Sprint_Hold() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_Mode_Sprint_Hold; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Input_Mode_Aim_Toggle() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_Mode_Aim_Toggle; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Input_Mode_Aim_Hold() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_Mode_Aim_Hold; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Input_State_Crouch_Pressed() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_State_Crouch_Pressed; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Input_State_Crouch_Held() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_State_Crouch_Held; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Input_State_Sprint_Pressed() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_State_Sprint_Pressed; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Input_State_Sprint_Held() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_State_Sprint_Held; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Input_State_Jump_Pressed() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Input_State_Jump_Pressed; 
    return Tag; 
}

// CHARACTER STATE TAGS
const FGameplayTag& FWWTagLibrary::Character_State_Alive() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Character_State_Alive; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Character_State_Dead() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Character_State_Dead; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Character_State_InCombat() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Character_State_InCombat; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Character_State_Interacting() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Character_State_Interacting; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Character_State_Stunned() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Character_State_Stunned; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Character_State_Ragdoll() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Character_State_Ragdoll; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Character_State_Invulnerable() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Character_State_Invulnerable; 
    return Tag; 
}

// CAMERA TAGS
const FGameplayTag& FWWTagLibrary::Camera_Mode_FirstPerson() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_FirstPerson; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_TrueFPS()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_TrueFPS;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Vehicle()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Vehicle;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Vehicle_Chase()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Vehicle_Chase;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Vehicle_Cockpit()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Vehicle_Cockpit;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Vehicle_Hood()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Vehicle_Hood;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Vehicle_Bumper()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Vehicle_Bumper;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Vehicle_Orbit()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Vehicle_Orbit;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Vehicle_Cinematic()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Vehicle_Cinematic;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Custom()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Custom;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Station_Default() { static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Default; return Tag; }
const FGameplayTag& FWWTagLibrary::Camera_Mode_Station_Generic() { static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Station_Generic; return Tag; }
const FGameplayTag& FWWTagLibrary::Camera_Mode_Station_Numpad() { static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Station_Numpad; return Tag; }
const FGameplayTag& FWWTagLibrary::Camera_Mode_Station_Lockpick() { static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Station_Lockpick; return Tag; }
const FGameplayTag& FWWTagLibrary::Camera_Mode_Station_Assembly() { static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Station_Assembly; return Tag; }
const FGameplayTag& FWWTagLibrary::Camera_Mode_Station_Cooking() { static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Station_Cooking; return Tag; }

const FGameplayTag& FWWTagLibrary::Camera_Mode_Custom_Default()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Custom_Default;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Custom_Killcam()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Custom_KillCam;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Custom_Stealth()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Custom_Stealth;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_Custom_Cinematic()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_Custom_Cinematic;
    return Tag;
}


const FGameplayTag& FWWTagLibrary::Camera_Mode_ThirdPerson() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_ThirdPerson; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Camera_Mode_TopDown() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Camera_Mode_TopDown; 
    return Tag; 
}


const FGameplayTag& FWWTagLibrary::Camera_Shoulder_Center()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Shoulder_Center; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Camera_Shoulder_Left()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Shoulder_Left; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Camera_Shoulder_Right()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Shoulder_Right; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Camera_Aim_None()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Aim_None; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Camera_Aim_Aiming()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Aim_Aiming;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Camera_Aim_Hip()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Aim_Hip; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Camera_Aim_Scope()
{
    static const FGameplayTag& Tag = WW_Internal::Camera_Aim_Scope; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Camera_State_Zooming() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Camera_State_Zooming; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Camera_State_Transitioning() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Camera_State_Transitioning; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Camera_State_LockedOn() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Camera_State_LockedOn; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::CameraShake_Combat_Explosion()
{
    static const FGameplayTag& Tag = WW_Internal::CameraShake_Combat_Explosion; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::CameraShake_Combat_Punch()
{
    static const FGameplayTag& Tag = WW_Internal::CameraShake_Combat_Punch; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::CameraShake_Combat_Grenade()
{
    static const FGameplayTag& Tag = WW_Internal::CameraShake_Combat_Grenade; 
    return Tag; 
}

// INTERACTION TAGS
const FGameplayTag& FWWTagLibrary::Interaction_Action_Pickup() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_Action_Pickup; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_Action_Drop() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_Action_Drop; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_Action_Use() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_Action_Use; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_Action_Equip() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_Action_Equip; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_Action_Unequip() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_Action_Unequip; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_Action_Talk() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_Action_Talk; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_Action_Open() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_Action_Open; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_Action_Close() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_Action_Close; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_Action_Examine() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_Action_Examine; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_State_Available() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_State_Available; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_State_InProgress() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_State_InProgress; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_State_Holding() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_State_Holding; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_State_Mashing() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_State_Mashing; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Interaction_State_Cooldown() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Interaction_State_Cooldown; 
    return Tag; 
}


// ITEM TAGS (Pickupable world items)

const FGameplayTag& FWWTagLibrary::Item_Type_Weapon_Ranged()
{
    static const FGameplayTag& Tag = WW_Internal::Item_Type_Weapon_Ranged;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Item_Type_Weapon_Melee()
{
    static const FGameplayTag& Tag = WW_Internal::Item_Type_Weapon_Melee;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Item_Type_Throwable()
{
    static const FGameplayTag& Tag = WW_Internal::Item_Type_Throwable;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Item_Type_Armor()
{
    static const FGameplayTag& Tag = WW_Internal::Item_Type_Armor;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Item_Type_Consumable()
{
    static const FGameplayTag& Tag = WW_Internal::Item_Type_Consumable;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Item_Rarity_Common()
{
    static const FGameplayTag& Tag = WW_Internal::Item_Rarity_Common;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Item_Rarity_Uncommon()
{
    static const FGameplayTag& Tag = WW_Internal::Item_Rarity_Uncommon;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Item_Rarity_Rare()
{
    static const FGameplayTag& Tag = WW_Internal::Item_Rarity_Rare;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Item_Rarity_Epic()
{
    static const FGameplayTag& Tag = WW_Internal::Item_Rarity_Epic;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Item_Rarity_Legendary()
{
    static const FGameplayTag& Tag = WW_Internal::Item_Rarity_Legendary;
    return Tag;
}

// INVENTORY TAGS

const FGameplayTag& FWWTagLibrary::Inventory_Item_Type_Attachment()
{
    return WW_Internal::Inventory_Item_Type_Attachment;

}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Type_Wearable()
{
    return WW_Internal::Inventory_Item_Type_Wearable;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Type_Throwable()
{
    return WW_Internal::Inventory_Item_Type_Throwable;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Type_Weapon() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Type_Weapon; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Type_Armor() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Type_Armor; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Type_Consumable() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Type_Consumable; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Type_KeyItem() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Type_KeyItem; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Type_Crafting() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Type_Crafting; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Type_Quest() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Type_Quest; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Rarity_Common() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Rarity_Common; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Rarity_Uncommon() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Rarity_Uncommon; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Rarity_Rare() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Rarity_Rare; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Rarity_Epic() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Rarity_Epic; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Rarity_Legendary() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Rarity_Legendary; 
    return Tag; 
}


const FGameplayTag& FWWTagLibrary::Inventory_Slot_MainHand() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_MainHand; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_OffHand() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_OffHand; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_QuickSlot_0()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_QuickSlot_0;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_QuickSlot_1()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_QuickSlot_1;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_QuickSlot_2()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_QuickSlot_2;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_QuickSlot_3()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_QuickSlot_3;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_QuickSlot_4()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_QuickSlot_4;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_QuickSlot_5()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_QuickSlot_5;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_QuickSlot_6()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_QuickSlot_6;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_QuickSlot_7()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_QuickSlot_7;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_QuickSlot_8()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_QuickSlot_8;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_QuickSlot_9()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_QuickSlot_9;
    return Tag;
}

inline const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Head()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Head; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Torso()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Torso; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Gloves()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Gloves; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Legs()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Legs; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Shoes()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Shoes; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Armor_Top()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Armor_Top; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Armor_Bottom()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Armor_Bottom; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Armor_Head()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Armor_Head; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Armor_Gloves()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Armor_Gloves; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Undearwear()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Underwear; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Chest()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Accessory_Chest; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Ear_Left()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Accessory_Ear_Left; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Ear_Right()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Accessory_Ear_Right; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Arm_Left()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Accessory_Arm_Left; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Arm_Right()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Accessory_Arm_Right; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Ring_Left()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Accessory_Ring_Left; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Ring_Right()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Slot_Wearable_Accessory_Ring_Right; 
    return Tag; 
}

// INVENTORY TYPE TAGS
const FGameplayTag& FWWTagLibrary::Inventory_Type_PlayerInventory()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Type_PlayerInventory;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Type_Equipment()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Type_Equipment;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Type_Container()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Type_Container;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Device_State_Off()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Device_State_Off;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Device_State_Idle()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Device_State_Idle;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Device_State_InUse()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Device_State_InUse;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Device_State_Broken()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Device_State_Broken;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Device_State_Maintenance()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Device_State_Maintenance;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Device_State_NoPower()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Device_State_NoPower;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Application_State_Closed()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Application_State_Closed;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Application_State_Opening()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Application_State_Opening;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Application_State_Open()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Application_State_Open;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Application_State_Minimized()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Application_State_Minimized;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Simulator_Application_State_Closing()
{
    static const FGameplayTag& Tag = WW_Internal::Simulator_Application_State_Closing;
    return Tag;
}

// Simulator framework minigame types
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Type_Manipulation() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Type_Manipulation; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Type_Lockpick() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Type_Lockpick; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Type_Sequence() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Type_Sequence; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Type_Timing() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Type_Timing; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Type_Calibration() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Type_Calibration; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Type_Temperature() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Type_Temperature; return Tag; }

// Objectives
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Objective_ItemSnapped() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Objective_ItemSnapped; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Objective_AllPartsAssembled() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Objective_AllPartsAssembled; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Objective_LockOpened() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Objective_LockOpened; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Objective_CodeEntered() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Objective_CodeEntered; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Objective_SequenceComplete() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Objective_SequenceComplete; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Objective_TemperatureMaintained() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Objective_TemperatureMaintained; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Objective_CalibrationHeld() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Objective_CalibrationHeld; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_Objective_TimingHit() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_Objective_TimingHit; return Tag; }

// MiniGame IDs
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Sequence_VaultNumpad() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Sequence_VaultNumpad; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Sequence_Keypad() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Sequence_Keypad; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Lockpick_Standard() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Lockpick_Standard; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Lockpick_Advanced() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Lockpick_Advanced; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Lockpick_Master() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Lockpick_Master; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Assembly_CarEngine() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Assembly_CarEngine; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Assembly_Weapon() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Assembly_Weapon; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Temperature_CookingSteak() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Temperature_CookingSteak; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Temperature_Smithing() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Temperature_Smithing; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Timing_SafeDial() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Timing_SafeDial; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Timing_Tumbler() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Timing_Tumbler; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Calibration_Scope() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Calibration_Scope; return Tag; }
const FGameplayTag& FWWTagLibrary::Simulator_MiniGame_ID_Calibration_Radio() { static const FGameplayTag& Tag = WW_Internal::Simulator_MiniGame_ID_Calibration_Radio; return Tag; }

// Input: Numpad
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_0, "Input.Numpad.0");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_1, "Input.Numpad.1");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_2, "Input.Numpad.2");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_3, "Input.Numpad.3");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_4, "Input.Numpad.4");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_5, "Input.Numpad.5");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_6, "Input.Numpad.6");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_7, "Input.Numpad.7");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_8, "Input.Numpad.8");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_9, "Input.Numpad.9");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_Clear, "Input.Numpad.Clear");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_Numpad_Enter, "Input.Numpad.Enter");

// Input: QTE
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_QTE_North, "Input.QTE.North");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_QTE_South, "Input.QTE.South");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_QTE_East, "Input.QTE.East");
UE_DEFINE_GAMEPLAY_TAG(Simulator_Input_QTE_West, "Input.QTE.West");

// INVENTORY BEHAVIOR TAGS
const FGameplayTag& FWWTagLibrary::Inventory_Item_Behavior_Equip()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Behavior_Equip;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Behavior_Unequip()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Behavior_Unequip;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Behavior_Consume()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Behavior_Consume;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Behavior_Wear()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Behavior_Wear;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Behavior_Throw()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Behavior_Throw;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Behavior_Activate()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Behavior_Activate;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Behavior_Toggle()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Behavior_Toggle;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Behavior_Use()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Behavior_Use;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Behavior_Split()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Behavior_Split;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Behavior_Drop()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Behavior_Drop;
    return Tag;
}

// INVENTORY FLAG TAGS
const FGameplayTag& FWWTagLibrary::Inventory_Item_Flags_NoTrade()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Flags_NoTrade;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Flags_NoDrop()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Flags_NoDrop;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Flags_Stolen()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Flags_Stolen;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::Inventory_Item_Flags_QuestItem()
{
    static const FGameplayTag& Tag = WW_Internal::Inventory_Item_Flags_QuestItem;
    return Tag;
}


// CHEAT TAGS
const FGameplayTag& FWWTagLibrary::Cheat_Permission_None() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Cheat_Permission_None; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Cheat_Permission_User() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Cheat_Permission_User; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Cheat_Permission_Developer() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Cheat_Permission_Developer; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Cheat_Permission_Admin() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Cheat_Permission_Admin; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Cheat_Permission_GodMode() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Cheat_Permission_GodMode; 
    return Tag; 
}

// DEBUG TAGS
const FGameplayTag& FWWTagLibrary::Debug() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_Input()
{
    static const FGameplayTag& Tag = WW_Internal::Debug_Input; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_AI() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_AI; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_Camera() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_Camera; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_Character() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_Character; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_Combat() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_Combat; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_CQC() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_CQC; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_Interaction() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_Interaction; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_Inventory() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_Inventory; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_InventoryInteraction() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_InventoryInteraction; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_Movement() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_Movement; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_Parkour() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_Parkour; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_Pawn() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_Pawn; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_SaveSystem() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_SaveSystem; 
    return Tag; 
}

const FGameplayTag& FWWTagLibrary::Debug_Widget() 
{ 
    static const FGameplayTag& Tag = WW_Internal::Debug_Widget; 
    return Tag; 
}



const FGameplayTag& FWWTagLibrary::UI_Mode_Game()
{
    static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("UI.Mode.Game"));
    return Tag;
}

const FGameplayTag& FWWTagLibrary::UI_Mode_Inventory()
{
    static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("UI.Mode.Inventory"));
    return Tag;
}

const FGameplayTag& FWWTagLibrary::UI_Mode_Crafting()
{
    static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("UI.Mode.Crafting"));
    return Tag;
}

const FGameplayTag& FWWTagLibrary::UI_Mode_Shop()
{
    static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("UI.Mode.Shop"));
    return Tag;
}

const FGameplayTag& FWWTagLibrary::UI_Mode_Menu()
{
    static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("UI.Mode.Menu"));
    return Tag;
}

// UI - Inventory item Sorting
FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Custom()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Custom");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Name_Ascending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Name.Ascending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Name_Descending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Name.Descending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Rarity_Ascending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Rarity.Ascending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Rarity_Descending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Rarity.Descending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Type_Ascending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Type.Ascending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Type_Descending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Type.Descending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Weight_Ascending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Weight.Ascending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Weight_Descending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Weight.Descending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Value_Ascending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Value.Ascending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Value_Descending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Value.Descending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Quantity_Ascending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Quantity.Ascending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Quantity_Descending()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Quantity.Descending");

}

FGameplayTag FWWTagLibrary::UI_Inventory_Sort_Search()
{
    return FGameplayTag::RequestGameplayTag("UI.Inventory.Sort.Search");

}

// UI- WIDGET CATEGORIES

FGameplayTag FWWTagLibrary::UI_Widget_Category_ContextMenu()
{
    return FGameplayTag::RequestGameplayTag(FName("UI.Widget.Category.ContextMenu")); 

}

FGameplayTag FWWTagLibrary::UI_Widget_Category_Modal()
{
    return FGameplayTag::RequestGameplayTag(FName("UI.Widget.Category.Modal")); 

}

FGameplayTag FWWTagLibrary::UI_Widget_Category_Window()
{
    return FGameplayTag::RequestGameplayTag(FName("UI.Widget.Category.Window")); 

}

FGameplayTag FWWTagLibrary::UI_Widget_Category_PauseMenu()
{
    return FGameplayTag::RequestGameplayTag(FName("UI.Widget.Category.PauseMenu")); 

}

FGameplayTag FWWTagLibrary::UI_Widget_Category_HUD()
{
    return FGameplayTag::RequestGameplayTag(FName("UI.Widget.Category.HUD"));

}

// UI WIDGET STATE MACHINE STATES

const FGameplayTag& FWWTagLibrary::UI_Widget_State_Closed()
{
    static const FGameplayTag& Tag = WW_Internal::UI_Widget_State_Closed;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::UI_Widget_State_AnimatingIn()
{
    static const FGameplayTag& Tag = WW_Internal::UI_Widget_State_AnimatingIn;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::UI_Widget_State_Visible()
{
    static const FGameplayTag& Tag = WW_Internal::UI_Widget_State_Visible;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::UI_Widget_State_Paused()
{
    static const FGameplayTag& Tag = WW_Internal::UI_Widget_State_Paused;
    return Tag;
}

const FGameplayTag& FWWTagLibrary::UI_Widget_State_AnimatingOut()
{
    static const FGameplayTag& Tag = WW_Internal::UI_Widget_State_AnimatingOut;
    return Tag;
}

// UI DOCK ZONE TAGS
const FGameplayTag& FWWTagLibrary::UI_Dock_Zone()
{
    static const FGameplayTag& Tag = WW_Internal::UI_Dock_Zone;
    return Tag;
}

// SPAWN SYSTEM TAGS
const FGameplayTag& FWWTagLibrary::Spawn_Type_Pickup() { static const FGameplayTag& Tag = WW_Internal::Spawn_Type_Pickup; return Tag; }
const FGameplayTag& FWWTagLibrary::Spawn_Type_AI() { static const FGameplayTag& Tag = WW_Internal::Spawn_Type_AI; return Tag; }
const FGameplayTag& FWWTagLibrary::Spawn_Type_Prop() { static const FGameplayTag& Tag = WW_Internal::Spawn_Type_Prop; return Tag; }
const FGameplayTag& FWWTagLibrary::Spawn_Cleanup_Lifetime() { static const FGameplayTag& Tag = WW_Internal::Spawn_Cleanup_Lifetime; return Tag; }
const FGameplayTag& FWWTagLibrary::Spawn_Cleanup_Immediate() { static const FGameplayTag& Tag = WW_Internal::Spawn_Cleanup_Immediate; return Tag; }
