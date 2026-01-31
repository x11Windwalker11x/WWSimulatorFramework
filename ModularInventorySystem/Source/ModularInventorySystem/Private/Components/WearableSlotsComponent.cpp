// WearableSlotsComponent.cpp
// Location: ModularInventorySystem/Private/Components/WearableSlotsComponent.cpp

#include "Components/WearableSlotsComponent.h"
#include "Components/InventoryComponent.h"
#include "Interfaces/ModularInventorySystem/InventoryInterface.h"
#include "Net/UnrealNetwork.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
DEFINE_LOG_CATEGORY_STATIC(LogWearableSlots, Log, All);

UWearableSlotsComponent::UWearableSlotsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UWearableSlotsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CacheTags();
    InitializeSlots();
    GetInventoryComponent();
}

void UWearableSlotsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UWearableSlotsComponent, WearableSlots, COND_OwnerOnly);
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UWearableSlotsComponent::CacheTags()
{
    Tag_Item_Behavior_Wear = FWWTagLibrary::Inventory_Item_Behavior_Wear();
    // Cache all 18 slot types
    AllSlotTypes = {
        FWWTagLibrary::Inventory_Slot_Wearable_Head(),
        FWWTagLibrary::Inventory_Slot_Wearable_Torso(),
        FWWTagLibrary::Inventory_Slot_Wearable_Gloves(),
        FWWTagLibrary::Inventory_Slot_Wearable_Legs(),
        FWWTagLibrary::Inventory_Slot_Wearable_Shoes(),
        FWWTagLibrary::Inventory_Slot_Wearable_Armor_Top(),
        FWWTagLibrary::Inventory_Slot_Wearable_Armor_Bottom(),
        FWWTagLibrary::Inventory_Slot_Wearable_Armor_Head(),
        FWWTagLibrary::Inventory_Slot_Wearable_Armor_Gloves(),
        FWWTagLibrary::Inventory_Slot_Wearable_Undearwear(),
        FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Chest(),
        FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Ear_Left(),
        FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Ear_Right(),
        FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Arm_Left(),
        FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Arm_Right(),
        FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Ring_Left(),
        FWWTagLibrary::Inventory_Slot_Wearable_Accessory_Ring_Right()
    };
}

void UWearableSlotsComponent::InitializeSlots()
{
    for (const FGameplayTag& SlotType : AllSlotTypes)
    {
        WearableSlots.Add(SlotType, FInventorySlot());
    }
}

// ============================================================================
// HELPERS
// ============================================================================

UInventoryComponent* UWearableSlotsComponent::GetInventoryComponent()
{
    UActorComponent* AC_InventoryComponent  = nullptr;
    AActor* Owner = this->GetOwner();
    if (Owner && Owner->GetClass()->ImplementsInterface(UInventoryInterface::StaticClass()))
    {
        AC_InventoryComponent = IInventoryInterface::Execute_GetInventoryComponent(Owner);
    }
    
    if (!AC_InventoryComponent && !InventoryComponent.IsValid())
    {
        UE_LOG(LogWearableSlots, Warning, TEXT("WearableSlotsComponent requires InventoryComponent on same actor"));
    }
    InventoryComponent = Cast<UInventoryComponent>(AC_InventoryComponent);
    
    return InventoryComponent.Get();
}

FGameplayTag UWearableSlotsComponent::ConvertSlotTagToItemTag(FGameplayTag SlotTag) const
{
    // Inventory.Slot.Wearable.Head → Item.Wearable.Head
    FString SlotString = SlotTag.ToString();
    FString ItemString = SlotString.Replace(TEXT("Inventory.Slot.Wearable"), TEXT("Item.Wearable"));
    return FGameplayTag::RequestGameplayTag(FName(*ItemString), false);
}

// ============================================================================
// QUERIES
// ============================================================================

const FInventorySlot& UWearableSlotsComponent::GetWearableSlot(FGameplayTag SlotType) const
{
    static FInventorySlot EmptySlot;
    const FInventorySlot* Found = WearableSlots.Find(SlotType);
    return Found ? *Found : EmptySlot;
}

bool UWearableSlotsComponent::IsSlotEmpty(FGameplayTag SlotType) const
{
    const FInventorySlot* Found = WearableSlots.Find(SlotType);
    return !Found || Found->IsEmpty();
}

