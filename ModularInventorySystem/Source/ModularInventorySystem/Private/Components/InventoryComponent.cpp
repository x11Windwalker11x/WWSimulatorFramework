// InventoryComponent.cpp
// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Actors/Interactables/Item/Pickup/PickupActor_Master.h"
#include "Delegates/ModularInventorySystem/InventoryDelegates.h"
#include "Subsystems/UniversalSpawnManager.h"
#include "GameplayTags.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Logging/InteractableInventoryLogging.h"
#include "Utilities/JsonReader/ItemJsonReader.h"
#include "Subsystems/SaveSystem/SaveableRegistrySubsystem.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

// ============================================================================
// CONSTRUCTOR & LIFECYCLE
// ============================================================================

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetOwner()->HasAuthority())
    {
        Inventory.SetNum(MaxInventorySlots);
        
        for (FInventorySlot& Slot : Inventory)
        {
            Slot.Clear();
        }
        
        UE_LOG(LogInventoryInteractableSystem, Log, TEXT("InventoryComponent initialized: %d slots"), MaxInventorySlots);
    }

    if (USaveableRegistrySubsystem* Registry = USaveableRegistrySubsystem::Get(this))
    {
        Registry->RegisterSaveable(this);
    }
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (USaveableRegistrySubsystem* Registry = USaveableRegistrySubsystem::Get(this))
    {
        Registry->UnregisterSaveable(GetSaveID_Implementation());
    }
    Super::EndPlay(EndPlayReason);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UInventoryComponent, Inventory, COND_OwnerOnly);
}

// ============================================================================
// INTERFACE IMPLEMENTATIONS
// ============================================================================

UActorComponent* UInventoryComponent::GetInventoryComponent_Implementation()
{
    return this;
}

int32 UInventoryComponent::GetItemCount_Implementation(FName ItemID) const
{
    return GetItemCountPure(ItemID);
}

bool UInventoryComponent::TryConsumeItem_Implementation(FName ItemID, int32 Quantity)
{
    int32 Qty = Quantity;
    return TryConsumeItem(ItemID, Qty, INDEX_NONE);
}

bool UInventoryComponent::AddItem_Implementation(FName ItemID, int32 Quantity, float Quality, float Durability, bool bIsStolen)
{
    return AddItem(ItemID, Quantity, Quality, Durability, bIsStolen);
}

bool UInventoryComponent::RemoveItemByInstance_Implementation(FGuid InstanceID, int32 Quantity)
{
    return RemoveItemByInstance(InstanceID, Quantity);
}

bool UInventoryComponent::DropItem_Implementation(FGameplayTag InventoryType, int32 SlotIndex, int32 Quantity)
{
    return DropItem(InventoryType, SlotIndex, Quantity);
}

bool UInventoryComponent::UseItem_Implementation(FGameplayTag InventoryType, int32 SlotIndex)
{
    return UseItem(InventoryType, SlotIndex);
}

bool UInventoryComponent::EquipItem_Implementation(FGameplayTag InventoryType, int32 SlotIndex)
{
    return EquipItem(InventoryType, SlotIndex);
}

bool UInventoryComponent::UnequipItem_Implementation(FGameplayTag InventoryType, int32 SlotIndex)
{
    return UnequipItem(InventoryType, SlotIndex);
}

int32 UInventoryComponent::FindEmptySlot_Implementation(FGameplayTag InventoryType) const
{
    return FindEmptySlot(InventoryType);
}







void UInventoryComponent::AddComparisonEntry(TArray<FStatComparisonEntry>& Entries, const FName& StatName,
    const FString& DisplayName, float CurrentValue, float CompareValue, bool bHigherIsBetter,
    const FString& UnitSuffix) const
{
    if (FMath::IsNearlyZero(CurrentValue) && FMath::IsNearlyZero(CompareValue)) return;
    
    FStatComparisonEntry Entry;
    Entry.StatName = StatName;
    Entry.StatDisplayName = FText::FromString(DisplayName);
    Entry.CurrentValue = CurrentValue;
    Entry.CompareValue = CompareValue;
    Entry.bHigherIsBetter = bHigherIsBetter;
    Entry.UnitSuffix = UnitSuffix;
    Entries.Add(Entry);

}

void UInventoryComponent::OnRep_Inventory()
{
    UE_LOG(LogInventoryInteractableSystem, Verbose, TEXT("Client: Inventory replicated"));
    OnInventoryChanged.Broadcast(FWWTagLibrary::Inventory_Type_PlayerInventory());
    MarkSaveDirty();
}

// ============================================================================
// DATA ACCESS
// ============================================================================

void UInventoryComponent::Server_TryConsumeItem_Implementation(const FName& ItemID, int32 Quantity)
{
    if (GetItemCount(ItemID) < Quantity) return;
    
    Internal_ConsumeItem(ItemID, Quantity);

}

void UInventoryComponent::Server_SetSlot_Implementation(int32 Index, FInventorySlot& Slot)
{
    Internal_SetSlot(Index, Slot);
}

bool UInventoryComponent::SetSlot(int32 Index, FInventorySlot& Slot)
{
    if (!Inventory.IsValidIndex(Index)) return false;
    if (!GetOwner()->HasAuthority())
    {
        Server_SetSlot(Index, Slot);
        return true;
    }
    return Internal_SetSlot(Index, Slot);
}

const TArray<FInventorySlot>& UInventoryComponent::GetInventory(FGameplayTag InInventoryTypeTag) const
{
    if (InInventoryTypeTag.MatchesTagExact(InInventoryTypeTag))
    {
        return Inventory;
    }
    return TArray<FInventorySlot>();
}

bool UInventoryComponent::GetSlot(FGameplayTag InInventoryTypeTag, int32 SlotIndex, FInventorySlot& OutSlot) const
{
    const TArray<FInventorySlot>* InventoryArray = GetInventoryArray(InInventoryTypeTag);
    if (!InventoryArray || !InventoryArray->IsValidIndex(SlotIndex))
    {
        return false;
    }
    OutSlot = (*InventoryArray)[SlotIndex];
    return true;
}

bool UInventoryComponent::GetItemData(FName ItemID, FItemData& OutItemData) const
{
    if (!ItemDataTable || ItemID.IsNone())
    {
        return false;
    }
    
    static const FString Context = TEXT("InventoryItemLookup");
    FItemData* FoundData = ItemDataTable->FindRow<FItemData>(ItemID, Context);
    
    if (FoundData)
    {
        OutItemData = *FoundData;
        return true;
    }
    return false;
}

TArray<FInventorySlot>* UInventoryComponent::GetInventoryArray(FGameplayTag InInventoryTypeTag)
{
    if (InInventoryTypeTag == FWWTagLibrary::Inventory_Type_PlayerInventory())
    {
        return &Inventory;
    }
    return nullptr;
}

const TArray<FInventorySlot>* UInventoryComponent::GetInventoryArray(FGameplayTag InInventoryTypeTag) const
{
    if (InInventoryTypeTag == FWWTagLibrary::Inventory_Type_PlayerInventory())
    {
        return &Inventory;
    }
    return nullptr;
}

int32 UInventoryComponent::GetItemCountPure(FName ItemID) const
{
    if (ItemID.IsNone()) return 0;
    
    int32 TotalCount = 0;
    for (const FInventorySlot& Slot : Inventory)
    {
        if (Slot.ItemID == ItemID)
        {
            TotalCount += Slot.Quantity;
        }
    }
    return TotalCount;
}



bool UInventoryComponent::HasItem_Implementation(FName ItemID, int32 MinQuantity)
{
    return HasItem(ItemID, MinQuantity);
}

bool UInventoryComponent::HasItem(FName ItemID, int32 MinQuantity) const
{
    return GetItemCount(ItemID) >= MinQuantity;
}

int32 UInventoryComponent::FindEmptySlot(FGameplayTag InInventoryTypeTag) const
{
    const TArray<FInventorySlot>* InventoryArray = GetInventoryArray(InInventoryTypeTag);
    if (!InventoryArray) return INDEX_NONE;
    
    for (int32 i = 0; i < InventoryArray->Num(); ++i)
    {
        if ((*InventoryArray)[i].IsEmpty())
        {
            return i;
        }
    }
    return INDEX_NONE;
}

