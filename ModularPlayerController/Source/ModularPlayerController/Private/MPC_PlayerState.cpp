	// Fill out your copyright notice in the Description page of Project Settings.


#include "MPC_PlayerState.h"
#include "ModularPlayerController_Master.h"

// MyPlayerState.cpp
void AMPC_PlayerState::SetCheatPermission(ECheatPermissionLevel NewPermission)
{
    CheatPermissionLevel = NewPermission;
}

void AMPC_PlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMPC_PlayerState, CheatPermissionLevel);
}

void AMPC_PlayerState::OnRep_SetCheatPermission()
{
    if (AModularPlayerController_Master* MPC = Cast<AModularPlayerController_Master>(GetOwner()))
    {
        MPC->SetCheatPermission(CheatPermissionLevel);
    }
}