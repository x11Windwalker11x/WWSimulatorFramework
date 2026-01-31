// CameraShakeModule_Combat.h
// Windwalker Framework - Combat Camera Shake Module

#pragma once

#include "CoreMinimal.h"
#include "Camera/Shake/CameraShakeModule_Master.h"
#include "CameraShakeModule_Combat.generated.h"

/**
 * Combat-themed camera shake module
 * Presets for: explosions, gunfire, melee, impacts, etc.
 */
UCLASS(Blueprintable, BlueprintType)
class MODULARPLAYERCONTROLLER_API UCameraShakeModule_Combat : public UCameraShakeModule_Master
{
	GENERATED_BODY()

public:
	UCameraShakeModule_Combat();
};