int32 UInventoryComponent::FindSlotIndexByInstanceID(FGuid InstanceID) const
{
    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        if (Inventory[i].InstanceID == InstanceID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

bool UInventoryComponent::ValidateSlotIndex(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const
{
    const TArray<FInventorySlot>* InventoryArray = GetInventoryArray(InInventoryTypeTag);
    return InventoryArray && InventoryArray->IsValidIndex(SlotIndex);
}

bool UInventoryComponent::TryAutoStack(FName ItemID, int32& Quantity, float Quality)
{
    if (Quantity <= 0) return false;
    
    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData) || ItemData.MaxStackSize <= 1) return false;
    
    for (FInventorySlot& Slot : Inventory)
    {
        if (Slot.ItemID == ItemID && 
            FMath::Abs(Slot.Quality - Quality) < 0.01f &&
            Slot.Quantity < ItemData.MaxStackSize)
        {
            int32 SpaceAvailable = ItemData.MaxStackSize - Slot.Quantity;
            int32 ToAdd = FMath::Min(SpaceAvailable, Quantity);
            
            Slot.Quantity += ToAdd;
            Quantity -= ToAdd;
            
            if (Quantity <= 0) return true;
        }
    }
    return Quantity <= 0;
}

bool UInventoryComponent::TryConsumeItem(const FName& ItemID, int32& Quantity, int32 SlotIndex)
{
    if (GetItemCount(ItemID) < Quantity) return false;
    
    if (GetOwner()->HasAuthority())
    {
        Internal_ConsumeItem(ItemID, Quantity);
    }
    else
    {
        Server_TryConsumeItem(ItemID, Quantity);
    }
    
    return true;

}

void UInventoryComponent::BroadcastSlotChange(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    OnInventorySlotChanged.Broadcast(InInventoryTypeTag, SlotIndex);
    OnInventoryChanged.Broadcast(InInventoryTypeTag);
}

FItemPreviewData UInventoryComponent::GetItemPreviewData(int32 SlotIndex) const
{
    FItemPreviewData Preview;
    
    if (!Inventory.IsValidIndex(SlotIndex)) return Preview;
    
    const FInventorySlot& Slot = Inventory[SlotIndex];
    if (Slot.IsEmpty()) return Preview;
    
    // Get static data from DataTable
    UDataTable* DT = UItemJsonReader::GetItemDataTable();
    if (!DT) return Preview;
    
    const FItemData* ItemData = DT->FindRow<FItemData>(Slot.ItemID, TEXT("GetItemPreviewData"));
    if (!ItemData) return Preview;
    
    // Populate preview
    Preview.ID = Slot.ItemID;
    Preview.DisplayName = ItemData->DisplayName;
    Preview.Description = ItemData->Description;
    Preview.Icon = ItemData->Soft_Icon;
    Preview.Rarity = ItemData->Rarity;
    Preview.ItemType = ItemData->BaseTags.First(); // Or specific type tag
    Preview.Quantity = Slot.Quantity;
    Preview.CurrentQuality = Slot.Quality * 100.0f;
    Preview.Value = ItemData->BaseValue;
    Preview.Stats.Add(TEXT("Weight"), ItemData->Weight);
    // Add stats from CustomData
    Preview.Stats = Slot.CustomData;
    
    // Add durability if applicable
    if (ItemData->bHasDurability)
    {
        Preview.Stats.Add(TEXT("Durability"), Slot.Durability * 100.0f);
    }
    // Type-specific stats
    if (ItemData->IsRangedWeapon() && ItemData->RangedStats.IsValid())
    {
        Preview.Stats.Add(TEXT("DPS"), ItemData->RangedStats.GetDPS());
        Preview.Stats.Add(TEXT("Damage"), ItemData->RangedStats.Damage);
        Preview.Stats.Add(TEXT("RateOfFire"), ItemData->RangedStats.RateOfFire);
        Preview.Stats.Add(TEXT("Accuracy"), ItemData->RangedStats.Accuracy * 100.0f);
        Preview.Stats.Add(TEXT("Range"), ItemData->RangedStats.Range);
        Preview.Stats.Add(TEXT("MagSize"), ItemData->RangedStats.MagSize);
        Preview.Stats.Add(TEXT("ReloadTime"), ItemData->RangedStats.ReloadTime);
    }
    else if (ItemData->IsMeleeWeapon() && ItemData->MeleeStats.IsValid())
    {
        Preview.Stats.Add(TEXT("DPS"), ItemData->MeleeStats.GetDPS());
        Preview.Stats.Add(TEXT("Damage"), ItemData->MeleeStats.Damage);
        Preview.Stats.Add(TEXT("AttackSpeed"), ItemData->MeleeStats.AttackSpeed);
        Preview.Stats.Add(TEXT("Range"), ItemData->MeleeStats.Range);
        Preview.Stats.Add(TEXT("StaminaCost"), ItemData->MeleeStats.StaminaCost);
    }
    else if (ItemData->IsThrowable() && ItemData->ThrowableStats.IsValid())
    {
        if (ItemData->ThrowableStats.bCanCauseDamage)
        {
            Preview.Stats.Add(TEXT("Damage"), ItemData->ThrowableStats.Damage);
        }
        Preview.Stats.Add(TEXT("ImpactRadius"), ItemData->ThrowableStats.ImpactRadius);
        Preview.Stats.Add(TEXT("FuseTime"), ItemData->ThrowableStats.FuseTime);
    }
    else if (ItemData->IsArmor() && ItemData->ArmorStats.IsValid())
    {
        Preview.Stats.Add(TEXT("ArmorPoints"), ItemData->ArmorStats.ArmorPoints);
        Preview.Stats.Add(TEXT("Protection"), ItemData->ArmorStats.DamageReduction * 100.0f);
        Preview.Stats.Add(TEXT("Mobility"), ItemData->ArmorStats.MovementModifier * 100.0f);
    }
    else if (ItemData->IsConsumable() && ItemData->ConsumableStats.IsValid())
    {
        if (ItemData->ConsumableStats.HealthRestore > 0.0f)
            Preview.Stats.Add(TEXT("HealthRestore"), ItemData->ConsumableStats.HealthRestore);
        if (ItemData->ConsumableStats.StaminaRestore > 0.0f)
            Preview.Stats.Add(TEXT("StaminaRestore"), ItemData->ConsumableStats.StaminaRestore);
        if (ItemData->ConsumableStats.Duration > 0.0f)
            Preview.Stats.Add(TEXT("Duration"), ItemData->ConsumableStats.Duration);
    }

    return Preview;
}


// ============================================================================
// CORE ITEM OPERATIONS (Public API - routes to server or internal)
// ============================================================================

bool UInventoryComponent::ToggleInventory()
{
    return true;
}

bool UInventoryComponent::AddItem(FName ItemID, int32 Quantity, float Quality, float Durability, bool bIsStolen)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_AddItem(ItemID, Quantity, Quality, Durability, bIsStolen);
        return true;
    }
    return Internal_AddItem(ItemID, Quantity, Quality, Durability, bIsStolen);
}


bool UInventoryComponent::RemoveItemByInstance(FGuid InstanceID, int32 Quantity)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_RemoveItem(InstanceID, Quantity);
        return true;
    }
    return Internal_RemoveItemByInstance(InstanceID, Quantity);
}

bool UInventoryComponent::MoveItem(FGameplayTag SourceType, int32 SourceSlot, 
                                    FGameplayTag DestType, int32 DestSlot)
{
    // Server/Listen Server host - execute directly
    if (GetOwner()->HasAuthority())
    {
        return Internal_MoveItem(SourceType, SourceSlot, DestType, DestSlot);
    }

    // Client - predict locally, then send to server
    int32 PredictionID = 0;
    if (Predicted_MoveItem(SourceType, SourceSlot, DestType, DestSlot, PredictionID))
    {
        Server_MoveItem(SourceType, SourceSlot, DestType, DestSlot, PredictionID);
        return true;
    }
    
    return false;
}


