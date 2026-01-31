// QuickSlotsComponent.cpp
// Location: ModularInventorySystem/Private/Components/QuickSlotsComponent.cpp

#include "Components/QuickSlotsComponent.h"
#include "Components/InventoryComponent.h"
#include "Interfaces/ModularInventorySystem/InventoryInterface.h"
#include "Logging/InteractableInventoryLogging.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuickSlots, Log, All);

UQuickSlotsComponent::UQuickSlotsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UQuickSlotsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize slots
    QuickSlots.SetNum(MaxQuickSlots);
    
    // Cache inventory component
    GetInventoryComponent();
}

void UQuickSlotsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UQuickSlotsComponent, QuickSlots);
}

// ============================================================================
// HELPERS
// ============================================================================

UInventoryComponent* UQuickSlotsComponent::GetInventoryComponent()
{
    UActorComponent* AC_InventoryComponent  = nullptr;
    AActor* Owner = this->GetOwner();
    if (Owner && Owner->GetClass()->ImplementsInterface(UInventoryInterface::StaticClass()))
    {
        AC_InventoryComponent = IInventoryInterface::Execute_GetInventoryComponent(Owner);
    }
    
    if (!AC_InventoryComponent && !InventoryComponent.IsValid())
    {
        UE_LOG(LogDebugSystem, Warning, TEXT("WearableSlotsComponent requires InventoryComponent on same actor"));
    }
    InventoryComponent = Cast<UInventoryComponent>(AC_InventoryComponent);
    
    return InventoryComponent.Get();
}

// ============================================================================
// QUERIES
// ============================================================================

const FInventorySlot& UQuickSlotsComponent::GetQuickSlot(int32 Index) const
{
    static FInventorySlot EmptySlot;
    if (!QuickSlots.IsValidIndex(Index)) return EmptySlot;
    return QuickSlots[Index];
}

bool UQuickSlotsComponent::IsQuickSlotEmpty(int32 Index) const
{
    if (!QuickSlots.IsValidIndex(Index)) return true;
    return QuickSlots[Index].IsEmpty();
}

bool UQuickSlotsComponent::CanAssignToQuickSlot(const FInventorySlot& Slot) const
{
    if (Slot.IsEmpty()) return false;
    
    // Check for equippable or wearable tags
    // Uses existing behavior/inventory tags
    return Slot.ItemTags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Item.Behavior.Equippable"))) ||
           Slot.ItemTags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Item.Behavior.Usable"))) ||
           Slot.ItemTags.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("Item.Behavior.Consumable"))) ||
           Slot.ItemTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Item.Wearable")));
}

