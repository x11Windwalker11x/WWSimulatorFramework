#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MasterWidgets/ManagedWidget_Master.h"
#include "RootWidget.generated.h"
/*
 *Lives under the very root of all widgets
 *Achieves drag and drop ops
 */
UCLASS()
class MODULARINVENTORYSYSTEM_API URootWidget : public UManagedWidget_Master
{
	GENERATED_BODY()

protected:
	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation
	) override;
};