bool UInventoryComponent::SplitStack(FGameplayTag SourceType, int32 SourceSlot, 
                                      int32 SplitAmount, FGameplayTag DestType, int32 DestSlot)
{
    if (GetOwner()->HasAuthority())
    {
        return Internal_SplitStack(SourceType, SourceSlot, SplitAmount, DestType, DestSlot);
    }

    int32 PredictionID = 0;
    if (Predicted_SplitStack(SourceType, SourceSlot, SplitAmount, DestType, DestSlot, PredictionID))
    {
        Server_SplitStack(SourceType, SourceSlot, SplitAmount, DestType, DestSlot, PredictionID);
        return true;
    }
    
    return false;
}

bool UInventoryComponent::DropItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex, int32 Quantity)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_DropItem(InInventoryTypeTag, SlotIndex, Quantity);
        return true;
    }
    return Internal_DropItem(InInventoryTypeTag, SlotIndex, Quantity);
}

bool UInventoryComponent::UseItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_UseItem(InInventoryTypeTag, SlotIndex);
        return true;
    }
    return Internal_UseItem(InInventoryTypeTag, SlotIndex);
}

bool UInventoryComponent::EquipItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_EquipItem(InInventoryTypeTag, SlotIndex);
        return true;
    }
    return Internal_EquipItem(InInventoryTypeTag, SlotIndex);
}

bool UInventoryComponent::UnequipItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_UnequipItem(InInventoryTypeTag, SlotIndex);
        return true;
    }
    return Internal_UnequipItem(InInventoryTypeTag, SlotIndex);
}

bool UInventoryComponent::ToggleFavorite(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    if (GetOwner()->HasAuthority())
    {
        return Internal_ToggleFavorite(InInventoryTypeTag, SlotIndex);
    }

    int32 PredictionID = 0;
    if (Predicted_ToggleFavorite(InInventoryTypeTag, SlotIndex, PredictionID))
    {
        Server_ToggleFavorite(InInventoryTypeTag, SlotIndex, PredictionID);
        return true;
    }
    
    return false;
}



void UInventoryComponent::CombineSimilarItems(FGameplayTag InInventoryTypeTag, int32 SourceSlotIndex)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_CombineSimilarItems(InInventoryTypeTag, SourceSlotIndex);
        return;
    }
    Internal_CombineSimilarItems(InInventoryTypeTag, SourceSlotIndex);
}

TArray<FStatComparisonEntry> UInventoryComponent::BuildStatComparison(const FInventorySlot& CurrentItem, const FInventorySlot& CompareItem) const
{
    TArray<FStatComparisonEntry> Entries;
    UDataTable* DT = UItemJsonReader::GetItemDataTable();
    if (!DT) return Entries;
    
    const FItemData* A = DT->FindRow<FItemData>(CurrentItem.ItemID, TEXT("Compare"));
    const FItemData* B = DT->FindRow<FItemData>(CompareItem.ItemID, TEXT("Compare"));
    if (!A || !B) return Entries;
    
    // Collect all stat keys from both items
    TSet<FName> AllKeys;
    for (const auto& Pair : CurrentItem.CustomData) AllKeys.Add(Pair.Key);
    for (const auto& Pair : CompareItem.CustomData) AllKeys.Add(Pair.Key);
    
    // Build comparison entries
    for (const FName& Key : AllKeys)
    {
        FStatComparisonEntry Entry;
        Entry.StatName = Key;
        Entry.StatDisplayName = FText::FromName(Key); // Or lookup from localization
        Entry.CurrentValue = CurrentItem.CustomData.Contains(Key) ? CurrentItem.CustomData[Key] : 0.0f;
        Entry.CompareValue = CompareItem.CustomData.Contains(Key) ? CompareItem.CustomData[Key] : 0.0f;
        Entries.Add(Entry);
    }

/*TODO: 
 *Wearables and quickslots implmeent these on mouseover as well?
 *need to implement widgets too
 */
    
    // Add durability comparison
    FStatComparisonEntry DurabilityEntry;
    DurabilityEntry.StatName = TEXT("Durability");
    DurabilityEntry.StatDisplayName = FText::FromString(TEXT("Durability"));
    DurabilityEntry.CurrentValue = CurrentItem.Durability * 100.0f;
    DurabilityEntry.CompareValue = CompareItem.Durability * 100.0f;
    Entries.Add(DurabilityEntry);

    //Does this compare the item rarirty?
    // Add quality comparison
    FStatComparisonEntry QualityEntry;
    QualityEntry.StatName = TEXT("Quality");
    QualityEntry.StatDisplayName = FText::FromString(TEXT("Quality"));
    QualityEntry.CurrentValue = CurrentItem.Quality * 100.0f;
    QualityEntry.CompareValue = CompareItem.Quality * 100.0f;
    Entries.Add(QualityEntry);

    // Ranged weapon stats
    if (A->IsRangedWeapon() || B->IsRangedWeapon())
    {
        const FRangedWeaponStats& RA = A->RangedStats;
        const FRangedWeaponStats& RB = B->RangedStats;
        
        AddComparisonEntry(Entries, TEXT("DPS"), TEXT("DPS"), RA.GetDPS(), RB.GetDPS(), true, TEXT(""));
        AddComparisonEntry(Entries, TEXT("Damage"), TEXT("Damage"), RA.Damage, RB.Damage, true, TEXT(""));
        AddComparisonEntry(Entries, TEXT("RateOfFire"), TEXT("Fire Rate"), RA.RateOfFire, RB.RateOfFire, true, TEXT("/s"));
        AddComparisonEntry(Entries, TEXT("Accuracy"), TEXT("Accuracy"), RA.Accuracy * 100.f, RB.Accuracy * 100.f, true, TEXT("%"));
        AddComparisonEntry(Entries, TEXT("Range"), TEXT("Range"), RA.Range, RB.Range, true, TEXT("m"));
        AddComparisonEntry(Entries, TEXT("MagSize"), TEXT("Mag Size"), (float)RA.MagSize, (float)RB.MagSize, true, TEXT(""));
        AddComparisonEntry(Entries, TEXT("ReloadTime"), TEXT("Reload"), RA.ReloadTime, RB.ReloadTime, false, TEXT("s"));
        AddComparisonEntry(Entries, TEXT("RecoilV"), TEXT("Vertical Recoil"), RA.RecoilVertical, RB.RecoilVertical, false, TEXT(""));
        AddComparisonEntry(Entries, TEXT("RecoilH"), TEXT("Horizontal Recoil"), RA.RecoilHorizontal, RB.RecoilHorizontal, false, TEXT(""));
    }
    
    // Melee weapon stats
    if (A->IsMeleeWeapon() || B->IsMeleeWeapon())
    {
        const FMeleeWeaponStats& MA = A->MeleeStats;
        const FMeleeWeaponStats& MB = B->MeleeStats;
        
        AddComparisonEntry(Entries, TEXT("DPS"), TEXT("DPS"), MA.GetDPS(), MB.GetDPS(), true, TEXT(""));
        AddComparisonEntry(Entries, TEXT("Damage"), TEXT("Damage"), MA.Damage, MB.Damage, true, TEXT(""));
        AddComparisonEntry(Entries, TEXT("AttackSpeed"), TEXT("Attack Speed"), MA.AttackSpeed, MB.AttackSpeed, true, TEXT("/s"));
        AddComparisonEntry(Entries, TEXT("Range"), TEXT("Reach"), MA.Range, MB.Range, true, TEXT("cm"));
        AddComparisonEntry(Entries, TEXT("StaminaCost"), TEXT("Stamina Cost"), MA.StaminaCost, MB.StaminaCost, false, TEXT(""));
        AddComparisonEntry(Entries, TEXT("BlockEfficiency"), TEXT("Block"), MA.BlockEfficiency * 100.f, MB.BlockEfficiency * 100.f, true, TEXT("%"));
    }
    
    // Throwable stats
    if (A->IsThrowable() || B->IsThrowable())
    {
        const FThrowableWeaponStats& TA = A->ThrowableStats;
        const FThrowableWeaponStats& TB = B->ThrowableStats;
        
        AddComparisonEntry(Entries, TEXT("Damage"), TEXT("Damage"), TA.Damage, TB.Damage, true, TEXT(""));
        AddComparisonEntry(Entries, TEXT("ImpactRadius"), TEXT("Blast Radius"), TA.ImpactRadius, TB.ImpactRadius, true, TEXT("m"));
        AddComparisonEntry(Entries, TEXT("FuseTime"), TEXT("Fuse"), TA.FuseTime, TB.FuseTime, false, TEXT("s"));
        AddComparisonEntry(Entries, TEXT("ThrowForce"), TEXT("Throw Force"), TA.ThrowForce, TB.ThrowForce, true, TEXT(""));
    }
    
    // Armor stats
    if (A->IsArmor() || B->IsArmor())
    {
        const FArmorStats& AA = A->ArmorStats;
        const FArmorStats& AB = B->ArmorStats;
        
        AddComparisonEntry(Entries, TEXT("ArmorPoints"), TEXT("Armor"), (float)AA.ArmorPoints, (float)AB.ArmorPoints, true, TEXT(""));
        AddComparisonEntry(Entries, TEXT("DamageReduction"), TEXT("Protection"), AA.DamageReduction * 100.f, AB.DamageReduction * 100.f, true, TEXT("%"));
        AddComparisonEntry(Entries, TEXT("MovementModifier"), TEXT("Mobility"), AA.MovementModifier * 100.f, AB.MovementModifier * 100.f, true, TEXT("%"));
        AddComparisonEntry(Entries, TEXT("NoiseModifier"), TEXT("Stealth"), (1.f - AA.NoiseModifier) * 100.f, (1.f - AB.NoiseModifier) * 100.f, true, TEXT("%"));
        AddComparisonEntry(Entries, TEXT("ExplosiveResist"), TEXT("Blast Resist"), AA.ExplosiveResist * 100.f, AB.ExplosiveResist * 100.f, true, TEXT("%"));
    }
    
    // Consumable stats
    if (A->IsConsumable() || B->IsConsumable())
    {
        const FConsumableStats& CA = A->ConsumableStats;
        const FConsumableStats& CB = B->ConsumableStats;
        
        AddComparisonEntry(Entries, TEXT("HealthRestore"), TEXT("Health"), CA.HealthRestore, CB.HealthRestore, true, TEXT(""));
        AddComparisonEntry(Entries, TEXT("StaminaRestore"), TEXT("Stamina"), CA.StaminaRestore, CB.StaminaRestore, true, TEXT(""));
        AddComparisonEntry(Entries, TEXT("Duration"), TEXT("Duration"), CA.Duration, CB.Duration, true, TEXT("s"));
        AddComparisonEntry(Entries, TEXT("UseTime"), TEXT("Use Time"), CA.UseTime, CB.UseTime, false, TEXT("s"));
    }
    

    // Common stats (all items)
    AddComparisonEntry(Entries, TEXT("Weight"), TEXT("Weight"), A->Weight, B->Weight, false, TEXT("kg"));
    AddComparisonEntry(Entries, TEXT("Value"), TEXT("Value"), (float)A->BaseValue, (float)B->BaseValue, true, TEXT(""));
    AddComparisonEntry(Entries, TEXT("Durability"), TEXT("Durability"), CurrentItem.Durability * 100.f, CompareItem.Durability * 100.f, true, TEXT("%"));
    AddComparisonEntry(Entries, TEXT("Quality"), TEXT("Quality"), CurrentItem.Quality * 100.f, CompareItem.Quality * 100.f, true, TEXT("%"));
    

    return Entries;
}