int32 UQuickSlotsComponent::FindQuickSlotByInstanceID(FGuid InstanceID) const
{
    for (int32 i = 0; i < QuickSlots.Num(); ++i)
    {
        if (QuickSlots[i].InstanceID == InstanceID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

// ============================================================================
// OPERATIONS
// ============================================================================

bool UQuickSlotsComponent::AssignToQuickSlot(int32 InventoryIndex, int32 QuickSlotIndex)
{
    if (!QuickSlots.IsValidIndex(QuickSlotIndex)) return false;
    
    if (!GetOwner()->HasAuthority())
    {
        Server_AssignToQuickSlot(InventoryIndex, QuickSlotIndex);
        return true;
    }
    
    return Internal_AssignToQuickSlot(InventoryIndex, QuickSlotIndex);
}

bool UQuickSlotsComponent::Internal_AssignToQuickSlot(int32 InventoryIndex, int32 QuickSlotIndex)
{
    UInventoryComponent* InvComp = GetInventoryComponent();
    if (!InvComp) return false;
    
    const TArray<FInventorySlot>& Inventory = InvComp->GetInventory();
    if (!Inventory.IsValidIndex(InventoryIndex)) return false;
    
    const FInventorySlot& SourceSlot = Inventory[InventoryIndex];
    
    if (!CanAssignToQuickSlot(SourceSlot))
    {
        UE_LOG(LogQuickSlots, Warning, TEXT("Item cannot be assigned to quickslot"));
        return false;
    }
    
    // Store reference (copy slot data, keeps same InstanceID)
    FInventorySlot OldSlot = QuickSlots[QuickSlotIndex];
    QuickSlots[QuickSlotIndex] = SourceSlot;
    QuickSlots[QuickSlotIndex].bIsEquipped = true;
    QuickSlots[QuickSlotIndex].SlotType = FGameplayTag::RequestGameplayTag(
        FName(*FString::Printf(TEXT("Slot.QuickSlot.%d"), QuickSlotIndex + 1)));
    
    OnQuickSlotChanged.Broadcast(QuickSlotIndex, QuickSlots[QuickSlotIndex]);
    
    return true;
}

bool UQuickSlotsComponent::ClearQuickSlot(int32 QuickSlotIndex)
{
    if (!QuickSlots.IsValidIndex(QuickSlotIndex)) return false;
    
    if (!GetOwner()->HasAuthority())
    {
        Server_ClearQuickSlot(QuickSlotIndex);
        return true;
    }
    
    return Internal_ClearQuickSlot(QuickSlotIndex);
}

bool UQuickSlotsComponent::Internal_ClearQuickSlot(int32 QuickSlotIndex)
{
    if (QuickSlots[QuickSlotIndex].IsEmpty()) return false;
    
    QuickSlots[QuickSlotIndex].Clear();
    
    OnQuickSlotChanged.Broadcast(QuickSlotIndex, QuickSlots[QuickSlotIndex]);
    
    return true;
}

bool UQuickSlotsComponent::UseQuickSlot(int32 QuickSlotIndex)
{
    if (!QuickSlots.IsValidIndex(QuickSlotIndex)) return false;
    if (QuickSlots[QuickSlotIndex].IsEmpty()) return false;
    
    if (!GetOwner()->HasAuthority())
    {
        Server_UseQuickSlot(QuickSlotIndex);
        return true;
    }
    
    return Internal_UseQuickSlot(QuickSlotIndex);
}

bool UQuickSlotsComponent::Internal_UseQuickSlot(int32 QuickSlotIndex)
{
    UInventoryComponent* InvComp = GetInventoryComponent();
    if (!InvComp) return false;
    
    const FInventorySlot& Slot = QuickSlots[QuickSlotIndex];
    if (Slot.IsEmpty()) return false;
    
    // Find original item in inventory by InstanceID
    int32 InvIndex = InvComp->FindSlotIndexByInstanceID(Slot.InstanceID);
    if (InvIndex == INDEX_NONE)
    {
        // Item no longer in inventory, clear quickslot
        Internal_ClearQuickSlot(QuickSlotIndex);
        return false;
    }
    
    // Delegate to inventory's use logic
    bool bUsed = InvComp->UseItem(FGameplayTag(), InvIndex);
    
    if (bUsed)
    {
        OnQuickSlotUsed.Broadcast(QuickSlotIndex);
        
        // Check if item was consumed (quantity 0)
        const TArray<FInventorySlot>& Inventory = InvComp->GetInventory();
        if (Inventory.IsValidIndex(InvIndex) && Inventory[InvIndex].IsEmpty())
        {
            Internal_ClearQuickSlot(QuickSlotIndex);
        }
    }
    
    return bUsed;
}

bool UQuickSlotsComponent::SwapQuickSlots(int32 IndexA, int32 IndexB)
{
    if (!QuickSlots.IsValidIndex(IndexA) || !QuickSlots.IsValidIndex(IndexB)) return false;
    if (IndexA == IndexB) return false;
    
    if (!GetOwner()->HasAuthority())
    {
        Server_SwapQuickSlots(IndexA, IndexB);
        return true;
    }
    
    return Internal_SwapQuickSlots(IndexA, IndexB);
}

bool UQuickSlotsComponent::Internal_SwapQuickSlots(int32 IndexA, int32 IndexB)
{
    FInventorySlot Temp = QuickSlots[IndexA];
    QuickSlots[IndexA] = QuickSlots[IndexB];
    QuickSlots[IndexB] = Temp;
    
    // Update equipped slot types
    if (!QuickSlots[IndexA].IsEmpty())
    {
        QuickSlots[IndexA].SlotType = FGameplayTag::RequestGameplayTag(
            FName(*FString::Printf(TEXT("Slot.QuickSlot.%d"), IndexA + 1)));
    }
    if (!QuickSlots[IndexB].IsEmpty())
    {
        QuickSlots[IndexB].SlotType = FGameplayTag::RequestGameplayTag(
            FName(*FString::Printf(TEXT("Slot.QuickSlot.%d"), IndexB + 1)));
    }
    
    OnQuickSlotChanged.Broadcast(IndexA, QuickSlots[IndexA]);
    OnQuickSlotChanged.Broadcast(IndexB, QuickSlots[IndexB]);
    
    return true;
}

// ============================================================================
// REPLICATION
// ============================================================================

void UQuickSlotsComponent::OnRep_QuickSlots()
{
    for (int32 i = 0; i < QuickSlots.Num(); ++i)
    {
        OnQuickSlotChanged.Broadcast(i, QuickSlots[i]);
    }
}

// ============================================================================
// SERVER RPCs
// ============================================================================

bool UQuickSlotsComponent::Server_AssignToQuickSlot_Validate(int32 InventoryIndex, int32 QuickSlotIndex)
{
    return QuickSlots.IsValidIndex(QuickSlotIndex);
}

void UQuickSlotsComponent::Server_AssignToQuickSlot_Implementation(int32 InventoryIndex, int32 QuickSlotIndex)
{
    Internal_AssignToQuickSlot(InventoryIndex, QuickSlotIndex);
}

bool UQuickSlotsComponent::Server_ClearQuickSlot_Validate(int32 QuickSlotIndex)
{
    return QuickSlots.IsValidIndex(QuickSlotIndex);
}

void UQuickSlotsComponent::Server_ClearQuickSlot_Implementation(int32 QuickSlotIndex)
{
    Internal_ClearQuickSlot(QuickSlotIndex);
}

bool UQuickSlotsComponent::Server_UseQuickSlot_Validate(int32 QuickSlotIndex)
{
    return QuickSlots.IsValidIndex(QuickSlotIndex);
}

void UQuickSlotsComponent::Server_UseQuickSlot_Implementation(int32 QuickSlotIndex)
{
    Internal_UseQuickSlot(QuickSlotIndex);
}

bool UQuickSlotsComponent::Server_SwapQuickSlots_Validate(int32 IndexA, int32 IndexB)
{
    return QuickSlots.IsValidIndex(IndexA) && QuickSlots.IsValidIndex(IndexB);
}

void UQuickSlotsComponent::Server_SwapQuickSlots_Implementation(int32 IndexA, int32 IndexB)
{
    Internal_SwapQuickSlots(IndexA, IndexB);
}