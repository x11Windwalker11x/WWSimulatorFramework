// WidgetManager.cpp
// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/WidgetManager.h"
#include "Blueprint/UserWidget.h"
#include "Components/InventoryComponent.h"
#include "UI/HelperUI/ContextMenu/MIIS_RightClickContextMenu.h"
#include "Logging/InteractableInventoryLogging.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UWidgetManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("WidgetManager initialized"));
}

void UWidgetManager::Deinitialize()
{
    ClearSelection();
    ExitQuestCombineMode();
    ExitAttachmentMode();
    ExitCompareMode();

    Super::Deinitialize();
}

UWidgetManager* UWidgetManager::Get(APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return nullptr;
    }
    
    ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    if (!LocalPlayer)
    {
        return nullptr;
    }
    
    return LocalPlayer->GetSubsystem<UWidgetManager>();
}

// ============================================================================
// WIDGET MANAGEMENT
// ============================================================================

UUserWidget* UWidgetManager::ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC || !WidgetClass)
    {
        return nullptr;
    }
    
    UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (Widget)
    {
        Widget->AddToViewport(ZOrder);
    }
    
    return Widget;
}

void UWidgetManager::HideWidget(UUserWidget* Widget)
{
    if (Widget)
    {
        Widget->RemoveFromParent();
    }
}

UUserWidget* UWidgetManager::ShowWidgetAtCursor(TSubclassOf<UUserWidget> WidgetClass, FVector2D Offset)
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC || !WidgetClass)
    {
        return nullptr;
    }

    // Create widget with high Z-order (above most UI)
    UUserWidget* Widget = ShowWidget(WidgetClass, 100);
    if (Widget)
    {
        float MouseX, MouseY;
        if (PC->GetMousePosition(MouseX, MouseY))
        {
            Widget->SetPositionInViewport(FVector2D(MouseX, MouseY) + Offset, false);
        }
    }

    return Widget;
}

void UWidgetManager::CloseActiveContextMenu()
{
    if (ActiveContextMenu)
    {
        ActiveContextMenu->RemoveFromParent();
        ActiveContextMenu = nullptr;
    }
}

// ============================================================================
// SLOT SELECTION
// ============================================================================

bool UWidgetManager::IsSlotSelected(FGameplayTag InventoryType, int32 SlotIndex) const
{
    FInventorySlotReference Ref(InventoryType, SlotIndex);
    return SelectedSlots.Contains(Ref);
}

void UWidgetManager::SelectSlot(FGameplayTag InventoryType, int32 SlotIndex)
{
    FInventorySlotReference Ref(InventoryType, SlotIndex);
    if (!SelectedSlots.Contains(Ref))
    {
        SelectedSlots.Add(Ref);
        OnSelectionChanged.Broadcast();
    }
}

void UWidgetManager::DeselectSlot(FGameplayTag InventoryType, int32 SlotIndex)
{
    FInventorySlotReference Ref(InventoryType, SlotIndex);
    if (SelectedSlots.Remove(Ref) > 0)
    {
        OnSelectionChanged.Broadcast();
    }
}

void UWidgetManager::ToggleSlotSelection(FGameplayTag InventoryType, int32 SlotIndex)
{
    if (IsSlotSelected(InventoryType, SlotIndex))
    {
        DeselectSlot(InventoryType, SlotIndex);
    }
    else
    {
        SelectSlot(InventoryType, SlotIndex);
    }
}

void UWidgetManager::ClearSelection()
{
    if (SelectedSlots.Num() > 0)
    {
        SelectedSlots.Empty();
        OnSelectionChanged.Broadcast();
    }
}

int32 UWidgetManager::GetSelectedSlotCount() const
{
    return SelectedSlots.Num();
}

TArray<FInventorySlotReference> UWidgetManager::GetSelectedSlots() const
{
    return SelectedSlots;
}

// ============================================================================
// HELPER WIDGETS
// ============================================================================

void UWidgetManager::ShowSplitQuantityWidget(FGameplayTag InventoryType, int32 SlotIndex)
{
    // TODO: Implement split quantity widget display
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("ShowSplitQuantityWidget called for slot %d"), SlotIndex);
}

void UWidgetManager::ShowExamineWidget(FGameplayTag InventoryType, int32 SlotIndex)
{
    // TODO: Implement examine widget display
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("ShowExamineWidget called for slot %d"), SlotIndex);
}

// ============================================================================
// QUEST COMBINE MODE
// ============================================================================

void UWidgetManager::EnterQuestCombineMode(FGameplayTag InventoryType, int32 SourceSlotIndex)
{
    // Exit attachment mode if active
    if (bIsInAttachmentMode)
    {
        ExitAttachmentMode();
    }
    
    bIsInQuestCombineMode = true;
    QuestCombineSourceInventory = InventoryType;
    QuestCombineSourceSlot = SourceSlotIndex;
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("🧪 Entered Quest Combine Mode - Source: %s, Slot: %d"),
        *InventoryType.ToString(), SourceSlotIndex);
    
    // Broadcast mode change so UI can update (highlight valid targets)
    OnSelectionModeChanged.Broadcast();
}

void UWidgetManager::ExitQuestCombineMode()
{
    if (!bIsInQuestCombineMode)
    {
        return;
    }
    
    bIsInQuestCombineMode = false;
    QuestCombineSourceInventory = FGameplayTag();
    QuestCombineSourceSlot = -1;
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("🧪 Exited Quest Combine Mode"));
    
    OnSelectionModeChanged.Broadcast();
}

