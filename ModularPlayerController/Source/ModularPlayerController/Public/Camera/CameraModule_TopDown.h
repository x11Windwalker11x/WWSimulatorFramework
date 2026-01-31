// UCameraModule_TopDown.h
// Windwalker Modular Framework - Top Down Camera

#pragma once

#include "CoreMinimal.h"
#include "CameraModule_Master.h"
#include "CameraModule_TopDown.generated.h"

/**
 * Top Down camera module
 * RTS/MOBA/ARPG style — fixed angle looking down at character
 * Independent of UGC (no SpringArm rotation from player input)
 */
UCLASS(Blueprintable, BlueprintType)
class MODULARPLAYERCONTROLLER_API UCameraModule_TopDown : public UCameraModule_Master
{
    GENERATED_BODY()

public:
    UCameraModule_TopDown();
};