// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "AWF_DragDropOperation.generated.h"

/**
 * Drag Drop Operation for Window Dragging
 * Holds the widget reference and drag offset during drag operations
 * Based on Epic's official UMG Drag & Drop tutorial pattern
 */
UCLASS()
class ADVANCEDWIDGETFRAMEWORK_API UAWF_DragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** Reference to the widget being dragged */
	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	UUserWidget* WidgetReference = nullptr;
    
	/** Offset from widget's top-left corner where the user clicked */
	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	FVector2D DragOffset = FVector2D::ZeroVector;
};
















