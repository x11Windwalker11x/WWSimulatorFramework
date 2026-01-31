// BoxSelectionWidget.cpp
#include "Interaction/BoxSelectionWidget.h"

void UBoxSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();
    
	if (Border_SelectionBox)
	{
		// Set cyan color with transparency
		FLinearColor BorderColor(1.0f, 0.7f, 0.0f, 0.9f);
		Border_SelectionBox->SetBrushColor(BorderColor);
        
		// Set border thickness
		Border_SelectionBox->SetPadding(FMargin(2.0f));
	}
}