// ============================================================================
// INTERNAL SERVER LOGIC (actual implementation)
// ============================================================================

bool UInventoryComponent::Internal_AddItem(FName ItemID, int32 Quantity, float Quality, float Durability, bool bIsStolen)
{
    //TODO: Item Quality should be FGameplayTag, not float...
    /*
     * will be used to determine the color of the slot
     * and also the craftable output quality
     * and the experience player will get.
     */
    if (ItemID.IsNone() || Quantity <= 0) return false;
    
    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData))
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("AddItem failed - Invalid ItemID: %s"), *ItemID.ToString());
        return false;
    }
    
    int32 RemainingQuantity = Quantity;
    TryAutoStack(ItemID, RemainingQuantity, Quality);
    
    while (RemainingQuantity > 0)
    {
        int32 EmptySlot = FindEmptySlot(FWWTagLibrary::Inventory_Type_PlayerInventory());
        if (EmptySlot == INDEX_NONE)
        {
            UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("AddItem - Inventory full, %d items not added"), RemainingQuantity);
            break;
        }
        
        FInventorySlot& Slot = Inventory[EmptySlot];
        Slot.InstanceID = FGuid::NewGuid();
        Slot.ItemID = ItemID;
        Slot.Quantity = FMath::Min(RemainingQuantity, ItemData.MaxStackSize);
        Slot.MaxStackSize = ItemData.MaxStackSize;
        Slot.Quality = Quality;
        Slot.Durability = Durability;  // ← NEW
        Slot.bIsStolen = bIsStolen;
        Slot.ItemTags = ItemData.BaseTags;
        
        RemainingQuantity -= Slot.Quantity;
        
        BroadcastSlotChange(FWWTagLibrary::Inventory_Type_PlayerInventory(), EmptySlot);
        OnItemAdded.Broadcast(ItemID, Slot.Quantity, Slot.InstanceID);
    }
    
    MarkSaveDirty();
    return RemainingQuantity < Quantity;
}


bool UInventoryComponent::Internal_RemoveItemByInstance(FGuid InstanceID, int32 Quantity)
{
    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        FInventorySlot& Slot = Inventory[i];
        if (Slot.InstanceID == InstanceID)
        {
            FName ItemID = Slot.ItemID;
            int32 RemovedQuantity = FMath::Min(Quantity, Slot.Quantity);
            
            Slot.Quantity -= RemovedQuantity;
            if (Slot.Quantity <= 0) Slot.Clear();
            
            BroadcastSlotChange(FWWTagLibrary::Inventory_Type_PlayerInventory(), i);
            OnItemRemoved.Broadcast(ItemID, RemovedQuantity, InstanceID);
            MarkSaveDirty();
            return true;
        }
    }
    return false;
}

bool UInventoryComponent::Internal_MoveItem(FGameplayTag SourceType, int32 SourceSlot, FGameplayTag DestType, int32 DestSlot)
{
    TArray<FInventorySlot>* SourceArray = GetInventoryArray(SourceType);
    TArray<FInventorySlot>* DestArray = GetInventoryArray(DestType);
    
    if (!SourceArray || !DestArray ||
        !SourceArray->IsValidIndex(SourceSlot) || !DestArray->IsValidIndex(DestSlot))
    {
        return false;
    }
    
    FInventorySlot& Source = (*SourceArray)[SourceSlot];
    FInventorySlot& Dest = (*DestArray)[DestSlot];
    
    if (Source.IsEmpty()) return false;
    
    FInventorySlot Temp = Source;
    Source = Dest;
    Dest = Temp;
    
    BroadcastSlotChange(SourceType, SourceSlot);
    BroadcastSlotChange(DestType, DestSlot);
    return true;
}

bool UInventoryComponent::Internal_SplitStack(FGameplayTag SourceType, int32 SourceSlot, int32 SplitAmount, FGameplayTag DestType, int32 DestSlot)
{
    TArray<FInventorySlot>* SourceArray = GetInventoryArray(SourceType);
    TArray<FInventorySlot>* DestArray = GetInventoryArray(DestType);
    
    if (!SourceArray || !DestArray ||
        !SourceArray->IsValidIndex(SourceSlot) || !DestArray->IsValidIndex(DestSlot))
    {
        return false;
    }
    
    FInventorySlot& Source = (*SourceArray)[SourceSlot];
    FInventorySlot& Dest = (*DestArray)[DestSlot];
    
    if (Source.IsEmpty() || !Dest.IsEmpty() || SplitAmount <= 0 || SplitAmount >= Source.Quantity)
    {
        return false;
    }
    
    Dest = Source;
    Dest.InstanceID = FGuid::NewGuid();
    Dest.Quantity = SplitAmount;
    Source.Quantity -= SplitAmount;
    
    BroadcastSlotChange(SourceType, SourceSlot);
    BroadcastSlotChange(DestType, DestSlot);
    return true;
}

