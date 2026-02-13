// InventoryComponent.h
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
//Inventory Data Contains item, inventory, slot data
#include "Lib/Data/ModularInventorySystem/InventoryData.h"
//TODO: engine gets it wrong it should have been ModularInventorySystem instead.
#include "Windwalker_Productions_SharedDefaults.h"
#include "ModularInventorySystem/Public/Data/InventoryPrediction.h"
#include "Interfaces/ModularSaveGameSystem/SaveableInterface.h"

#include "InventoryComponent.generated.h"

// Forward declarations
class APickupActor_Master;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, FGameplayTag, InInventoryTypeTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotChanged, FGameplayTag, InInventoryTypeTag, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemAdded, FName, ItemID, int32, Quantity, FGuid, InstanceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemRemoved, FName, ItemID, int32, Quantity, FGuid, InstanceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemDropped, FInventorySlot,DroppedSlot, int32, Quantity, FTransform, Transform);
/** Fired when server rejects a predicted action - UI can show feedback */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPredictionRejected, int32, PredictionID, FString, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemConsumed, FName, ItemID, int32, Quantity);


/**
 * Inventory Component
 * Manages player inventory, equipment, and item operations
 * 
 * AAA Professional Standard:
 * - All game logic queries centralized here
 * - UI/widgets delegate to this component for all item checks
 * - Clean separation between data (FInventorySlot) and logic (UInventoryComponent)
 * - Server authoritative with client RPC routing
 */

/*
 *This could have inherit from DataTableOverwritableComponent
 *But since it won't use data table to config itself at the moment,
 *it is fine for it to inherit from an actor component.
 */
UCLASS(ClassGroup=(Components), meta=(BlueprintSpawnableComponent))
class MODULARINVENTORYSYSTEM_API UInventoryComponent : public UActorComponent, public ISaveableInterface
{
    GENERATED_BODY()



protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    UInventoryComponent();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================
    

    /** Maximum inventory slots */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    int32 MaxInventorySlots = 200;
    
    /** Item data table reference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    TObjectPtr<UDataTable> ItemDataTable;
    /*Inventory Type to determine which ops we can do on this component*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    FGameplayTag InventoryTypeTag = FWWTagLibrary::Inventory_Type_PlayerInventory();
    
    
    // ============================================================================
    // CORE ITEM OPERATIONS
    // ============================================================================

    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool ToggleInventory();
    
    /** Add item to inventory */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(FName ItemID, int32 Quantity, float Quality = 1.0f, float Durability = 1.0f, bool bIsStolen = false);
    
    /** Remove item by instance ID */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItemByInstance(FGuid InstanceID, int32 Quantity = 1);
    
    /** Move item between slots */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool MoveItem(FGameplayTag SourceType, int32 SourceSlot, FGameplayTag DestType, int32 DestSlot);
    
    /** Split stack */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SplitStack(FGameplayTag SourceType, int32 SourceSlot, int32 SplitAmount, FGameplayTag DestType, int32 DestSlot);
    
