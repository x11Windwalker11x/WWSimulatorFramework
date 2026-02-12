// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "GameplayTagContainer.h"
#include "InteractionSystem/InteractionPredictionState.h"
#include "Interface/InteractionSystem/InteractableInterface.h"
#include "Logging/InteractableInventoryLogging.h"
#include "Debug/DebugSubsystem.h"
#include "UI/Widget_InteractionPrompt.h"
#include "Interfaces/SimulatorFramework/PhysicalInteractionInterface.h"
#include "InteractionSystem/InteractionDefaultsConstants.h"

#include "InteractorComponent.generated.h"

class UDebugSubsystem;
class UInteractionSubsystem;
class UPhysicsGrabComponent;

UCLASS(ClassGroup=(Components), meta=(BlueprintSpawnableComponent))
class MODULARINTERACTIONSYSTEM_API UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractorComponent();

	// Delegate function to get pawn on posses
	APawn* OnPossess(APawn* InPawn);
	
	// Interaction - Client - Timer to get camera focused interactable
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FTimerHandle FocusUpdateTimerHandle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Performance")
	float FocusUpdateInterval = InteractionDefaultsConstants::GlobalInteractionFocusUpdateRate;
	
	// Interaction - Client callable
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void TryInteract();
	
	// Server RPC - Called from TryInteract on clients
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact(AActor* ClientFocusedActor);
	UFUNCTION(Client, Reliable)
	//Client side only interaction (has authority locally)
	void Client_PerformInteraction(AActor* Actor);
	UFUNCTION(NetMulticast, Reliable)
	/*Cast to both server and all clients including ai
	*Role is stricted to ROLE_Simulated proxy
	* So,updates for every controller except the local machines.
	* ie sv, non-local clients, ai
	*/
	void Multicast_PerformInteraction(AActor* Actor);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HideWidgetsForActor(AActor* Actor);

	//Client only widget show and hide
	UFUNCTION(Client, Unreliable)
	void Client_SetWidgetVisibility(UWidget* Widget, const ESlateVisibility Visibility = ESlateVisibility::Visible);
	UFUNCTION(Client, UnReliable)
	void Client_DestroyWidget(UWidget* Widget);
	



	UFUNCTION(BlueprintCallable, Category="Interaction")
	void UpdateFocus();

	UFUNCTION(BlueprintCallable, Category="Interaction")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	// Interaction Widget
	//Widgets
	UPROPERTY()
	UUserWidget* CurrentInteractionPrompt;
   
	UPROPERTY()
	UUserWidget* CurrentPreInteractionPrompt;
	
	UFUNCTION(BlueprintCallable, Client, Reliable, Category="Interaction")
	void Client_ShowInteractionPrompt(AActor* Interactable);
	
	UFUNCTION(BlueprintCallable, Client, Reliable, Category="Interaction")
	void Client_HideInteractionPrompt(AActor* Interactable);

	UFUNCTION(BlueprintCallable, Client, Reliable, Category="Interaction")
	void Client_ShowPreInteractionPrompt(AActor* Interactable);
	
	UFUNCTION(BlueprintCallable, Client, Reliable, Category="Interaction")
	void Client_HidePreInteractionPrompt(AActor* Interactable);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText InteractionText = INVTEXT("Interact");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Widgets")
	TSubclassOf<UUserWidget> Widget_InteractionPromptClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	TSoftClassPtr<UUserWidget> Widget_InteractionPromptClass_Soft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	TSubclassOf<UUserWidget> Widget_PreInteractionPromptClass;

	UPROPERTY(BlueprintReadWrite, Category="Interaction")
	bool bCanInteract = true;

	// Sphere Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Interaction")
	USphereComponent* PreInteractionSphereComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Interaction")
	USphereComponent* InteractionSphereComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Interaction")
	USphereComponent* PreInteractionComponentUI;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Interaction")
	USphereComponent* InteractionComponentUI;

	// Overlap Tracking
	UPROPERTY()
	TSet<AActor*> OverlappingInteractables;
	
	UPROPERTY()
	TSet<AActor*> PreInteractionUIOverlappingActors;  // ✅ This is correct
	
	UPROPERTY()
	TSet<AActor*> InteractionUIOverlappingActors;
	
	// Overlap Arrays
	TArray<AActor*> GetOverlappingInteractables() const;
	TArray<AActor*> GetPreInteractionUIOverlappingActors() const { return PreInteractionUIOverlappingActors.Array(); }
	TArray<AActor*> GetInteractionUIOverlappingActors() const { return InteractionUIOverlappingActors.Array(); }

	//LifeCycle
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ========== CLIENT PREDICTION ==========
    
	/** Enable/disable client prediction */
	UPROPERTY(EditAnywhere, Category = "Interaction|Prediction")
	bool bEnableClientPrediction = true;
    
	/** Tracks pending predictions awaiting server confirmation */
	UPROPERTY()
	TMap<int32, FPredictedInteraction> PendingPredictions;
    
	/** Next prediction ID to assign */
	int32 NextPredictionID = 0;
    
	/** Timeout for predictions (auto-rollback after this) */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Prediction")
	float PredictionTimeout = 2.0f;
    
	/** Timer handle for prediction cleanup */
	FTimerHandle PredictionCleanupTimerHandle;
    
	// ========== PREDICTION FUNCTIONS ==========
    
	/** Execute visual effects for predicted interaction */
	void ExecutePredictedEffects(AActor* Target, int32 PredictionID);
    
	/** Validate server-side target and execute predicted interaction */
	void ValidateAndExecutePredictedInteraction(AActor* ServerFoundTarget, int32 PredictionID);
    
	/** Server confirms prediction */
	UFUNCTION(Client, Reliable)
	void Client_ConfirmPrediction(int32 PredictionID);
    
	/** Server rejects prediction */
	UFUNCTION(Client, Reliable)
	void Client_RejectPrediction(int32 PredictionID);
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	APawn* OwnerPawn;
	
	/** Owner's player controller (cached for performance) */
	UPROPERTY()
	APlayerController* OwnerPlayerController;
	
	/** Timer to retry getting player controller if not ready at BeginPlay */
	FTimerHandle ControllerCheckTimer;
	
	/** Retry getting player controller (called by timer) */
	void TryGetPlayerController();
	
	/** Try to grab instead of interact if applicable */
	bool TryGrabFocusedActor();
	/** Check if actor should be grabbed instead of interacted */
	bool ShouldGrab(AActor* Actor) const;

	UPROPERTY()
	TWeakObjectPtr<UPhysicsGrabComponent> PhysicsGrabComponent;