void UWidgetManager::GetQuestCombineSource(FGameplayTag& OutInventoryType, int32& OutSlotIndex) const
{
    OutInventoryType = QuestCombineSourceInventory;
    OutSlotIndex = QuestCombineSourceSlot;
}

// ============================================================================
// ATTACHMENT MODE
// ============================================================================

void UWidgetManager::EnterAttachmentMode(FGameplayTag InventoryType, int32 AttachmentSlotIndex)
{
    // Exit quest combine mode if active
    if (bIsInQuestCombineMode)
    {
        ExitQuestCombineMode();
    }
    
    bIsInAttachmentMode = true;
    AttachmentSourceInventory = InventoryType;
    AttachmentSourceSlot = AttachmentSlotIndex;
    
    UE_LOG(LogInventoryInteractableSystem, Log, 
        TEXT("🔧 Entered Attachment Mode - Source: %s, Slot: %d"),
        *InventoryType.ToString(), AttachmentSlotIndex);
    
    // Broadcast mode change so UI can dim incompatible slots
    OnSelectionModeChanged.Broadcast();
}

void UWidgetManager::ExitAttachmentMode()
{
    if (!bIsInAttachmentMode)
    {
        return;
    }
    
    bIsInAttachmentMode = false;
    AttachmentSourceInventory = FGameplayTag();
    AttachmentSourceSlot = -1;
    
    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("🔧 Exited Attachment Mode"));
    
    OnSelectionModeChanged.Broadcast();
}

void UWidgetManager::GetAttachmentSource(FGameplayTag& OutInventoryType, int32& OutSlotIndex) const
{
    OutInventoryType = AttachmentSourceInventory;
    OutSlotIndex = AttachmentSourceSlot;
}

bool UWidgetManager::CanSlotAcceptAttachment(FGameplayTag InventoryType, int32 SlotIndex) const
{
    if (!bIsInAttachmentMode)
    {
        return false;
    }
    
    UInventoryComponent* InventoryComp = GetPlayerInventoryComponent();
    if (!InventoryComp)
    {
        return false;
    }
    
    // Delegate to InventoryComponent for actual compatibility check
    return InventoryComp->CanAttachToSlot(
        AttachmentSourceInventory, AttachmentSourceSlot,
        InventoryType, SlotIndex
    );
}

// ============================================================================
// COMPARE MODE
// ============================================================================

void UWidgetManager::EnterCompareMode(FGameplayTag InventoryType, int32 SlotIndex)
{
    // Exit other modes if active
    if (bIsInQuestCombineMode)
    {
        ExitQuestCombineMode();
    }
    if (bIsInAttachmentMode)
    {
        ExitAttachmentMode();
    }

    bIsInCompareMode = true;
    CompareSourceInventory = InventoryType;
    CompareSourceSlot = SlotIndex;

    UE_LOG(LogInventoryInteractableSystem, Log,
        TEXT("📊 Entered Compare Mode - Source: %s, Slot: %d"),
        *InventoryType.ToString(), SlotIndex);

    OnSelectionModeChanged.Broadcast();
}

void UWidgetManager::ExitCompareMode()
{
    if (!bIsInCompareMode)
    {
        return;
    }

    bIsInCompareMode = false;
    CompareSourceInventory = FGameplayTag();
    CompareSourceSlot = INDEX_NONE;

    UE_LOG(LogInventoryInteractableSystem, Log, TEXT("📊 Exited Compare Mode"));

    OnSelectionModeChanged.Broadcast();
}

void UWidgetManager::GetCompareSource(FGameplayTag& OutInventoryType, int32& OutSlotIndex) const
{
    OutInventoryType = CompareSourceInventory;
    OutSlotIndex = CompareSourceSlot;
}

// ============================================================================
// HOVER TRACKING
// ============================================================================

bool UWidgetManager::GetHoveredInventorySlot(FGameplayTag& OutInventoryType, int32& OutSlotIndex) const
{
    if (bHasHoveredSlot)
    {
        OutInventoryType = HoveredSlot.InventoryType;
        OutSlotIndex = HoveredSlot.SlotIndex;
        return true;
    }

    OutInventoryType = FGameplayTag();
    OutSlotIndex = INDEX_NONE;
    return false;
}

void UWidgetManager::SetHoveredSlot(FGameplayTag InventoryType, int32 SlotIndex)
{
    HoveredSlot.InventoryType = InventoryType;
    HoveredSlot.SlotIndex = SlotIndex;
    bHasHoveredSlot = true;
}

void UWidgetManager::ClearHoveredSlot()
{
    HoveredSlot = FInventorySlotReference();
    bHasHoveredSlot = false;
}

// ============================================================================
// HELPERS
// ============================================================================

UInventoryComponent* UWidgetManager::GetPlayerInventoryComponent() const
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return nullptr;
    }
    
    APawn* Pawn = PC->GetPawn();
    if (!Pawn)
    {
        return nullptr;
    }
    
    return Pawn->FindComponentByClass<UInventoryComponent>();
}

APlayerController* UWidgetManager::GetOwningPlayer() const
{
    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if (!LocalPlayer)
    {
        return nullptr;
    }
    
    return LocalPlayer->GetPlayerController(GetWorld());
}