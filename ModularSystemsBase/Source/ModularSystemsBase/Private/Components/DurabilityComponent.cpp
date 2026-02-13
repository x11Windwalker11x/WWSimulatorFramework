#include "Components/DurabilityComponent.h"
#include "GameplayTagAssetInterface.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/SaveSystem/SaveableRegistrySubsystem.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"

UDurabilityComponent::UDurabilityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    bTagsCached = false;
    // Default tag   
    CacheTags();
    DurabilityRequiredTag = CachedTag_HasDurability;

}

void UDurabilityComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeDurabilityFromOwner();

    if (USaveableRegistrySubsystem* Registry = USaveableRegistrySubsystem::Get(this))
    {
        Registry->RegisterSaveable(this);
    }
}

void UDurabilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UDurabilityComponent, CurrentDurability);
    DOREPLIFETIME(UDurabilityComponent, bIsBroken);
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UDurabilityComponent::InitializeDurabilityFromOwner()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Check if owner has durability tag via IGameplayTagAssetInterface
    if (const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Owner))
    {
        FGameplayTagContainer OwnerTags;
        TagInterface->GetOwnedGameplayTags(OwnerTags);
        bHasDurability = OwnerTags.HasTag(DurabilityRequiredTag);
    }
    else
    {
        // Fallback: assume no durability
        bHasDurability = false;
    }
}

// ============================================================================
// PUBLIC API
// ============================================================================

void UDurabilityComponent::ApplyDurabilityLoss(float Amount, AActor* Causer)
{
    if (!bHasDurability || bIsBroken) return;
    if (Amount <= 0.0f) return;

    if (!GetOwner()->HasAuthority())
    {
        Server_ApplyDurabilityLoss(Amount, Causer);
        return;
    }

    Internal_ApplyDurabilityLoss(Amount, Causer);
}

void UDurabilityComponent::ApplyUseDamage(AActor* Causer)
{
    ApplyDurabilityLoss(DurabilityLossPerUse, Causer);
}

void UDurabilityComponent::Repair(float Amount)
{
    if (!bHasDurability) return;
    if (Amount <= 0.0f) return;

    if (!GetOwner()->HasAuthority())
    {
        Server_Repair(Amount);
        return;
    }

    Internal_Repair(Amount);
}

void UDurabilityComponent::RepairFull()
{
    Repair(1.0f);
}

void UDurabilityComponent::SetDurability(float NewDurability)
{
    if (!GetOwner()->HasAuthority()) return;

    float OldDurability = CurrentDurability;
    CurrentDurability = FMath::Clamp(NewDurability, 0.0f, 1.0f);
    
    bool bWasBroken = bIsBroken;
    bIsBroken = CurrentDurability <= 0.0f;

    if (!FMath::IsNearlyEqual(OldDurability, CurrentDurability))
    {
        OnDurabilityChanged.Broadcast(OldDurability, CurrentDurability, nullptr);
    }

    if (bIsBroken && !bWasBroken)
    {
        OnItemBroken.Broadcast(nullptr);
    }
    else if (!bIsBroken && bWasBroken)
    {
        OnItemRepaired.Broadcast();
    }
}
// ============================================================================
// INTERFACE FUNCTIONS
// ============================================================================

void UDurabilityComponent::ApplyDurabilityLoss_Implementation(float Amount, AActor* Causer)
{
    if (!bHasDurability || bIsBroken) return;
    if (Amount <= 0.0f) return;

    if (!GetOwner()->HasAuthority())
    {
        Server_ApplyDurabilityLoss(Amount, Causer);
        return;
    }

    Internal_ApplyDurabilityLoss(Amount, Causer);
}

void UDurabilityComponent::ApplyUseDamage_Implementation(AActor* Causer)
{
    ApplyDurabilityLoss_Implementation(DurabilityLossPerUse, Causer);
}

void UDurabilityComponent::Repair_Implementation(float Amount)
{
    if (!bHasDurability) return;
    if (Amount <= 0.0f) return;

    if (!GetOwner()->HasAuthority())
    {
        Server_Repair(Amount);
        return;
    }

    Internal_Repair(Amount);
}

void UDurabilityComponent::RepairFull_Implementation()
{
    Repair_Implementation(1.0f);
}

void UDurabilityComponent::SetDurability_Implementation(float NewDurability)
{
    if (!GetOwner()->HasAuthority()) return;

    float OldDurability = CurrentDurability;
    CurrentDurability = FMath::Clamp(NewDurability, 0.0f, 1.0f);
    
    bool bWasBroken = bIsBroken;
    bIsBroken = CurrentDurability <= 0.0f;

    if (!FMath::IsNearlyEqual(OldDurability, CurrentDurability))
    {
        OnDurabilityChanged.Broadcast(OldDurability, CurrentDurability, nullptr);
    }

    if (bIsBroken && !bWasBroken)
    {
        OnItemBroken.Broadcast(nullptr);
    }
    else if (!bIsBroken && bWasBroken)
    {
        OnItemRepaired.Broadcast();
    }
}

float UDurabilityComponent::GetDurability_Implementation() const
{
    return CurrentDurability;
}

float UDurabilityComponent::GetDurabilityPercent_Implementation() const
{
    return CurrentDurability * 100.0f;
}

bool UDurabilityComponent::IsBroken_Implementation() const
{
    return bIsBroken;
}

