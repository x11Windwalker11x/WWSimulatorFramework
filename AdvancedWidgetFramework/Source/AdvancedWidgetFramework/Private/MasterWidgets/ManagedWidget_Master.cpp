// Fill your copyright here.

#include "MasterWidgets/ManagedWidget_Master.h"
#include "Subsystems/WidgetManager.h"

void UManagedWidget_Master::NativeConstruct()
{
	Super::NativeConstruct();
    
	if (bAutoRegister && WidgetCategoryTag.IsValid())
	{
		if (UWidgetManager* WidgetMgr = UWidgetManager::Get(GetOwningPlayer()))
		{
			WidgetMgr->RegisterWidgetWithCategory(this, WidgetCategoryTag);
		}
	}
}

void UManagedWidget_Master::NativeDestruct()
{
	if (bAutoRegister)
	{
		if (UWidgetManager* WidgetMgr = UWidgetManager::Get(GetOwningPlayer()))
		{
			WidgetMgr->UnregisterWidgetFromStack(this);
		}
	}
    
	Super::NativeDestruct();
}