bool UWearableSlotsComponent::CanEquipToSlot(const FInventorySlot& Slot, FGameplayTag SlotType) const
{
    if (Slot.IsEmpty()) return false;
    
    // Step 1: Must have wearable behavior
    if (!Slot.ItemTags.HasTag(Tag_Item_Behavior_Wear))
    {
        return false;
    }
    
    // Step 2: Must match slot type
    FGameplayTag RequiredItemTag = ConvertSlotTagToItemTag(SlotType);
    return RequiredItemTag.IsValid() && Slot.ItemTags.HasTag(RequiredItemTag);
}


FGameplayTag UWearableSlotsComponent::FindSlotByInstanceID(FGuid InstanceID) const
{
    for (const auto& Pair : WearableSlots)
    {
        if (Pair.Value.InstanceID == InstanceID)
        {
            return Pair.Key;
        }
    }
    return FGameplayTag();
}

TArray<FGameplayTag> UWearableSlotsComponent::GetAllSlotTypes() const
{
    return AllSlotTypes;
}

FInventorySlot UWearableSlotsComponent::GetEquippedComparisonItem(FGameplayTag SlotType) const
{
    return GetWearableSlot(SlotType);
}

// ============================================================================
// OPERATIONS
// ============================================================================

bool UWearableSlotsComponent::EquipToSlot(int32 InventoryIndex, FGameplayTag SlotType)
{
    if (!WearableSlots.Contains(SlotType)) return false;
    
    if (!GetOwner()->HasAuthority())
    {
        Server_EquipToSlot(InventoryIndex, SlotType);
        return true;
    }
    
    return Internal_EquipToSlot(InventoryIndex, SlotType);
}

bool UWearableSlotsComponent::Internal_EquipToSlot(int32 InventoryIndex, FGameplayTag SlotType)
{
    UInventoryComponent* InvComp = GetInventoryComponent();
    if (!InvComp) return false;
    
    const TArray<FInventorySlot>& Inventory = InvComp->GetInventory();
    if (!Inventory.IsValidIndex(InventoryIndex)) return false;
    
    const FInventorySlot& SourceSlot = Inventory[InventoryIndex];
    
    if (!CanEquipToSlot(SourceSlot, SlotType))
    {
        UE_LOG(LogWearableSlots, Warning, TEXT("Item cannot be equipped to slot %s"), *SlotType.ToString());
        return false;
    }
    
    // Get current equipped item (for swap)
    FInventorySlot OldEquipped = WearableSlots[SlotType];
    
    // Equip new item
    WearableSlots[SlotType] = SourceSlot;
    WearableSlots[SlotType].bIsEquipped = true;
    WearableSlots[SlotType].SlotType = SlotType;
    
    // Update inventory - remove equipped item, add old equipped if any
    // Note: This requires InventoryComponent to have SetSlot or similar
    FInventorySlot UpdatedSourceSlot = OldEquipped;
    if (OldEquipped.IsEmpty())
    {
        UpdatedSourceSlot.Clear();
    }
    else
    {
        UpdatedSourceSlot.bIsEquipped = false;
        UpdatedSourceSlot.SlotType = FGameplayTag();
    }
    
    InvComp->SetSlot(InventoryIndex, UpdatedSourceSlot);
    
    OnWearableSlotChanged.Broadcast(SlotType, WearableSlots[SlotType]);
    OnWearableEquipped.Broadcast(SlotType);
    
    return true;
}

bool UWearableSlotsComponent::UnequipFromSlot(FGameplayTag SlotType)
{
    if (!WearableSlots.Contains(SlotType)) return false;
    
    if (!GetOwner()->HasAuthority())
    {
        Server_UnequipFromSlot(SlotType);
        return true;
    }
    
    return Internal_UnequipFromSlot(SlotType);
}