private:

	bool Server_Interact_Validate(AActor* ClientFocusedActor);
	bool Server_Interact_Predicted_Validate(AActor* ClientFocusedActor,
		FVector_NetQuantize10 ClientViewLocation,
		int32 PredictionID);

	/** Rollback failed prediction */
	void RollbackPrediction(int32 PredictionID);
    
	/** Cleanup timed-out predictions */
	void CleanupTimedOutPredictions();

	// Subsystems
	UPROPERTY()
	UInteractionSubsystem* InteractionSubsystem;
	
	UPROPERTY()
	UDebugSubsystem* DebugSubsystem;

	// UPROPERTY()
	// UWidgetSubsystem* WidgetSubsystem;

	// Debug
	FGameplayTag DebugTag_Interaction;
	UDebugSubsystem* GetDebugSubsystem();

	// Focus
	TWeakObjectPtr<AActor> FocusedActor;
	void SetFocusedActor(AActor* NewFocus);

	// Actual interaction logic - called by both client and server
	//void PerformInteraction();

	// Setup
	void SetupInteractionSphereComponent(
		USphereComponent* SphereComponent,
		float Radius,
		const FGameplayTag& DebugTag,
		bool bCheckForInteractables = true,
		bool bIsforUI = false
		//Removed Color: DebugDataAsset in DebugSubsystem will handle it!
		//FColor Color = DebugDefaultsConstants::DEBUG_LAYER_INTERACTION_LINEARCOLOR.ToFColor(true)
		);
	
	//All widgets should be set up here for creating them in constructor.
	//Hove to call it from constructor
	/**
	 * Helper to load widget class in constructor
	 * @param WidgetPath - Path to the widget blueprint
	 * @return Loaded widget class or nullptr
	 */
	TSubclassOf<UUserWidget> LoadWidgetClass(const TCHAR* WidgetPath);
	// Overlap Events - Loading Spheres

	UFUNCTION()
	void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Overlap Events - UI Spheres
	UFUNCTION()
	void OnPreInteractionComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnFullInteractionComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepHitResult);
	
	UFUNCTION()
	void OnPreInteractionComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void OnFullInteractionComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	/** Server RPC with prediction tracking */
	
	/** Server RPC with prediction tracking */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact_Predicted(AActor* ClientFocusedActor,
		FVector_NetQuantize10 ClientViewLocation,
		int32 PredictionID);


	void DebugOverlapSphereCheck(UPrimitiveComponent* SphereCollisionComponent);

	//just for debug overlap sphere check function delete this later
	FTimerHandle TH_DelayedDebugTimer;

};