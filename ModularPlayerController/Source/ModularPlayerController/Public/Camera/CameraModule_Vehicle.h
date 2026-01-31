// Windwalker Modular Player Framework - Racing/Vehicle Camera

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModule_Master.h"
#include "CameraModule_Vehicle.generated.h"

/*
 * Racing/Vehicle camera module
 * Multiple camera modes for vehicles — chase, cockpit, hood, etc.
 */
UCLASS(Blueprintable, BlueprintType)
class MODULARPLAYERCONTROLLER_API UCameraModule_Vehicle : public UCameraModule_Master
{
    GENERATED_BODY()

public:
    UCameraModule_Vehicle();
};