#include "UI/Widget_PreInteraction.h"
#include "Components/Image.h"

void UWidget_PreInteraction::SetDotColor(FLinearColor NewColor)
{
	if (Image_PreInteraction)
	{
		Image_PreInteraction->SetColorAndOpacity(NewColor);
	}
}

void UWidget_PreInteraction::SetDotSize(float Size)
{
	if (Image_PreInteraction)
	{
		Image_PreInteraction->SetRenderScale(FVector2D(Size, Size));
	}
}
