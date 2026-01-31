// PawnComponentManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ComponentManager.h"
#include "PawnComponentManager.generated.h"

class UInventoryComponent;
class UInteractorComponent;
class UInteractableComponent;
class UInputActionMechanicComponent;
class APlayerController;
class AAIController;
class AMPC_PlayerCameraManager;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULARPLAYERCONTROLLER_API UPawnComponentManager : public UComponentManager
{
    GENERATED_BODY()

public:
    UPawnComponentManager();

protected:
    virtual void BeginPlay() override;

public:
    //========================================
    // CACHED COMPONENTS (auto-found at BeginPlay)
    //========================================
    UFUNCTION(BlueprintCallable, Category = "Controllers")
    AMPC_PlayerCameraManager* GetOwnerCameraManager() const { return OwnerCameraManager; }
    
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UInventoryComponent* InventoryComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UInteractorComponent* InteractorComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UInteractableComponent* InteractableComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UInputActionMechanicComponent* InputActionMechanicComponent;

    //========================================
    // CACHED CONTROLLERS (set via OnPossessed)
    //========================================
    
    UPROPERTY(BlueprintReadOnly, Category = "Controllers")
    APlayerController* OwnerPlayerController;

    UPROPERTY(BlueprintReadOnly, Category = "Controllers")
    AAIController* OwnerAIController;

    UPROPERTY(BlueprintReadOnly, Category = "Controllers")
    AMPC_PlayerCameraManager* OwnerCameraManager;

    //========================================
    // SETUP (call from Pawn::PossessedBy)
    //========================================
    
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void OnPawnPossessed(AController* NewController);

    //========================================
    // GETTERS (for interface forwarding)
    //========================================

    UFUNCTION(BlueprintCallable, Category = "Components")
    virtual FGameplayTagContainer GetTagsFromComponent(const FName& CrafterID, UActorComponent* TargetComponent, const UDataTable* DataTable ) const override;

    UFUNCTION(BlueprintCallable, Category = "Components")
    UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

    UFUNCTION(BlueprintCallable, Category = "Components")
    UInteractorComponent* GetInteractorComponent() const { return InteractorComponent; }

    UFUNCTION(BlueprintCallable, Category = "Components")
    UInteractableComponent* GetInteractableComponent() const { return InteractableComponent; }

    UFUNCTION(BlueprintCallable, Category = "Controllers")
    APlayerController* GetOwnerPlayerController() const { return OwnerPlayerController; }

    UFUNCTION(BlueprintCallable, Category = "Controllers")
    AAIController* GetOwnerAIController() const { return OwnerAIController; }

private:
    void CacheComponents();
    void BindDelegates();
    void SetupCameraManager(APlayerController* PC);

    //========================================
    // CROSS-SYSTEM HANDLERS
    //========================================
    
    UFUNCTION()
    void HandleItemDropped(const FInventorySlot& DroppedSlot, FVector DropLocation);

    UFUNCTION()
    void HandleInventoryChanged();
};