    /** Drop item from inventory */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool DropItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex, int32 Quantity = -1);
    
    /** Use item */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex);
    
    /** Equip item */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool EquipItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex);
    
    /** Unequip item */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UnequipItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex);
    
    /** Toggle favorite status */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool ToggleFavorite(FGameplayTag InInventoryTypeTag, int32 SlotIndex);

    UFUNCTION(BlueprintPure, Category = "Inventory|Preview")
    TArray<FStatComparisonEntry> BuildStatComparison(const FInventorySlot& CurrentItem, const FInventorySlot& CompareItem) const;

    // ============================================================================
    // SERVER RPCs (Client requests → Server validates)
    // ============================================================================

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_AddItem(FName ItemID, int32 Quantity, float Quality, float Durability, bool bIsStolen);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RemoveItem(FGuid InstanceID, int32 Quantity);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_DropItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_MoveItem(FGameplayTag SourceType, int32 SourceSlot, 
                         FGameplayTag DestType, int32 DestSlot, int32 PredictionID);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SplitStack(FGameplayTag SourceType, int32 SourceSlot, 
                           int32 SplitAmount, FGameplayTag DestType, int32 DestSlot, 
                           int32 PredictionID);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UseItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_EquipItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UnequipItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ToggleFavorite(FGameplayTag InInventoryTypeTag, int32 SlotIndex, int32 PredictionID);


    UFUNCTION(Server, Reliable, WithValidation)
    void Server_CombineSimilarItems(FGameplayTag InInventoryTypeTag, int32 SourceSlotIndex);

    /*Server telling client whether op is succeeded*/
    UFUNCTION(Client, Reliable)
    void Client_ConfirmPrediction(int32 PredictionID, bool bSuccess, const FString& FailReason);

    UFUNCTION(Server, Reliable)
    void Server_TryConsumeItem(const FName& ItemID, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void Server_SetSlot(int32 Index, FInventorySlot& Slot);
    
    

    // ============================================================================
    // DATA ACCESS
    // ============================================================================

    /*SetSlot*/
    UFUNCTION(BlueprintCallable, Category = "Inventory | Data")
    bool SetSlot(int32 Index, FInventorySlot& Slot);
    /* Get PlayerInventoryData*/
    UFUNCTION(BlueprintPure, Catgeory = "Inventory | Data")
    const TArray<FInventorySlot>& GetInventory(FGameplayTag InInInventoryTypeTagTag = FWWTagLibrary::Inventory_Type_PlayerInventory()) const;
    /** Get slot data by inventory type and index */
    UFUNCTION(BlueprintPure, Category = "Inventory|Data")
    bool GetSlot(FGameplayTag InInventoryTypeTag, int32 SlotIndex, FInventorySlot& OutSlot) const;
    
    /** Get item data from data table */
    UFUNCTION(BlueprintPure, Category = "Inventory|Data")
    bool GetItemData(FName ItemID, FItemData& OutItemData) const;
    
    /** Get inventory array by type */
    TArray<FInventorySlot>* GetInventoryArray(FGameplayTag InInventoryTypeTag);
    const TArray<FInventorySlot>* GetInventoryArray(FGameplayTag InInventoryTypeTag) const;
    
    /** Get item count */
    UFUNCTION(BlueprintPure, Category = "Inventory|Data")
    int32 GetItemCountPure(FName ItemID) const;
    
    /** Check if has item */
    UFUNCTION(BlueprintPure, Category = "Inventory|Data")
    bool HasItem(FName ItemID, int32 MinQuantity = 1) const;
    
    /** Find empty slot */
    UFUNCTION(BlueprintPure, Category = "Inventory|Data")
    int32 FindEmptySlot(FGameplayTag InInventoryTypeTag) const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Data")
    int32 FindSlotIndexByInstanceID(FGuid InstanceID) const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Preview")
    FItemPreviewData GetItemPreviewData(int32 SlotIndex) const;
    


    
    
    // ============================================================================
    // SLOT QUERY HELPERS (AAA Professional Standard)
    // ============================================================================
    
    UFUNCTION(BlueprintPure, Category = "Inventory|Query")
    bool IsSlotEquippable(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory|Query")
    bool IsSlotEquipped(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory|Query")
    bool IsSlotUsable(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory|Query")
    bool IsSlotDroppable(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory|Query")
    bool IsSlotAttachment(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory|Query")
    bool IsSlotCombinable(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory|Query")
    bool CanAttachToSlot(FGameplayTag AttachmentInInventoryTypeTag, int32 AttachmentSlotIndex,
                         FGameplayTag TargetInInventoryTypeTag, int32 TargetSlotIndex) const;
    
    // ============================================================================
    // COMBINE SYSTEM FUNCTIONS
    // ============================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|Combine")
    void CombineSimilarItems(FGameplayTag InInventoryTypeTag, int32 SourceSlotIndex);
    
    UFUNCTION(BlueprintPure, Category = "Inventory|Combine")
    bool CanCombineItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory|Combine")
    int32 GetCombinableItemCount(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const;

    // ============================================================================
    // INTERFACE IMPLEMENTATIONS
    // ============================================================================ functions
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    UActorComponent* GetInventoryComponent();
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    int32 GetItemCount(FName ItemID) const;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
    bool TryConsumeItem(FName ItemID, int32 Quantity);
    virtual bool HasItem_Implementation(FName ItemID, int32 MinQuantity) override;

    // NEW
    virtual bool AddItem_Implementation(FName ItemID, int32 Quantity, float Quality, float Durability, bool bIsStolen) override;
    virtual bool RemoveItemByInstance_Implementation(FGuid InstanceID, int32 Quantity) override;
    virtual bool DropItem_Implementation(FGameplayTag InventoryType, int32 SlotIndex, int32 Quantity) override;
    virtual bool UseItem_Implementation(FGameplayTag InventoryType, int32 SlotIndex) override;
    virtual bool EquipItem_Implementation(FGameplayTag InventoryType, int32 SlotIndex) override;
    virtual bool UnequipItem_Implementation(FGameplayTag InventoryType, int32 SlotIndex) override;
    virtual int32 FindEmptySlot_Implementation(FGameplayTag InventoryType) const override;

    // ============================================================================
    // EVENTS
    // ============================================================================
    
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnInventoryChanged OnInventoryChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnInventorySlotChanged OnInventorySlotChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemAdded OnItemAdded;
    
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemRemoved OnItemRemoved;
    
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemDropped OnItemDropped;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnPredictionRejected OnPredictionRejected;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemConsumed OnItemConsumed;


protected:

    // ============================================================================
    // COMPARISION FEATURE
    // ============================================================================

    void AddComparisonEntry(
    TArray<FStatComparisonEntry>& Entries,
    const FName& StatName,
    const FString& DisplayName,
    float CurrentValue,
    float CompareValue,
    bool bHigherIsBetter,
    const FString& UnitSuffix) const;



    // ============================================================================
    // INVENTORY ARRAYS
    // ============================================================================
    
    /** Player inventory slots */
    /** It is determined by the tag this component has*/
    UPROPERTY(SaveGame, ReplicatedUsing=OnRep_Inventory, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> Inventory;
    
    // ============================================================================
    // REPLICATION
    // ============================================================================
    
    UFUNCTION()
    void OnRep_Inventory();


    // ============================================================================
    // CLIENT PREDICTION
    // ============================================================================
    /* Client Prediction:
     * Normally request can wait up to .5 ms depending on internal engine speed
     * and the current connection speed
     * What prediction does is to update ui immediately and then notify server
     * if the server rejects due to desync or some other kind of issue,
     * We will rewind the action to its previous state. 
     */
    
    /** Counter for generating unique prediction IDs */
    int32 PredictionIDCounter = 0;

    /** Pending predictions awaiting server confirmation */
    TMap<int32, FInventoryPrediction> PendingPredictions;

    /** Max time to wait for server confirmation before auto-cleanup (seconds) */
    static constexpr float PredictionTimeout = 5.0f;

    /** Generate next prediction ID */
    int32 GeneratePredictionID();

    /** Cache slot state before prediction */
    void CachePrediction(int32 PredictionID, const FInventoryPrediction& Prediction);

    /** Rollback a rejected prediction */
    void RollbackPrediction(int32 PredictionID);

    /** Clean up confirmed/expired predictions */
    void CleanupPrediction(int32 PredictionID);

    /** Apply predicted move locally (client-side) */
    bool Predicted_MoveItem(FGameplayTag SourceType, int32 SourceSlot, 
                            FGameplayTag DestType, int32 DestSlot, int32& OutPredictionID);

    /** Apply predicted split locally (client-side) */
    bool Predicted_SplitStack(FGameplayTag SourceType, int32 SourceSlot, 
                              int32 SplitAmount, FGameplayTag DestType, int32 DestSlot, 
                              int32& OutPredictionID);

    // ----------------------------------------------------------------------------
    //  Client Prediction: VALIDATION DECLARATIONS 
    // ----------------------------------------------------------------------------

    /** Apply predicted favorite toggle locally (client-side) */
    bool Predicted_ToggleFavorite(FGameplayTag InInventoryTypeTag, int32 SlotIndex, 
                                   int32& OutPredictionID);

    bool Server_MoveItem_Validate(FGameplayTag SourceType, int32 SourceSlot, 
                               FGameplayTag DestType, int32 DestSlot, int32 PredictionID);
    bool Server_SplitStack_Validate(FGameplayTag SourceType, int32 SourceSlot, 
                                     int32 SplitAmount, FGameplayTag DestType, int32 DestSlot, 
                                     int32 PredictionID);
    bool Server_ToggleFavorite_Validate(FGameplayTag InInventoryTypeTag, int32 SlotIndex, int32 PredictionID);

    
    
    // ============================================================================
    // INTERNAL HELPERS
    // ============================================================================
    
    bool ValidateSlotIndex(FGameplayTag InInventoryTypeTag, int32 SlotIndex) const;
    bool TryAutoStack(FName ItemID, int32& Quantity, float Quality);
    bool TryConsumeItem(const FName& ItemID, int32& Quantity, int32 SlotIndex);
    void BroadcastSlotChange(FGameplayTag InInventoryTypeTag, int32 SlotIndex);

    // ============================================================================
    // INTERNAL SERVER LOGIC (called after RPC validation)
    // ============================================================================

    bool Internal_AddItem(FName ItemID, int32 Quantity, float Quality, float Durability, bool bIsStolen);
    bool Internal_RemoveItemByInstance(FGuid InstanceID, int32 Quantity);
    bool Internal_MoveItem(FGameplayTag SourceType, int32 SourceSlot, FGameplayTag DestType, int32 DestSlot);
    bool Internal_SplitStack(FGameplayTag SourceType, int32 SourceSlot, int32 SplitAmount, FGameplayTag DestType, int32 DestSlot);
    bool Internal_DropItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex, int32 Quantity);
    bool Internal_UseItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex);
    bool Internal_EquipItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex);
    bool Internal_UnequipItem(FGameplayTag InInventoryTypeTag, int32 SlotIndex);
    bool Internal_ToggleFavorite(FGameplayTag InInventoryTypeTag, int32 SlotIndex);
    void Internal_CombineSimilarItems(FGameplayTag InInventoryTypeTag, int32 SourceSlotIndex);
    void Internal_ConsumeItem(const FName& ItemID, int32 Quantity);
    bool Internal_SetSlot(int32 Index, FInventorySlot& Slot);

    // ============================================================================
    // SAVE SYSTEM (ISaveableInterface)
    // ============================================================================

    virtual FString GetSaveID_Implementation() const override;
    virtual int32 GetSavePriority_Implementation() const override;
    virtual FGameplayTag GetSaveType_Implementation() const override;
    virtual bool SaveState_Implementation(FSaveRecord& OutRecord) override;
    virtual bool LoadState_Implementation(const FSaveRecord& InRecord) override;
    virtual bool IsDirty_Implementation() const override;
    virtual void ClearDirty_Implementation() override;
    virtual void OnSaveDataLoaded_Implementation() override;

    bool bSaveDirty = false;
    void MarkSaveDirty();
};