bool UInventoryComponent::Internal_DropItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex, int32 Quantity)
{
    if (!ValidateSlotIndex(InInventoryTypeTag, SlotIndex)) return false;
    
    TArray<FInventorySlot>* InventoryArray = GetInventoryArray(InInventoryTypeTag);
    FInventorySlot& Slot = (*InventoryArray)[SlotIndex];
    
    if (Slot.IsEmpty()) return false;
    
    // Cache before modification
    FInventorySlot DroppedSlot = Slot;
    int32 DropQuantity = (Quantity < 0) ? Slot.Quantity : FMath::Min(Quantity, Slot.Quantity);
    DroppedSlot.Quantity = DropQuantity;

    //Spawn in the world
    FVector ViewLoc;
    FRotator ViewRot;
    FVector SpawnScale = FVector (1.0f, 1.0f, 1.0f);
    FTransform SpawnTransform;
    float SpawnDistance = InventoryItemDefaultsConstants::DEFAULT_INVENTORY_ITEM_SPAWN_DISTANCE;

    // Calculate drop Transform
    FVector DropLocation = FVector::ZeroVector;
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        OwnerPawn->GetActorEyesViewPoint(ViewLoc, ViewRot);
        DropLocation = ViewLoc + ViewRot.Vector() * SpawnDistance ;
        
        // Set Transform
        SpawnTransform.SetLocation(DropLocation);
        SpawnTransform.SetRotation(ViewRot.Quaternion());
        SpawnTransform.SetScale3D(SpawnScale);
    }
    else if (AActor* OwnerActor = GetOwner())
    {
        //Calculate Drop Location
        FVector BBOrigin;
        FVector BBExtent;
        FVector Forward = OwnerActor->GetActorForwardVector();
        OwnerActor->GetActorBounds(false, BBOrigin, BBExtent,true);
        
        DropLocation = BBOrigin + Forward * SpawnDistance * BBExtent;
        ViewRot = Forward.Rotation();
        
        SpawnTransform.SetLocation(DropLocation);
        SpawnTransform.SetRotation(ViewRot.Quaternion());
        SpawnTransform.SetScale3D(SpawnScale);
    }
    else
    {
        FString Message = FString::Printf(TEXT(" DropItem: No Valid Owner Found: %s"), *GetName());
        UDebugSubsystem::PrintDebug(this, WWDebugTags::Debug_Inventory(), Message, false, EDebugVerbosity::Warning);
        return false;
    }
    
    // Remove from inventory
    Slot.Quantity -= DropQuantity;
    if (Slot.Quantity <= 0) Slot.Clear();
        
    //Spawn in the world
    if (UUniversalSpawnManager* SpawnManager = UUniversalSpawnManager::Get(this))
    {
        FItemData ItemData;
        
        if (GetItemData(DroppedSlot.ItemID, ItemData))
        {
            SpawnManager->SpawnActor(
                SpawnTransform.GetLocation(),
                SpawnTransform.GetRotation().Rotator(),
                ItemData.PickupActorClass,
                DroppedSlot.ItemID,
                DropQuantity,
                DroppedSlot.Durability,
                true);
        }
    }
    BroadcastSlotChange(InInventoryTypeTag, SlotIndex);
    OnItemDropped.Broadcast(DroppedSlot, DropQuantity, SpawnTransform);

    return true;
}

bool UInventoryComponent::Internal_UseItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    if (!ValidateSlotIndex(InInventoryTypeTag, SlotIndex)) return false;
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("UseItem called for slot %d"), SlotIndex);
    // TODO: Implement actual use logic
    return true;
}

bool UInventoryComponent::Internal_EquipItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    if (!ValidateSlotIndex(InInventoryTypeTag, SlotIndex)) return false;
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("EquipItem called for slot %d"), SlotIndex);
    // TODO: Implement actual equip logic
    return true;
}

bool UInventoryComponent::Internal_UnequipItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    if (!ValidateSlotIndex(InInventoryTypeTag, SlotIndex)) return false;
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("UnequipItem called for slot %d"), SlotIndex);
    // TODO: Implement actual unequip logic
    return true;
}

bool UInventoryComponent::Internal_ToggleFavorite(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    if (!ValidateSlotIndex(InInventoryTypeTag, SlotIndex)) return false;
    
    TArray<FInventorySlot>* InventoryArray = GetInventoryArray(InInventoryTypeTag);
    FInventorySlot& Slot = (*InventoryArray)[SlotIndex];
    
    if (Slot.IsEmpty()) return false;
    
    Slot.bIsFavorite = !Slot.bIsFavorite;
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("Toggled favorite for slot %d - Now: %s"), 
        SlotIndex, Slot.bIsFavorite ? TEXT("Favorited") : TEXT("Not Favorited"));
    
    BroadcastSlotChange(InInventoryTypeTag, SlotIndex);
    return true;
}

void UInventoryComponent::Internal_CombineSimilarItems(FGameplayTag InInventoryTypeTag, int32 SourceSlotIndex)
{
    TArray<FInventorySlot>* InventorySlots = GetInventoryArray(InInventoryTypeTag);
    if (!InventorySlots || !InventorySlots->IsValidIndex(SourceSlotIndex))
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("CombineSimilarItems failed - Invalid inventory or slot index"));
        return;
    }
    
    const FInventorySlot& SourceSlot = (*InventorySlots)[SourceSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("CombineSimilarItems failed - Source slot is empty"));
        return;
    }
    
    if (SourceSlot.bIsFavorite)
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, TEXT("CombineSimilarItems failed - Cannot combine favorited items"));
        return;
    }
    
    const FName TargetItemID = SourceSlot.ItemID;
    const int32 TargetRarity = SourceSlot.Rarity;
    const int32 MaxStackSize = SourceSlot.MaxStackSize;
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("Starting combine for Item: %s, Rarity: %d, MaxStack: %d"),
        *TargetItemID.ToString(), TargetRarity, MaxStackSize);
    
    TArray<int32> SimilarSlotIndices;
    int32 TotalQuantity = 0;
    
    for (int32 i = 0; i < InventorySlots->Num(); ++i)
    {
        const FInventorySlot& Slot = (*InventorySlots)[i];
        
        if (Slot.ItemID == TargetItemID &&
            Slot.Rarity == TargetRarity &&
            !Slot.bIsFavorite &&
            !Slot.IsEmpty())
        {
            SimilarSlotIndices.Add(i);
            TotalQuantity += Slot.Quantity;
            
            UE_LOG(LogInventoryInteractableSystem, Log, 
                TEXT("  Found similar item in slot %d - Quantity: %d"), i, Slot.Quantity);
        }
    }
    
    if (SimilarSlotIndices.Num() <= 1)
    {
        UE_LOG(LogInventoryInteractableSystem, Warning, 
            TEXT("No similar items to combine (found %d slots)"), SimilarSlotIndices.Num());
        return;
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("Found %d similar items with total quantity: %d"),
        SimilarSlotIndices.Num(), TotalQuantity);
    
    int32 FullStacksNeeded = TotalQuantity / MaxStackSize;
    int32 Remainder = TotalQuantity % MaxStackSize;
    int32 SlotsNeeded = FullStacksNeeded + (Remainder > 0 ? 1 : 0);
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("Optimal: %d full stacks + %d remainder = %d slots needed (currently using %d)"),
        FullStacksNeeded, Remainder, SlotsNeeded, SimilarSlotIndices.Num());
    
    FInventorySlot TemplateSlot = (*InventorySlots)[SimilarSlotIndices[0]];
    
    for (int32 Index : SimilarSlotIndices)
    {
        (*InventorySlots)[Index].Clear();
    }
    
    int32 RemainingQuantity = TotalQuantity;
    int32 SlotsFilled = 0;
    
    for (int32 i = 0; i < SimilarSlotIndices.Num() && RemainingQuantity > 0; ++i)
    {
        int32 SlotIndex = SimilarSlotIndices[i];
        FInventorySlot& Slot = (*InventorySlots)[SlotIndex];
        
        int32 QuantityForThisSlot = FMath::Min(RemainingQuantity, MaxStackSize);
        
        Slot = TemplateSlot;
        Slot.InstanceID = FGuid::NewGuid();
        Slot.Quantity = QuantityForThisSlot;
        
        RemainingQuantity -= QuantityForThisSlot;
        SlotsFilled++;
        
        UE_LOG(LogInventoryInteractableSystem, Log, 
            TEXT("  Created stack in slot %d with quantity %d"), SlotIndex, QuantityForThisSlot);
    }
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("Combine complete! Used %d slots (freed %d slots)"),
        SlotsFilled, SimilarSlotIndices.Num() - SlotsFilled);
    
    OnInventoryChanged.Broadcast(InInventoryTypeTag);
}

