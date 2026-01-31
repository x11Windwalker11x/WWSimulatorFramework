// UCameraModule_TrueFPS.h
// Windwalker Modular Framework - True First Person Camera

#pragma once

#include "CoreMinimal.h"
#include "CameraModule_Master.h"
#include "Camera/CameraModule_Master.h"
#include "CameraModule_TrueFPS.generated.h"

class ACharacter;

/**
 * True First Person camera module
 * Camera attached to head socket — player sees their body/legs looking down
 * Independent of UGC (no SpringArm)
 */
UCLASS(Blueprintable, BlueprintType)
class MODULARPLAYERCONTROLLER_API UCameraModule_TrueFPS : public UCameraModule_Master
{
    GENERATED_BODY()

public:
    UCameraModule_TrueFPS();
};