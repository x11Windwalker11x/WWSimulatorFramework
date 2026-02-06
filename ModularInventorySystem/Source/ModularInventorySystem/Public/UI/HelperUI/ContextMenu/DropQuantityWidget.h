// DropQuantityWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "GameplayTags.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "Subsystems/InventoryWidgetManager.h" // Contains FInventorySlotReference
#include "DropQuantityWidget.generated.h"

class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuantityConfirmed, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuantityCancelled);

UCLASS()
class MODULARINVENTORYSYSTEM_API UDropQuantityWidget : public UManagedWidget_Master
{
	GENERATED_BODY()

public:
	UDropQuantityWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY()
	TArray<FInventorySlotReference> TargetSlots;

	UPROPERTY()
	bool bIsMultiSelection = false;
	
	UPROPERTY(meta = (BindWidget))
	USlider* Slider_Quantity;

	UPROPERTY(meta = (BindWidget))
	UEditableText* EditableText_Quantity;

	UPROPERTY()
	UInventoryComponent* TargetInventoryComp;

	UPROPERTY()
	FGameplayTag TargetInventoryType;

	UPROPERTY()
	int32 TargetSlotIndex;

	void InitializeQuantityWidget(UInventoryComponent* InvComp, FGameplayTag InvType, int32 SlotIndex, int32 InMaxQuantity);
	void InitializeForMultiSelection(UInventoryComponent* InvComp, TArray<FInventorySlotReference> InSlots, int32 MinQuantity);

	UPROPERTY(BlueprintAssignable, Category = "Drop Quantity")
	FOnQuantityConfirmed OnQuantityConfirmed;
    
	UPROPERTY(BlueprintAssignable, Category = "Drop Quantity")
	FOnQuantityCancelled OnQuantityCancelled;
    
	UFUNCTION(BlueprintCallable, Category = "Drop Quantity")
	void InitializeQuantityWidget(int32 InMaxQuantity);
	
	UFUNCTION(BlueprintPure, Category = "Drop Quantity")
	int32 GetSelectedQuantity() const { return SelectedQuantity; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Drop Quantity")
	int32 MaxQuantity = 1;
    
	UPROPERTY(BlueprintReadOnly, Category = "Drop Quantity")
	int32 SelectedQuantity = 1;
    
	UFUNCTION(BlueprintCallable, Category = "Drop Quantity")
	void ConfirmDrop();
    
	UFUNCTION(BlueprintCallable, Category = "Drop Quantity")
	void ConfirmDropAll();
    
	UFUNCTION(BlueprintCallable, Category = "Drop Quantity")
	void Cancel();
    
	UFUNCTION(BlueprintCallable, Category = "Drop Quantity")
	void OnSliderValueChanged(float Value);
};