void UInventoryComponent::Internal_ConsumeItem(const FName& ItemID, int32 Quantity)
{
    int32 RemainingToConsume = Quantity;
    
    for (FInventorySlot& Slot : Inventory)
    {
        if (Slot.ItemID != ItemID) continue;
        if (RemainingToConsume <= 0) break;
        
        int32 ConsumeFromThisSlot = FMath::Min(RemainingToConsume, Slot.Quantity);
        Slot.Quantity -= ConsumeFromThisSlot;
        RemainingToConsume -= ConsumeFromThisSlot;
        
        if (Slot.Quantity <= 0)
        {
            Slot.Clear();
        }
    }
    
    OnItemConsumed.Broadcast(ItemID, Quantity);

}

bool UInventoryComponent::Internal_SetSlot(int32 Index, FInventorySlot& Slot)
{
    if (!Inventory.IsValidIndex(Index)) return false;
    FInventorySlot OldSlot = Inventory[Index];
    Inventory[Index] = Slot;

    BroadcastSlotChange(InventoryTypeTag, Index);
    return true;
}

// ============================================================================
// SERVER RPC VALIDATION & IMPLEMENTATION
// ============================================================================

bool UInventoryComponent::Server_AddItem_Validate(FName ItemID, int32 Quantity, float Quality, float Durability, bool bIsStolen)
{
    return !ItemID.IsNone() && Quantity > 0 && Durability >= 0.0f && Durability <= 1.0f;
}

void UInventoryComponent::Server_AddItem_Implementation(FName ItemID, int32 Quantity, float Quality, float Durability, bool bIsStolen)
{
    Internal_AddItem(ItemID, Quantity, Quality, Durability, bIsStolen);
}


bool UInventoryComponent::Server_RemoveItem_Validate(FGuid InstanceID, int32 Quantity)
{
    return InstanceID.IsValid() && Quantity > 0 && Quantity < 9999;
}

void UInventoryComponent::Server_RemoveItem_Implementation(FGuid InstanceID, int32 Quantity)
{
    Internal_RemoveItemByInstance(InstanceID, Quantity);
}

bool UInventoryComponent::Server_DropItem_Validate(FGameplayTag InInventoryTypeTag, int32 SlotIndex, int32 Quantity)
{
    return InInventoryTypeTag.IsValid() && SlotIndex >= 0 && SlotIndex < MaxInventorySlots;
}

void UInventoryComponent::Server_DropItem_Implementation(FGameplayTag InInventoryTypeTag, int32 SlotIndex, int32 Quantity)
{
    Internal_DropItem(InInventoryTypeTag, SlotIndex, Quantity);
}

bool UInventoryComponent::Server_MoveItem_Validate(FGameplayTag SourceType, int32 SourceSlot,
                                                    FGameplayTag DestType, int32 DestSlot,
                                                    int32 PredictionID)
{
    return SourceType.IsValid() && DestType.IsValid() &&
           SourceSlot >= 0 && SourceSlot < MaxInventorySlots &&
           DestSlot >= 0 && DestSlot < MaxInventorySlots;
}



void UInventoryComponent::Server_MoveItem_Implementation(FGameplayTag SourceType, int32 SourceSlot,
                                                          FGameplayTag DestType, int32 DestSlot,
                                                          int32 PredictionID)
{
    bool bSuccess = Internal_MoveItem(SourceType, SourceSlot, DestType, DestSlot);
    
    // Tell client whether prediction was correct
    if (PredictionID > 0)
    {
        Client_ConfirmPrediction(PredictionID, bSuccess, 
                                  bSuccess ? TEXT("") : TEXT("Server validation failed"));
    }
}

bool UInventoryComponent::Server_SplitStack_Validate(FGameplayTag SourceType, int32 SourceSlot,
                                                      int32 SplitAmount, FGameplayTag DestType, int32 DestSlot,
                                                      int32 PredictionID)
{
    return SourceType.IsValid() && DestType.IsValid() &&
           SourceSlot >= 0 && SourceSlot < MaxInventorySlots &&
           DestSlot >= 0 && DestSlot < MaxInventorySlots &&
           SplitAmount > 0 && SplitAmount < 9999;
}


void UInventoryComponent::Server_SplitStack_Implementation(FGameplayTag SourceType, int32 SourceSlot,
                                                            int32 SplitAmount, FGameplayTag DestType, int32 DestSlot,
                                                            int32 PredictionID)
{
    bool bSuccess = Internal_SplitStack(SourceType, SourceSlot, SplitAmount, DestType, DestSlot);
    
    if (PredictionID > 0)
    {
        Client_ConfirmPrediction(PredictionID, bSuccess,
                                  bSuccess ? TEXT("") : TEXT("Server validation failed"));
    }
}

bool UInventoryComponent::Server_UseItem_Validate(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    return InInventoryTypeTag.IsValid() && SlotIndex >= 0 && SlotIndex < MaxInventorySlots;
}

void UInventoryComponent::Server_UseItem_Implementation(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    Internal_UseItem(InInventoryTypeTag, SlotIndex);
}

bool UInventoryComponent::Server_EquipItem_Validate(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    return InInventoryTypeTag.IsValid() && SlotIndex >= 0 && SlotIndex < MaxInventorySlots;
}

void UInventoryComponent::Server_EquipItem_Implementation(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    Internal_EquipItem(InInventoryTypeTag, SlotIndex);
}

bool UInventoryComponent::Server_UnequipItem_Validate(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    return InInventoryTypeTag.IsValid() && SlotIndex >= 0 && SlotIndex < MaxInventorySlots;
}

void UInventoryComponent::Server_UnequipItem_Implementation(FGameplayTag InInventoryTypeTag, int32 SlotIndex)
{
    Internal_UnequipItem(InInventoryTypeTag, SlotIndex);
}

bool UInventoryComponent::Server_ToggleFavorite_Validate(FGameplayTag InInventoryTypeTag, int32 SlotIndex,
                                                          int32 PredictionID)
{
    return InInventoryTypeTag.IsValid() && SlotIndex >= 0 && SlotIndex < MaxInventorySlots;
}


void UInventoryComponent::Server_ToggleFavorite_Implementation(FGameplayTag InInventoryTypeTag, int32 SlotIndex,
                                                                int32 PredictionID)
{
    bool bSuccess = Internal_ToggleFavorite(InInventoryTypeTag, SlotIndex);
    
    if (PredictionID > 0)
    {
        Client_ConfirmPrediction(PredictionID, bSuccess,
                                  bSuccess ? TEXT("") : TEXT("Server validation failed"));
    }
}


bool UInventoryComponent::Server_CombineSimilarItems_Validate(FGameplayTag InInventoryTypeTag, int32 SourceSlotIndex)
{
    return InInventoryTypeTag.IsValid() && SourceSlotIndex >= 0 && SourceSlotIndex < MaxInventorySlots;
}

