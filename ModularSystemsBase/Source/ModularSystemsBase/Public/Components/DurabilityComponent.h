#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SimulatorFramework/DurabilityInterface.h"
#include "Interfaces/ModularSaveGameSystem/SaveableInterface.h"
#include "DurabilityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDurabilityChanged, float, OldDurability, float, NewDurability, AActor*, Causer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemBroken, AActor*, Causer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemRepaired);

/**
 * Manages durability for world actors (tools, stations, spawned items).
 * Tag-driven: Only active if owner has Item.Property.HasDurability tag.
 * Syncs with FInventorySlot.Durability when item re-enters inventory.
 */
UCLASS(ClassGroup=(SimulatorFramework), meta=(BlueprintSpawnableComponent))
class MODULARSYSTEMSBASE_API UDurabilityComponent : public UActorComponent, public IDurabilityInterface, public ISaveableInterface
{
    GENERATED_BODY()

public:
    UDurabilityComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ============================================================================
    // CONFIGURATION
    // ============================================================================

    /** Tag that determines if durability is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability|Config")
    FGameplayTag DurabilityRequiredTag;

    /** Durability loss per use (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DurabilityLossPerUse = 0.01f;

    /** Below this threshold, item is considered "low durability" */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LowDurabilityThreshold = 0.25f;

    // ============================================================================
    // PUBLIC API
    // ============================================================================

    /** Apply durability damage */
    UFUNCTION(BlueprintCallable, Category = "Durability")
    void ApplyDurabilityLoss(float Amount, AActor* Causer = nullptr);

    /** Apply single use worth of damage */
    UFUNCTION(BlueprintCallable, Category = "Durability")
    void ApplyUseDamage(AActor* Causer = nullptr);

    /** Repair durability */
    UFUNCTION(BlueprintCallable, Category = "Durability")
    void Repair(float Amount);

    /** Full repair */
    UFUNCTION(BlueprintCallable, Category = "Durability")
    void RepairFull();

    /** Set durability directly (for sync from inventory) */
    UFUNCTION(BlueprintCallable, Category = "Durability")
    void SetDurability(float NewDurability);

    // ============================================================================
    // QUERIES
    // ============================================================================

    UFUNCTION(BlueprintPure, Category = "Durability")
    FORCEINLINE float GetDurability() const { return CurrentDurability; }

    UFUNCTION(BlueprintPure, Category = "Durability")
    FORCEINLINE float GetDurabilityPercent() const { return CurrentDurability * 100.0f; }

    UFUNCTION(BlueprintPure, Category = "Durability")
    FORCEINLINE bool IsBroken() const { return bIsBroken; }

    UFUNCTION(BlueprintPure, Category = "Durability")
    FORCEINLINE bool IsLowDurability() const { return CurrentDurability <= LowDurabilityThreshold && !bIsBroken; }

    UFUNCTION(BlueprintPure, Category = "Durability")
    FORCEINLINE bool HasDurability() const { return bHasDurability; }

    UFUNCTION(BlueprintPure, Category = "Durability")
    FORCEINLINE bool IsUsable() const { return bHasDurability ? !bIsBroken : true; }


    // ============================================================================
    // INTERFACE IMPLEMENTATIONS
    // ============================================================================

    virtual void ApplyDurabilityLoss_Implementation(float Amount, AActor* Causer) override;
    virtual void ApplyUseDamage_Implementation(AActor* Causer) override;
    virtual void Repair_Implementation(float Amount) override;
    virtual void RepairFull_Implementation() override;
    virtual void SetDurability_Implementation(float NewDurability) override;

    virtual float GetDurability_Implementation() const override;
    virtual float GetDurabilityPercent_Implementation() const override;
    virtual bool IsBroken_Implementation() const override;
    virtual bool IsLowDurability_Implementation() const override;
    virtual bool HasDurability_Implementation() const override;
    virtual bool IsUsable_Implementation() const override;

    virtual UActorComponent* GetDurabilityComponentAsActorComponent_Implementation() override;

    // ============================================================================
    // EVENTS
    // ============================================================================

    UPROPERTY(BlueprintAssignable, Category = "Durability|Events")
    FOnDurabilityChanged OnDurabilityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Durability|Events")
    FOnItemBroken OnItemBroken;

    UPROPERTY(BlueprintAssignable, Category = "Durability|Events")
    FOnItemRepaired OnItemRepaired;

    // ============================================================================
    // SERVER RPCs
    // ============================================================================

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ApplyDurabilityLoss(float Amount, AActor* Causer);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Repair(float Amount);

protected:
    // ============================================================================
    // OPTIMISATION - CACHED TAGS
    // ============================================================================

    static FGameplayTag CachedTag_HasDurability;
    static bool bTagsCached;
    static void CacheTags();

    // ============================================================================
    // REPLICATED STATE
    // ============================================================================

    UPROPERTY(SaveGame, ReplicatedUsing = OnRep_CurrentDurability)
    float CurrentDurability = 1.0f;

    UPROPERTY(SaveGame, ReplicatedUsing = OnRep_IsBroken)
    bool bIsBroken = false;

    UFUNCTION()
    void OnRep_CurrentDurability();

    UFUNCTION()
    void OnRep_IsBroken();

    // ============================================================================
    // LOCAL STATE
    // ============================================================================

    /** Cached: Does owner have durability tag? */
    bool bHasDurability = false;

    /** Cached durability tag for checks */
    FGameplayTag CachedDurabilityTag;

    // ============================================================================
    // INTERNALS
    // ============================================================================

    void Internal_ApplyDurabilityLoss(float Amount, AActor* Causer);
    void Internal_Repair(float Amount);
    void CheckBrokenState(AActor* Causer);
    void InitializeDurabilityFromOwner();

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