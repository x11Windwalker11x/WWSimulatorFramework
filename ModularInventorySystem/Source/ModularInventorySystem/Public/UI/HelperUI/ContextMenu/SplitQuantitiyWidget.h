// SplitQuantityWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Components/EditableText.h"
#include "GameplayTagContainer.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "SplitQuantitiyWidget.generated.h" 

class UInventoryComponent;
struct FInventorySlotReference;

UCLASS()
class MODULARINVENTORYSYSTEM_API USplitQuantityWidget : public UManagedWidget_Master
{
    GENERATED_BODY()

public:
    USplitQuantityWidget(const FObjectInitializer& ObjectInitializer);
    virtual void NativeConstruct() override;
    // Single slot split
    void InitializeSplitWidget(UInventoryComponent* InvComp, FGameplayTag InvType, int32 SlotIndex, int32 MaxQuantity);
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
    // Multi-selection split
    void InitializeForMultiSelection(UInventoryComponent* InvComp, TArray<FInventorySlotReference> InSlots, int32 MinQuantity);

protected:
    APlayerController* PC = nullptr;
    
    UPROPERTY()
    int32 MaxQuantity = 1;
    
    UPROPERTY()
    int32 SelectedQuantity = 1;
    
    UPROPERTY()
    UInventoryComponent* TargetInventoryComp = nullptr;
    
    UPROPERTY()
    FGameplayTag TargetInventoryType;
    
    UPROPERTY()
    int32 TargetSlotIndex = -1;
    
    UPROPERTY()
    TArray<FInventorySlotReference> TargetSlots;
    
    UPROPERTY()
    bool bIsMultiSelection = false;
    
    UPROPERTY(meta = (BindWidget))
    USlider* Slider_Quantity;
    
    UPROPERTY(meta = (BindWidget))
    UEditableText* EditableText_Quantity;
    
    UFUNCTION(BlueprintCallable)
    void ConfirmSplit();
    
    UFUNCTION(BlueprintCallable)
    void Cancel();
    
    UFUNCTION(BlueprintCallable)
    void OnSliderValueChanged(float Value);
};