void UInventoryComponent::Server_CombineSimilarItems_Implementation(FGameplayTag InInventoryTypeTag, int32 SourceSlotIndex)
{
    Internal_CombineSimilarItems(InInventoryTypeTag, SourceSlotIndex);
}

// ============================================================================
// SLOT QUERY HELPERS
// ============================================================================

bool UInventoryComponent::IsSlotEquippable(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const
{
    FInventorySlot SlotData;
    if (!GetSlot(InInventoryTypeTag, SlotIndex, SlotData) || SlotData.IsEmpty()) return false;
    
    FItemData ItemData;
    if (!GetItemData(SlotData.ItemID, ItemData)) return false;
    
    return ItemData.BaseTags.HasTag(FWWTagLibrary::Inventory_Item_Behavior_Equip());
}

bool UInventoryComponent::IsSlotEquipped(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const
{
    FInventorySlot SlotData;
    if (!GetSlot(InInventoryTypeTag, SlotIndex, SlotData) || SlotData.IsEmpty()) return false;
    
    return InInventoryTypeTag == FWWTagLibrary::Inventory_Type_Equipment();
}

bool UInventoryComponent::IsSlotUsable(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const
{
    FInventorySlot SlotData;
    if (!GetSlot(InInventoryTypeTag, SlotIndex, SlotData) || SlotData.IsEmpty()) return false;
    
    FItemData ItemData;
    if (!GetItemData(SlotData.ItemID, ItemData)) return false;
    
    return ItemData.BaseTags.HasTag(FWWTagLibrary::Inventory_Item_Behavior_Use()) ||
           ItemData.BaseTags.HasTag(FWWTagLibrary::Inventory_Item_Behavior_Consume()) ||
           ItemData.BaseTags.HasTag(FWWTagLibrary::Inventory_Item_Behavior_Activate());
}

bool UInventoryComponent::IsSlotDroppable(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const
{
    FInventorySlot SlotData;
    if (!GetSlot(InInventoryTypeTag, SlotIndex, SlotData) || SlotData.IsEmpty()) return false;
    
    FItemData ItemData;
    if (!GetItemData(SlotData.ItemID, ItemData)) return false;
    
    return !ItemData.BaseTags.HasTag(FWWTagLibrary::Inventory_Item_Flags_NoDrop());
}

bool UInventoryComponent::IsSlotAttachment(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const
{
    FInventorySlot SlotData;
    if (!GetSlot(InInventoryTypeTag, SlotIndex, SlotData) || SlotData.IsEmpty()) return false;
    
    FItemData ItemData;
    if (!GetItemData(SlotData.ItemID, ItemData)) return false;
    
    return ItemData.BaseTags.HasTag(FWWTagLibrary::Inventory_Item_Type_Attachment());
}

bool UInventoryComponent::IsSlotCombinable(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const
{
    return CanCombineItem(InInventoryTypeTag, SlotIndex);
}

bool UInventoryComponent::CanAttachToSlot(FGameplayTag AttachmentInInventoryTypeTag, int32 AttachmentSlotIndex,
                                           FGameplayTag TargetInInventoryTypeTag, int32 TargetSlotIndex) const
{
    FInventorySlot AttachmentSlot;
    if (!GetSlot(AttachmentInInventoryTypeTag, AttachmentSlotIndex, AttachmentSlot) || AttachmentSlot.IsEmpty())
    {
        return false;
    }
    
    FInventorySlot TargetSlot;
    if (!GetSlot(TargetInInventoryTypeTag, TargetSlotIndex, TargetSlot) || TargetSlot.IsEmpty())
    {
        return false;
    }
    
    FItemData AttachmentData;
    FItemData TargetData;
    if (!GetItemData(AttachmentSlot.ItemID, AttachmentData) || !GetItemData(TargetSlot.ItemID, TargetData))
    {
        return false;
    }
    
    if (!AttachmentData.BaseTags.HasTag(FWWTagLibrary::Inventory_Item_Type_Attachment()))
    {
        return false;
    }
    
    if (!TargetData.BaseTags.HasTag(FWWTagLibrary::Inventory_Item_Behavior_Equip()))
    {
        return false;
    }
    
    return true;
}

// ============================================================================
// COMBINE QUERY HELPERS
// ============================================================================

bool UInventoryComponent::CanCombineItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const
{
    const TArray<FInventorySlot>* InventorySlots = GetInventoryArray(InInventoryTypeTag);
    if (!InventorySlots || !InventorySlots->IsValidIndex(SlotIndex)) return false;
    
    const FInventorySlot& SourceSlot = (*InventorySlots)[SlotIndex];
    if (SourceSlot.IsEmpty() || SourceSlot.bIsFavorite) return false;
    
    int32 SimilarItemCount = 0;
    int32 EmptySlotCount = 0;
    
    for (int32 i = 0; i < InventorySlots->Num(); ++i)
    {
        const FInventorySlot& Slot = (*InventorySlots)[i];
        
        if (Slot.IsEmpty())
        {
            EmptySlotCount++;
        }
        else if (i != SlotIndex &&
                 Slot.ItemID == SourceSlot.ItemID &&
                 Slot.Rarity == SourceSlot.Rarity &&
                 !Slot.bIsFavorite)
        {
            SimilarItemCount++;
        }
    }
    
    return (SimilarItemCount > 0) && (EmptySlotCount > 0);
}

int32 UInventoryComponent::GetCombinableItemCount(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const
{
    const TArray<FInventorySlot>* InventorySlots = GetInventoryArray(InInventoryTypeTag);
    if (!InventorySlots || !InventorySlots->IsValidIndex(SlotIndex)) return 0;
    
    const FInventorySlot& SourceSlot = (*InventorySlots)[SlotIndex];
    if (SourceSlot.IsEmpty()) return 0;
    
    int32 Count = 0;
    for (int32 i = 0; i < InventorySlots->Num(); ++i)
    {
        if (i == SlotIndex) continue;
        
        const FInventorySlot& Slot = (*InventorySlots)[i];
        if (!Slot.IsEmpty() &&
            Slot.ItemID == SourceSlot.ItemID &&
            Slot.Rarity == SourceSlot.Rarity &&
            !Slot.bIsFavorite)
        {
            Count++;
        }
    }
    
    return Count;
}

// ============================================================================
// PREDICTION HELPERS
// ============================================================================

int32 UInventoryComponent::GeneratePredictionID()
{
    return ++PredictionIDCounter;
}

void UInventoryComponent::CachePrediction(int32 PredictionID, const FInventoryPrediction& Prediction)
{
    FInventoryPrediction CachedPrediction = Prediction;
    CachedPrediction.Timestamp = FPlatformTime::Seconds();
    PendingPredictions.Add(PredictionID, CachedPrediction);
}

void UInventoryComponent::CleanupPrediction(int32 PredictionID)
{
    PendingPredictions.Remove(PredictionID);
}

void UInventoryComponent::RollbackPrediction(int32 PredictionID)
{
    FInventoryPrediction* Prediction = PendingPredictions.Find(PredictionID);
    if (!Prediction)
    {
        return;
    }

    // Restore source slot
    if (Prediction->SourceSlotIndex != INDEX_NONE)
    {
        TArray<FInventorySlot>* SourceArray = GetInventoryArray(Prediction->SourceInventoryType);
        if (SourceArray && SourceArray->IsValidIndex(Prediction->SourceSlotIndex))
        {
            (*SourceArray)[Prediction->SourceSlotIndex] = Prediction->SourceSlotCache;
            BroadcastSlotChange(Prediction->SourceInventoryType, Prediction->SourceSlotIndex);
        }
    }

    // Restore dest slot (if applicable)
    if (Prediction->DestSlotIndex != INDEX_NONE)
    {
        TArray<FInventorySlot>* DestArray = GetInventoryArray(Prediction->DestInventoryType);
        if (DestArray && DestArray->IsValidIndex(Prediction->DestSlotIndex))
        {
            (*DestArray)[Prediction->DestSlotIndex] = Prediction->DestSlotCache;
            BroadcastSlotChange(Prediction->DestInventoryType, Prediction->DestSlotIndex);
        }
    }

    UE_LOG(LogInventoryInteractableSystem, Warning, 
           TEXT("Prediction %d rolled back: %s"), PredictionID, *Prediction->OperationType.ToString());

    CleanupPrediction(PredictionID);
}

// ============================================================================
// PREDICTED LOCAL OPERATIONS (Client-Side)
// ============================================================================

bool UInventoryComponent::Predicted_MoveItem(FGameplayTag SourceType, int32 SourceSlot,
                                              FGameplayTag DestType, int32 DestSlot,
                                              int32& OutPredictionID)
{
    TArray<FInventorySlot>* SourceArray = GetInventoryArray(SourceType);
    TArray<FInventorySlot>* DestArray = GetInventoryArray(DestType);

    if (!SourceArray || !DestArray ||
        !SourceArray->IsValidIndex(SourceSlot) || !DestArray->IsValidIndex(DestSlot))
    {
        return false;
    }

    FInventorySlot& Source = (*SourceArray)[SourceSlot];
    FInventorySlot& Dest = (*DestArray)[DestSlot];

    if (Source.IsEmpty())
    {
        return false;
    }

    // Cache state BEFORE modification
    OutPredictionID = GeneratePredictionID();
    
    FInventoryPrediction Prediction;
    Prediction.OperationType = FName("MoveItem");
    Prediction.SourceInventoryType = SourceType;
    Prediction.SourceSlotIndex = SourceSlot;
    Prediction.SourceSlotCache = Source;
    Prediction.DestInventoryType = DestType;
    Prediction.DestSlotIndex = DestSlot;
    Prediction.DestSlotCache = Dest;
    
    CachePrediction(OutPredictionID, Prediction);

    // Apply locally (same logic as Internal_MoveItem)
    FInventorySlot Temp = Source;
    Source = Dest;
    Dest = Temp;

    // Broadcast for UI update
    BroadcastSlotChange(SourceType, SourceSlot);
    BroadcastSlotChange(DestType, DestSlot);

    return true;
}

bool UInventoryComponent::Predicted_SplitStack(FGameplayTag SourceType, int32 SourceSlot,
                                                int32 SplitAmount, FGameplayTag DestType, int32 DestSlot,
                                                int32& OutPredictionID)
{
    TArray<FInventorySlot>* SourceArray = GetInventoryArray(SourceType);
    TArray<FInventorySlot>* DestArray = GetInventoryArray(DestType);

    if (!SourceArray || !DestArray ||
        !SourceArray->IsValidIndex(SourceSlot) || !DestArray->IsValidIndex(DestSlot))
    {
        return false;
    }

    FInventorySlot& Source = (*SourceArray)[SourceSlot];
    FInventorySlot& Dest = (*DestArray)[DestSlot];

    if (Source.IsEmpty() || !Dest.IsEmpty() || SplitAmount <= 0 || SplitAmount >= Source.Quantity)
    {
        return false;
    }

    // Cache state BEFORE modification
    OutPredictionID = GeneratePredictionID();
    
    FInventoryPrediction Prediction;
    Prediction.OperationType = FName("SplitStack");
    Prediction.SourceInventoryType = SourceType;
    Prediction.SourceSlotIndex = SourceSlot;
    Prediction.SourceSlotCache = Source;
    Prediction.DestInventoryType = DestType;
    Prediction.DestSlotIndex = DestSlot;
    Prediction.DestSlotCache = Dest;
    
    CachePrediction(OutPredictionID, Prediction);

    // Apply locally (same logic as Internal_SplitStack)
    Dest = Source;
    Dest.InstanceID = FGuid::NewGuid();
    Dest.Quantity = SplitAmount;
    Source.Quantity -= SplitAmount;

    BroadcastSlotChange(SourceType, SourceSlot);
    BroadcastSlotChange(DestType, DestSlot);

    return true;
}

bool UInventoryComponent::Predicted_ToggleFavorite(FGameplayTag InInventoryTypeTag, int32 SlotIndex,
                                                    int32& OutPredictionID)
{
    TArray<FInventorySlot>* InventoryArray = GetInventoryArray(InInventoryTypeTag);
    if (!InventoryArray || !InventoryArray->IsValidIndex(SlotIndex))
    {
        return false;
    }

    FInventorySlot& Slot = (*InventoryArray)[SlotIndex];
    if (Slot.IsEmpty())
    {
        return false;
    }

    // Cache state BEFORE modification
    OutPredictionID = GeneratePredictionID();
    
    FInventoryPrediction Prediction;
    Prediction.OperationType = FName("ToggleFavorite");
    Prediction.SourceInventoryType = InInventoryTypeTag;
    Prediction.SourceSlotIndex = SlotIndex;
    Prediction.SourceSlotCache = Slot;
    // No dest slot for favorites
    
    CachePrediction(OutPredictionID, Prediction);

    // Apply locally
    Slot.bIsFavorite = !Slot.bIsFavorite;

    BroadcastSlotChange(InInventoryTypeTag, SlotIndex);

    return true;
}

// ============================================================================
// CLIENT RPC - Server Confirmation
// ============================================================================

void UInventoryComponent::Client_ConfirmPrediction_Implementation(int32 PredictionID, 
                                                                   bool bSuccess, 
                                                                   const FString& FailReason)
{
    if (bSuccess)
    {
        // Server confirmed - just cleanup, state already correct
        CleanupPrediction(PredictionID);
        UE_LOG(LogInventoryInteractableSystem, Verbose, 
               TEXT("Prediction %d confirmed"), PredictionID);
    }
    else
    {
        // Server rejected - rollback to cached state
        RollbackPrediction(PredictionID);
        OnPredictionRejected.Broadcast(PredictionID, FailReason);
        UE_LOG(LogInventoryInteractableSystem, Warning,
               TEXT("Prediction %d rejected: %s"), PredictionID, *FailReason);
    }
}

// ============================================================================
// SAVE SYSTEM (ISaveableInterface)
// ============================================================================

FString UInventoryComponent::GetSaveID_Implementation() const
{
    if (AActor* Owner = GetOwner())
    {
        return FString::Printf(TEXT("%s.%s"), *Owner->GetPathName(), *GetClass()->GetName());
    }
    return FString();
}

int32 UInventoryComponent::GetSavePriority_Implementation() const
{
    return 120;
}

FGameplayTag UInventoryComponent::GetSaveType_Implementation() const
{
    return FWWTagLibrary::Save_Category_Component();
}

bool UInventoryComponent::SaveState_Implementation(FSaveRecord& OutRecord)
{
    TArray<uint8> BinaryData;
    FMemoryWriter MemoryWriter(BinaryData, true);
    FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
    Ar.ArIsSaveGame = true;
    Ar.ArNoDelta = false;

    const_cast<UInventoryComponent*>(this)->Serialize(Ar);

    OutRecord.RecordID = FName(*GetSaveID_Implementation());
    OutRecord.RecordType = FWWTagLibrary::Save_Category_Component();
    OutRecord.BinaryData = MoveTemp(BinaryData);
    OutRecord.Timestamp = FDateTime::Now();
    OutRecord.Priority = GetSavePriority_Implementation();

    return true;
}

bool UInventoryComponent::LoadState_Implementation(const FSaveRecord& InRecord)
{
    if (InRecord.BinaryData.Num() == 0)
    {
        return false;
    }

    FMemoryReader MemoryReader(InRecord.BinaryData, true);
    FObjectAndNameAsStringProxyArchive Ar(MemoryReader, false);
    Ar.ArIsSaveGame = true;

    Serialize(Ar);
    OnSaveDataLoaded_Implementation();

    return true;
}

bool UInventoryComponent::IsDirty_Implementation() const
{
    return bSaveDirty;
}

void UInventoryComponent::ClearDirty_Implementation()
{
    bSaveDirty = false;
}

void UInventoryComponent::OnSaveDataLoaded_Implementation()
{
    OnRep_Inventory();
}

void UInventoryComponent::MarkSaveDirty()
{
    bSaveDirty = true;
}
