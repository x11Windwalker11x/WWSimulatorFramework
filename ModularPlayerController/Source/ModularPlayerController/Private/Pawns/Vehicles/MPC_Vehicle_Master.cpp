// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Vehicles/MPC_Vehicle_Master.h"

//#include "Editor/StatsViewer/Public/StatsViewerUtils.h"

// Sets default values
AMPC_Vehicle_Master::AMPC_Vehicle_Master()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
//Components
	PawnComponentManager = CreateDefaultSubobject<UPawnComponentManager>(TEXT("PawnComponentManager"));
	InRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("InRoot"));
	SK_GFX = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_GFX"));
	InputActionMechanicComponent = CreateDefaultSubobject<UInputActionMechanicComponent>(TEXT("InputActionMechanic"));
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));
	InteractorComponent = CreateDefaultSubobject<UInteractorComponent>(TEXT("InteractorComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	//Controller
	OwnerPlayerController = nullptr;
	OwnerAIController = nullptr;
	OwnerPlayerCameraManager = nullptr;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	OwnerPlayerController = nullptr;
	OwnerAIController = nullptr;

	//SetupAttachments
	SetRootComponent(InRootComponent);
	//SM_GFX->SetupAttachment(RootComponent);
	SK_GFX->SetupAttachment(RootComponent);
	
	CameraBoom->SetupAttachment(RootComponent);
	CameraComponent->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	

	//Setup TPS

	CameraBoom->bUsePawnControlRotation = true;
	CameraComponent->bUsePawnControlRotation = false;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	


}

// Called when the game starts or when spawned
void AMPC_Vehicle_Master::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMPC_Vehicle_Master::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMPC_Vehicle_Master::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// void AMPC_Vehicle_Master::OnJumpActionPressed()
// {
// 	UE_LOG(LogInput, Display, TEXT("On JumpActionPressed"));
// }

// void AMPC_Vehicle_Master::OnJumpActionPressed_Implementation()
// {
// 	//Handbreak logic here
// 	UE_LOG(LogInput, Log, TEXT("HandBrake Action Pressed"));
// }

void AMPC_Vehicle_Master::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (PawnComponentManager)
	{
		PawnComponentManager->OnPawnPossessed(NewController);
	}

}


UInventoryComponent* AMPC_Vehicle_Master::GetInventoryComponent_Implementation()
{
	return InventoryComponent;
}

UActorComponent* AMPC_Vehicle_Master::GetInventoryComponentAsActorComponent_Implementation()
{
	return InventoryComponent;
}

UClass* AMPC_Vehicle_Master::GetInteractorClass_Implementation()
{
	return GetClass();
}

AController* AMPC_Vehicle_Master::GetInteractorController_Implementation()
{
	return GetController();
}

APlayerController* AMPC_Vehicle_Master::GetInteractorPlayerController_Implementation()
{
	return GetController<APlayerController>();
}

APawn* AMPC_Vehicle_Master::GetInteractorPawn_Implementation()
{
	return this;
}

ACharacter* AMPC_Vehicle_Master::GetInteractorCharacter_Implementation()
{
	return nullptr;
}

UActorComponent* AMPC_Vehicle_Master::GetInteractorComponentAsActorComponent_Implementation(
	TSubclassOf<UActorComponent> ComponentClass)
{
	return GetComponentByClass(ComponentClass);
}


void AMPC_Vehicle_Master::SaveVehicleState(const FString& PlayerID)
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UMasterSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UMasterSaveSubsystem>())
            {
                // Save Vehicle Transform
                if (UCharacterSaveModule* CharModule = SaveSubsystem->GetCharacterModule())
                {
                    CharModule->SaveCharacterTransform(PlayerID, GetActorTransform(), World->GetMapName());
                }
                
                // Save Inventory
                if (UInventorySaveModule* InvModule = SaveSubsystem->GetInventoryModule())
                {
                    if (InventoryComponent)
                    {
                        FPlayerInventoryData& InvData = InvModule->GetOrCreatePlayerInventory(PlayerID);
                        InvData.InventoryItems.Empty();
                        // Save vehicle inventory/cargo
                    }
                }
                
                SaveSubsystem->QuickSave();
                
                UE_LOG(LogTemp, Log, TEXT("Saved vehicle state for: %s"), *PlayerID);
            }
        }
    }
}

void AMPC_Vehicle_Master::LoadVehicleState(const FString& PlayerID)
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UMasterSaveSubsystem* SaveSubsystem = GI->GetSubsystem<UMasterSaveSubsystem>())
            {
                // Load Vehicle Transform
                if (UCharacterSaveModule* CharModule = SaveSubsystem->GetCharacterModule())
                {
                    FTransform LoadedTransform;
                    FString LoadedLevel;
                    
                    if (CharModule->LoadCharacterTransform(PlayerID, LoadedTransform, LoadedLevel))
                    {
                        SetActorTransform(LoadedTransform);
                    }
                }
                
                // Load Inventory
                if (UInventorySaveModule* InvModule = SaveSubsystem->GetInventoryModule())
                {
                    if (InventoryComponent && InvModule->HasPlayerData(PlayerID))
                    {
                        FPlayerInventoryData& InvData = InvModule->GetOrCreatePlayerInventory(PlayerID);
                        // Load vehicle inventory/cargo
                    }
                }
                
                UE_LOG(LogTemp, Log, TEXT("Loaded vehicle state for: %s"), *PlayerID);
            }
        }
    }
}

void AMPC_Vehicle_Master::OnCameraToggle_Implementation(APawn* ControlledPawn)
{
	if (!ControlledPawn || !PawnComponentManager) return;

	if (AMPC_PlayerCameraManager* CamManager = PawnComponentManager->GetOwnerCameraManager())
	{
		CamManager->ToggleCamera(ControlledPawn);
	}
}
