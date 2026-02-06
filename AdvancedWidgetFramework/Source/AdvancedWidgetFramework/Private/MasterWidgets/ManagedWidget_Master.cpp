// Fill your copyright here.

#include "MasterWidgets/ManagedWidget_Master.h"
#include "Subsystems/AdvancedWidgetFramework/WidgetManagerBase.h"

void UManagedWidget_Master::NativeConstruct()
{
	Super::NativeConstruct();

	if (bAutoRegister && WidgetCategoryTag.IsValid())
	{
		if (UWidgetManagerBase* WidgetMgr = UWidgetManagerBase::Get(GetOwningPlayer()))
		{
			WidgetMgr->RegisterWidgetWithCategory(this, WidgetCategoryTag);
		}
	}
}

void UManagedWidget_Master::NativeDestruct()
{
	if (bAutoRegister)
	{
		if (UWidgetManagerBase* WidgetMgr = UWidgetManagerBase::Get(GetOwningPlayer()))
		{
			WidgetMgr->UnregisterWidgetFromStack(this);
		}
	}

	Super::NativeDestruct();
}
