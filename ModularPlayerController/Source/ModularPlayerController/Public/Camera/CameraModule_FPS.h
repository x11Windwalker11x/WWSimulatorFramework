// UCameraModule_FPS.h
// Windwalker Modular Framework - Classic First Person Camera

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModule_Master.h"
#include "CameraModule_FPS.generated.h"

/**
 * Classic first-person camera module
 * Camera at eye level, only hands/weapon visible (no body when looking down)
 * 
 * Features:
 * - Camera attached to eye/head position
 * - Hide character mesh (body not visible)
 * - Optional separate first-person mesh for hands/weapon
 * - Configurable camera offset
 * - Head bob support
 * - Independent of SpringArm
 * 
 * Usage:
 *   CameraManager->SetCameraMode(FGameplayTag::RequestGameplayTag("Camera.Mode.FPS"));
 */
UCLASS(Blueprintable, BlueprintType)
class MODULARPLAYERCONTROLLER_API UCameraModule_FPS : public UCameraModule_Master
{
    GENERATED_BODY()

public:
    UCameraModule_FPS();
};