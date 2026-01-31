// CameraModule_TPS.h
// Windwalker Framework - Third Person Shoulder Camera Module

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModule_Master.h"
#include "CameraModule_TPS.generated.h"

/**
 * Third Person Shoulder camera module
 * Supports: Aim states, Shoulder swap, Zoom
 */
UCLASS(Blueprintable, BlueprintType)
class MODULARPLAYERCONTROLLER_API UCameraModule_TPS : public UCameraModule_Master
{
    GENERATED_BODY()

public:
    UCameraModule_TPS();
};