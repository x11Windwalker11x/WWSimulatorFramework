#include "Utilities/Helpers/MSB_BlueprintFunctionLibrary_Base.h"


UActorComponent* UMSB_BlueprintFunctionLibrary_Base::GetInventoryComponent(UActorComponent* InventoryComponent)
{
	if (InventoryComponent == nullptr) return NULL;
	if (InventoryComponent->GetClass()->ImplementsInterface(UInventoryInterface::StaticClass()))
	{
		return InventoryComponent;
	}
	return NULL;
}

UActorComponent* UMSB_BlueprintFunctionLibrary_Base::GetCrafterComponent(UActorComponent* CrafterComponent)
{
	if (CrafterComponent == nullptr) return NULL;
	if (CrafterComponent->GetClass()->ImplementsInterface(UInventoryInterface::StaticClass()))
	{
		return CrafterComponent;
	}
	return NULL;
}
