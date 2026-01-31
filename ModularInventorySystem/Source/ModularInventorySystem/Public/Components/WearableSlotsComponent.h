// WearableSlotsComponent.h
/*
 *This is primarily designed for custom wearables:
 *Accessories: arms: bracelets, gauntlets etc..
 *top:neckalaces, talismans etc
 * ear: earrigns
 * rigns: one in each hand
 * underwear:undressable
 * top, bottom, shoes, hats or helmets, gloves
 * armor layer: chest, botttoms shoes, helmets..
 * only wearables can be equpped into the slots
 * wearables can also go int o quickslots for trading purposses mainly
 * requieres the owner to have inventory component.
 * inherits from inventory component, slot is the same FInventoryslot
 * data from InventoryData.h 
 */
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "WearableSlotsComponent.generated.h"

class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWearableSlotChanged, FGameplayTag, SlotType, const FInventorySlot&, NewSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWearableEquipped, FGameplayTag, SlotType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWearableUnequipped, FGameplayTag, SlotType);

/**
 * Manages wearable equipment slots (armor, clothing, accessories).
 * Requires UInventoryComponent on same actor.
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class MODULARINVENTORYSYSTEM_API UWearableSlotsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWearableSlotsComponent();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================================================
    // WEARABLE OPERATIONS
    // ============================================================================

    /** Equip item from inventory to wearable slot */
    UFUNCTION(BlueprintCallable, Category = "Wearables")
    bool EquipToSlot(int32 InventoryIndex, FGameplayTag SlotType);

    /** Unequip item from wearable slot back to inventory */
    UFUNCTION(BlueprintCallable, Category = "Wearables")
    bool UnequipFromSlot(FGameplayTag SlotType);

    /** Swap wearable with inventory item */
    UFUNCTION(BlueprintCallable, Category = "Wearables")
    bool SwapWithInventory(FGameplayTag SlotType, int32 InventoryIndex);

    // ============================================================================
    // QUERIES
    // ============================================================================

    UFUNCTION(BlueprintPure, Category = "Wearables")
    const FInventorySlot& GetWearableSlot(FGameplayTag SlotType) const;

    UFUNCTION(BlueprintPure, Category = "Wearables")
    bool IsSlotEmpty(FGameplayTag SlotType) const;

    UFUNCTION(BlueprintPure, Category = "Wearables")
    bool CanEquipToSlot(const FInventorySlot& Slot, FGameplayTag SlotType) const;

    UFUNCTION(BlueprintPure, Category = "Wearables")
    FGameplayTag FindSlotByInstanceID(FGuid InstanceID) const;

    UFUNCTION(BlueprintPure, Category = "Wearables")
    TArray<FGameplayTag> GetAllSlotTypes() const;

    UFUNCTION(BlueprintPure, Category = "Wearables")
    const TMap<FGameplayTag, FInventorySlot>& GetAllWearableSlots() const { return WearableSlots; }

    /** Get equipped item for comparison UI */
    UFUNCTION(BlueprintPure, Category = "Wearables")
    FInventorySlot GetEquippedComparisonItem(FGameplayTag SlotType) const;

    // ============================================================================
    // EVENTS
    // ============================================================================

    UPROPERTY(BlueprintAssignable, Category = "Wearables|Events")
    FOnWearableSlotChanged OnWearableSlotChanged;

    UPROPERTY(BlueprintAssignable, Category = "Wearables|Events")
    FOnWearableEquipped OnWearableEquipped;

    UPROPERTY(BlueprintAssignable, Category = "Wearables|Events")
    FOnWearableUnequipped OnWearableUnequipped;

    // ============================================================================
    // SERVER RPCs
    // ============================================================================

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_EquipToSlot(int32 InventoryIndex, FGameplayTag SlotType);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UnequipFromSlot(FGameplayTag SlotType);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SwapWithInventory(FGameplayTag SlotType, int32 InventoryIndex);

protected:
    // ============================================================================
    // DATA
    // ============================================================================

    UPROPERTY(ReplicatedUsing=OnRep_WearableSlots)
    TMap<FGameplayTag, FInventorySlot> WearableSlots;

    UPROPERTY()
    TWeakObjectPtr<UInventoryComponent> InventoryComponent;

    // Cached tags for validation
    UPROPERTY()
    TArray<FGameplayTag> AllSlotTypes;

    UPROPERTY()
    FGameplayTag Tag_Item_Behavior_Wear;

    // ============================================================================
    // REPLICATION
    // ============================================================================

    UFUNCTION()
    void OnRep_WearableSlots();

    // ============================================================================
    // INTERNALS
    // ============================================================================

    void InitializeSlots();
    void CacheTags();
    FGameplayTag ConvertSlotTagToItemTag(FGameplayTag SlotTag) const;

    bool Internal_EquipToSlot(int32 InventoryIndex, FGameplayTag SlotType);
    bool Internal_UnequipFromSlot(FGameplayTag SlotType);
    bool Internal_SwapWithInventory(FGameplayTag SlotType, int32 InventoryIndex);

    UInventoryComponent* GetInventoryComponent();
};