bool UWearableSlotsComponent::Internal_UnequipFromSlot(FGameplayTag SlotType)
{
    if (IsSlotEmpty(SlotType)) return false;
    
    UInventoryComponent* InvComp = GetInventoryComponent();
    if (!InvComp) return false;
    
    // Find empty slot in inventory
    int32 EmptyIndex = InvComp->FindEmptySlot(FGameplayTag());
    if (EmptyIndex == INDEX_NONE)
    {
        UE_LOG(LogWearableSlots, Warning, TEXT("No empty inventory slot to unequip"));
        return false;
    }
    
    // Move to inventory
    FInventorySlot Unequipped = WearableSlots[SlotType];
    Unequipped.bIsEquipped = false;
    Unequipped.SlotType = FGameplayTag();
    
    InvComp->SetSlot(EmptyIndex, Unequipped);
    
    // Clear wearable slot
    WearableSlots[SlotType].Clear();
    
    OnWearableSlotChanged.Broadcast(SlotType, WearableSlots[SlotType]);
    OnWearableUnequipped.Broadcast(SlotType);
    
    return true;
}

bool UWearableSlotsComponent::SwapWithInventory(FGameplayTag SlotType, int32 InventoryIndex)
{
    if (!WearableSlots.Contains(SlotType)) return false;
    
    if (!GetOwner()->HasAuthority())
    {
        Server_SwapWithInventory(SlotType, InventoryIndex);
        return true;
    }
    
    return Internal_SwapWithInventory(SlotType, InventoryIndex);
}

bool UWearableSlotsComponent::Internal_SwapWithInventory(FGameplayTag SlotType, int32 InventoryIndex)
{
    UInventoryComponent* InvComp = GetInventoryComponent();
    if (!InvComp) return false;
    
    const TArray<FInventorySlot>& Inventory = InvComp->GetInventory();
    if (!Inventory.IsValidIndex(InventoryIndex)) return false;
    
    const FInventorySlot& InvSlot = Inventory[InventoryIndex];
    
    // If inventory slot has item, must be valid for this wearable slot
    if (!InvSlot.IsEmpty() && !CanEquipToSlot(InvSlot, SlotType))
    {
        return false;
    }
    
    // Swap
    FInventorySlot OldWearable = WearableSlots[SlotType];
    
    // Equip from inventory
    if (!InvSlot.IsEmpty())
    {
        WearableSlots[SlotType] = InvSlot;
        WearableSlots[SlotType].bIsEquipped = true;
        WearableSlots[SlotType].SlotType = SlotType;
    }
    else
    {
        WearableSlots[SlotType].Clear();
    }
    
    // Put old wearable in inventory
    if (!OldWearable.IsEmpty())
    {
        OldWearable.bIsEquipped = false;
        OldWearable.SlotType = FGameplayTag();
        InvComp->SetSlot(InventoryIndex, OldWearable);
    }
    else
    {
        FInventorySlot Empty;
        InvComp->SetSlot(InventoryIndex, Empty);
    }
    
    OnWearableSlotChanged.Broadcast(SlotType, WearableSlots[SlotType]);
    
    return true;
}

// ============================================================================
// REPLICATION
// ============================================================================

void UWearableSlotsComponent::OnRep_WearableSlots()
{
    for (const auto& Pair : WearableSlots)
    {
        OnWearableSlotChanged.Broadcast(Pair.Key, Pair.Value);
    }
}

// ============================================================================
// SERVER RPCs
// ============================================================================

bool UWearableSlotsComponent::Server_EquipToSlot_Validate(int32 InventoryIndex, FGameplayTag SlotType)
{
    return WearableSlots.Contains(SlotType);
}

void UWearableSlotsComponent::Server_EquipToSlot_Implementation(int32 InventoryIndex, FGameplayTag SlotType)
{
    Internal_EquipToSlot(InventoryIndex, SlotType);
}

bool UWearableSlotsComponent::Server_UnequipFromSlot_Validate(FGameplayTag SlotType)
{
    return WearableSlots.Contains(SlotType);
}

void UWearableSlotsComponent::Server_UnequipFromSlot_Implementation(FGameplayTag SlotType)
{
    Internal_UnequipFromSlot(SlotType);
}

bool UWearableSlotsComponent::Server_SwapWithInventory_Validate(FGameplayTag SlotType, int32 InventoryIndex)
{
    return WearableSlots.Contains(SlotType);
}

void UWearableSlotsComponent::Server_SwapWithInventory_Implementation(FGameplayTag SlotType, int32 InventoryIndex)
{
    Internal_SwapWithInventory(SlotType, InventoryIndex);
}