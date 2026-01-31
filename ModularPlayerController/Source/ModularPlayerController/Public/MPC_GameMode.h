#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WidgetManager.h"
#include "Blueprint/UserWidget.h"
#include "UI/RootWidget.h"
#include "MPC_GameMode.generated.h"

class UWidgetSubsystem;
class UDeveloperWhitelistAsset;

UCLASS()
class MODULARPLAYERCONTROLLER_API AMPC_GameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMPC_GameMode();

    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;

    //Widgets
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UUserWidget> RootWidgetClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode")
    bool bUseDragAndDropOps = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cheats")
    UDeveloperWhitelistAsset* DeveloperWhitelistAsset;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
    UWidgetManager* WidgetManager;
protected:
    /**
     * Helper to register a widget pool with error logging
     */
    bool RegisterWidgetPool(
    UWidgetManager* InWidgetManager, 
        const TCHAR* InWidgetPath,
        int32 InPoolSize,
        FName InPoolName,
        int32 InZOrder
    );
    
    //Creates Root Widget for Drag&Drop Ops
    void CreateRootWidget(bool bUseDragDropOps = true);

};