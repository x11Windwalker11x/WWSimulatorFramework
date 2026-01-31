#include "UI/RootWidget.h"
#include "Operations/AWF_DragDropOperation.h"
#include "UI/InventoryResizableWindowWidget.h"

bool URootWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UAWF_DragDropOperation* DragOp =
		Cast<UAWF_DragDropOperation>(InOperation);

	if (!DragOp || !DragOp->WidgetReference)
	{
		return false;
	}

	UInventoryResizableWindowWidget* Window =
		Cast<UInventoryResizableWindowWidget>(DragOp->WidgetReference);

	if (!Window)
	{
		return false;
	}

	const FVector2D LocalMouse =
		InGeometry.AbsoluteToLocal(
			InDragDropEvent.GetScreenSpacePosition()
		);

	const FVector2D NewPos = LocalMouse - DragOp->DragOffset;

	Window->SetPositionInViewport(NewPos, false);
	Window->SetVisibility(ESlateVisibility::Visible);

	return true;
}

