#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTags.h"
#include "Interfaces/ModularInventorySystem/InventoryInterface.h"
#include "Factories/BlueprintFunctionLibraryFactory.h"
#include "MSB_BlueprintFunctionLibrary_Base.generated.h"

UCLASS()
class MODULARSYSTEMSBASE_API UMSB_BlueprintFunctionLibrary_Base : public UBlueprintFunctionLibraryFactory
{
	GENERATED_BODY()

public:

	//InterfaceCalls
	UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
	static UActorComponent* GetInventoryComponent(UActorComponent* InventoryComponent);
	UFUNCTION(BlueprintPure, Category = "Crafting Helpers")
	static UActorComponent* GetCrafterComponent(UActorComponent* CrafterComponent);
	
};



#include "Kismet/KismetSystemLibrary.h"

template<typename T>
static void ExecuteMyInterfaceFunc(T* Target)
{
	// 1. Static check: Ensure we are passing an Actor or Component
	static_assert(std::is_base_of_v<UObject, T>, "Target must be a UObject (Actor or Component).");

	if (!Target) return;

	// 2. Runtime check: Does this specific instance implement the interface?
	// Replace 'UMyInterface' with your actual interface class name
	if (Target->GetClass()->ImplementsInterface(UMyInterface::StaticClass()))
	{
		// 3. Execution: Using the UHT generated static Execute_ function
		// This works regardless of whether the interface is implemented in C++ or Blueprint
		IMyInterface::Execute_SomeInterfaceFunction(Target /*, args here */);
	}
}