// SharedDefaults/Interfaces/SimulatorFramework/DeviceInterface.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "DeviceInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDeviceInterface : public UInterface
{
	GENERATED_BODY()
};

class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API IDeviceInterface
{
	GENERATED_BODY()

public:
	// === STATE ===
    
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Device")
	FGameplayTag GetDeviceState() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Device")
	void SetDeviceState(FGameplayTag NewState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Device")
	bool IsUsable() const;

	// === POWER ===
    
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Device")
	void TurnOn();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Device")
	void TurnOff();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Device")
	bool IsPoweredOn() const;

	// === MAINTENANCE ===
    
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Device")
	bool NeedsMaintenance() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Device")
	void PerformMaintenance();

	// === COMPONENT ACCESS ===
    
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Device")
	UActorComponent* GetDeviceComponent();
};