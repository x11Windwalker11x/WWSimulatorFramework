// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ECheatPermissionLevel.h"
#include "MPC_PlayerState.generated.h"

/**
 * 
 */
 // 
UCLASS()
class MODULARPLAYERCONTROLLER_API AMPC_PlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    UPROPERTY(Replicated, BlueprintReadOnly)
    ECheatPermissionLevel CheatPermissionLevel = ECheatPermissionLevel::GodMode;

    void SetCheatPermission(ECheatPermissionLevel NewPermission);
    UFUNCTION()
    void OnRep_SetCheatPermission();

    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
};

