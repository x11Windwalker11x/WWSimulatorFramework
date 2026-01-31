// QuickSlotsComponent.h
// Location: ModularInventorySystem/Public/Components/QuickSlotsComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
#include "QuickSlotsComponent.generated.h"

class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuickSlotChanged, int32, SlotIndex, const FInventorySlot&, NewSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuickSlotUsed, int32, SlotIndex);

/**
 * Manages quickslots for fast item access.
 * Requires UInventoryComponent on same actor.
 * 
 * Items in quickslots are REFERENCES to inventory items,
 * not copies. Using a quickslot item affects inventory.
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class MODULARINVENTORYSYSTEM_API UQuickSlotsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuickSlotsComponent();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickSlots|Config")
    int32 MaxQuickSlots = 10;

    // ============================================================================
    // QUICK SLOT OPERATIONS
    // ============================================================================

    /** Assign inventory item to quickslot */
    UFUNCTION(BlueprintCallable, Category = "QuickSlots")
    bool AssignToQuickSlot(int32 InventoryIndex, int32 QuickSlotIndex);

    /** Clear quickslot */
    UFUNCTION(BlueprintCallable, Category = "QuickSlots")
    bool ClearQuickSlot(int32 QuickSlotIndex);

    /** Use item in quickslot (consume, equip, etc.) */
    UFUNCTION(BlueprintCallable, Category = "QuickSlots")
    bool UseQuickSlot(int32 QuickSlotIndex);

    /** Swap two quickslots */
    UFUNCTION(BlueprintCallable, Category = "QuickSlots")
    bool SwapQuickSlots(int32 IndexA, int32 IndexB);

    // ============================================================================
    // QUERIES
    // ============================================================================

    UFUNCTION(BlueprintPure, Category = "QuickSlots")
    const FInventorySlot& GetQuickSlot(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "QuickSlots")
    bool IsQuickSlotEmpty(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "QuickSlots")
    bool CanAssignToQuickSlot(const FInventorySlot& Slot) const;

    UFUNCTION(BlueprintPure, Category = "QuickSlots")
    int32 FindQuickSlotByInstanceID(FGuid InstanceID) const;

    UFUNCTION(BlueprintPure, Category = "QuickSlots")
    const TArray<FInventorySlot>& GetAllQuickSlots() const { return QuickSlots; }

    // ============================================================================
    // EVENTS
    // ============================================================================

    UPROPERTY(BlueprintAssignable, Category = "QuickSlots|Events")
    FOnQuickSlotChanged OnQuickSlotChanged;

    UPROPERTY(BlueprintAssignable, Category = "QuickSlots|Events")
    FOnQuickSlotUsed OnQuickSlotUsed;

    // ============================================================================
    // SERVER RPCs
    // ============================================================================

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_AssignToQuickSlot(int32 InventoryIndex, int32 QuickSlotIndex);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ClearQuickSlot(int32 QuickSlotIndex);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UseQuickSlot(int32 QuickSlotIndex);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SwapQuickSlots(int32 IndexA, int32 IndexB);

protected:
    // ============================================================================
    // DATA
    // ============================================================================

    UPROPERTY(ReplicatedUsing=OnRep_QuickSlots)
    TArray<FInventorySlot> QuickSlots;

    UPROPERTY()
    TWeakObjectPtr<UInventoryComponent> InventoryComponent;

    // ============================================================================
    // REPLICATION
    // ============================================================================

    UFUNCTION()
    void OnRep_QuickSlots();

    // ============================================================================
    // INTERNALS
    // ============================================================================

    bool Internal_AssignToQuickSlot(int32 InventoryIndex, int32 QuickSlotIndex);
    bool Internal_ClearQuickSlot(int32 QuickSlotIndex);
    bool Internal_UseQuickSlot(int32 QuickSlotIndex);
    bool Internal_SwapQuickSlots(int32 IndexA, int32 IndexB);

    UInventoryComponent* GetInventoryComponent();
};