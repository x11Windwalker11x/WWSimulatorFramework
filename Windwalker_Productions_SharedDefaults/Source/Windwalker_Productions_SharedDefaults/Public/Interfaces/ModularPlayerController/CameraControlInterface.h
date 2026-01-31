// CameraControlInterface.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Lib/Events/CameraEvents.h"
#include "CameraControlInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCameraControlInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that control camera modes
 * Implemented by camera manager or orchestrator component
 */
class SHAREDDEFAULTS_API ICameraControlInterface
{
	GENERATED_BODY()

public:
	/** Request a camera mode change */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
	bool RequestCameraMode(const FCameraModeRequest& Request);

	/** Release a previously requested camera mode */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
	void ReleaseCameraMode(const FCameraModeRelease& Release);

	/** Get current active camera mode */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
	FGameplayTag GetCurrentCameraMode() const;

	/** Check if a specific mode is active */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera")
	bool IsCameraModeActive(const FGameplayTag& ModeTag) const;
};