bool UDurabilityComponent::IsLowDurability_Implementation() const
{
    return CurrentDurability <= LowDurabilityThreshold && !bIsBroken;
}

bool UDurabilityComponent::HasDurability_Implementation() const
{
    return bHasDurability;
}

bool UDurabilityComponent::IsUsable_Implementation() const
{
    return bHasDurability ? !bIsBroken : true;
}

UActorComponent* UDurabilityComponent::GetDurabilityComponentAsActorComponent_Implementation()
{
    return this;
}


// ============================================================================
// SERVER RPCs
// ============================================================================

bool UDurabilityComponent::Server_ApplyDurabilityLoss_Validate(float Amount, AActor* Causer)
{
    return Amount > 0.0f && Amount <= 1.0f && bHasDurability;
}

void UDurabilityComponent::Server_ApplyDurabilityLoss_Implementation(float Amount, AActor* Causer)
{
    Internal_ApplyDurabilityLoss(Amount, Causer);
}

bool UDurabilityComponent::Server_Repair_Validate(float Amount)
{
    return Amount > 0.0f && bHasDurability;
}

void UDurabilityComponent::Server_Repair_Implementation(float Amount)
{
    Internal_Repair(Amount);
}

// ============================================================================
// INTERNALS
// ============================================================================

void UDurabilityComponent::Internal_ApplyDurabilityLoss(float Amount, AActor* Causer)
{
    if (bIsBroken) return;

    float OldDurability = CurrentDurability;
    CurrentDurability = FMath::Max(0.0f, CurrentDurability - Amount);

    MarkSaveDirty();
    OnDurabilityChanged.Broadcast(OldDurability, CurrentDurability, Causer);

    CheckBrokenState(Causer);
}

void UDurabilityComponent::Internal_Repair(float Amount)
{
    float OldDurability = CurrentDurability;
    bool bWasBroken = bIsBroken;

    CurrentDurability = FMath::Min(1.0f, CurrentDurability + Amount);
    bIsBroken = false;
    MarkSaveDirty();

    if (!FMath::IsNearlyEqual(OldDurability, CurrentDurability))
    {
        OnDurabilityChanged.Broadcast(OldDurability, CurrentDurability, nullptr);
    }

    if (bWasBroken)
    {
        OnItemRepaired.Broadcast();
    }
}

void UDurabilityComponent::CheckBrokenState(AActor* Causer)
{
    if (CurrentDurability <= 0.0f && !bIsBroken)
    {
        bIsBroken = true;
        OnItemBroken.Broadcast(Causer);
    }
}

// ============================================================================
// ONREPS
// ============================================================================

void UDurabilityComponent::OnRep_CurrentDurability()
{
    // Visual update on clients if needed
}

void UDurabilityComponent::OnRep_IsBroken()
{
    if (bIsBroken)
    {
        OnItemBroken.Broadcast(nullptr);
    }
    else
    {
        OnItemRepaired.Broadcast();
    }
}

void UDurabilityComponent::CacheTags()
{
    if (bTagsCached) return;

    CachedTag_HasDurability = FGameplayTag::RequestGameplayTag(TEXT("Item.Property.HasDurability"), false);
    bTagsCached = true;
}

void UDurabilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (USaveableRegistrySubsystem* Registry = USaveableRegistrySubsystem::Get(this))
    {
        Registry->UnregisterSaveable(GetSaveID_Implementation());
    }
    Super::EndPlay(EndPlayReason);
}

// ============================================================================
// SAVE SYSTEM (ISaveableInterface)
// ============================================================================

FString UDurabilityComponent::GetSaveID_Implementation() const
{
    if (AActor* Owner = GetOwner())
    {
        return FString::Printf(TEXT("%s.%s"), *Owner->GetPathName(), *GetClass()->GetName());
    }
    return FString();
}

int32 UDurabilityComponent::GetSavePriority_Implementation() const
{
    return 110;
}

FGameplayTag UDurabilityComponent::GetSaveType_Implementation() const
{
    return FWWTagLibrary::Save_Category_Component();
}

bool UDurabilityComponent::SaveState_Implementation(FSaveRecord& OutRecord)
{
    TArray<uint8> BinaryData;
    FMemoryWriter MemoryWriter(BinaryData, true);
    FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
    Ar.ArIsSaveGame = true;
    Ar.ArNoDelta = false;

    const_cast<UDurabilityComponent*>(this)->Serialize(Ar);

    OutRecord.RecordID = FName(*GetSaveID_Implementation());
    OutRecord.RecordType = FWWTagLibrary::Save_Category_Component();
    OutRecord.BinaryData = MoveTemp(BinaryData);
    OutRecord.Timestamp = FDateTime::Now();
    OutRecord.Priority = GetSavePriority_Implementation();

    return true;
}

bool UDurabilityComponent::LoadState_Implementation(const FSaveRecord& InRecord)
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

bool UDurabilityComponent::IsDirty_Implementation() const
{
    return bSaveDirty;
}

void UDurabilityComponent::ClearDirty_Implementation()
{
    bSaveDirty = false;
}

void UDurabilityComponent::OnSaveDataLoaded_Implementation()
{
    // Refresh visual state after load
    OnRep_CurrentDurability();
    OnRep_IsBroken();
}

void UDurabilityComponent::MarkSaveDirty()
{
    bSaveDirty = true;
}
