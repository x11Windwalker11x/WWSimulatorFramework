#include "MPC_GameMode.h"
#include "MPC_PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "ModularCheatManager_Master.h"
#include "DeveloperWhitelistAsset.h"
#include "ModularPlayerController_Master.h"
#include "InteractionSystem/InteractionDefaultsConstants.h"
#include "Subsystems/WidgetManager.h"

AMPC_GameMode::AMPC_GameMode()
{
    WidgetManager = nullptr;
}

void AMPC_GameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->IsLocalController())
        {
            WidgetManager = UWidgetManager::Get(PC);
            CreateRootWidget(bUseDragAndDropOps);
        }
    }

    if (!WidgetManager)
    {
        UE_LOG(LogTemp, Error, TEXT("WidgetManager not found!"));
        return;
    }

    RegisterWidgetPool(
        WidgetManager,
        InteractionDefaultsConstants::Widget_InteractionPromptPath,
        0,
        InteractionDefaultsConstants::PoolName_InteractionPrompts,
        InteractionDefaultsConstants::ZOrder_InteractionPrompts
    );
    
    RegisterWidgetPool(
        WidgetManager,
        InteractionDefaultsConstants::Widget_PreInteractionPromptPath,
        0,
        InteractionDefaultsConstants::PoolName_PreInteractionDots,
        InteractionDefaultsConstants::ZOrder_PreInteractionDots
    );
}

bool AMPC_GameMode::RegisterWidgetPool(
    UWidgetManager* InWidgetManager,
    const TCHAR* InWidgetPath,
    int32 InPoolSize,
    FName InPoolName,
    int32 InZOrder)
{
    if (!InWidgetManager)
    {
        UE_LOG(LogTemp, Error, TEXT("WidgetManager is null!"));
        return false;
    }

    TSubclassOf<UUserWidget> WidgetClass = LoadClass<UUserWidget>(nullptr, InWidgetPath);
    
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load widget: %s"), InWidgetPath);
        return false;
    }

    InWidgetManager->RegisterWidget(
        WidgetClass,
        InPoolSize,
        InZOrder,
        false,
        false
    );
    
    UE_LOG(LogTemp, Log, TEXT("Widget '%s' registered (Pool=%d, Z:%d)"), 
        *WidgetClass->GetName(), InPoolSize, InZOrder);
    
    return true;
}

void AMPC_GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!NewPlayer) return;

    AMPC_PlayerState* PS = NewPlayer->GetPlayerState<AMPC_PlayerState>();
    if (!PS) return;

    FString DeveloperName = PS->GetName();
    ECheatPermissionLevel AssignedPermission = ECheatPermissionLevel::GodMode;
    
    UE_LOG(LogTemp, Log, TEXT("DeveloperName from PlayerState: %s"), *DeveloperName);

    if (DeveloperWhitelistAsset && DeveloperWhitelistAsset->HasDeveloper(DeveloperName))
    {
        AssignedPermission = DeveloperWhitelistAsset->GetPermissionForPlayer(DeveloperName);
    }

    if (AController* PC = Cast<AController>(PS->GetOwner()))
    {
        if (AModularPlayerController_Master* MPC = Cast<AModularPlayerController_Master>(PC))
        {
            PS->SetCheatPermission(AssignedPermission);
            UE_LOG(LogTemp, Log, TEXT("Assigned cheat permission %s to player %s"), 
                *UEnum::GetValueAsString(AssignedPermission), *PS->GetPlayerName());
        }
    }
}
void AMPC_GameMode::CreateRootWidget(bool bUseDragDropOps)
{
    URootWidget* WDGT_Root =
    CreateWidget<URootWidget>(
        GetWorld(),
        RootWidgetClass
    );

    WDGT_Root->AddToViewport(0); // lowest Z-order

}