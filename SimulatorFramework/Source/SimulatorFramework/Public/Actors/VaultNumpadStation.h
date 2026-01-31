// VaultNumpadStation.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SimulatorFramework/MiniGameStationInterface.h"
#include "Lib/Data/SimulatorFramework/MiniGameData.h"
#include "VaultNumpadStation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVaultStateChanged, bool, bIsOpen);

/**
 * Example vault actor with numpad mini-game
 * Demonstrates IMiniGameStationInterface implementation
 */
UCLASS(BlueprintType, Blueprintable)
class SIMULATORFRAMEWORK_API AVaultNumpadStation : public AActor, public IMiniGameStationInterface
{
    GENERATED_BODY()

public:
    AVaultNumpadStation();

    // === IMiniGameStationInterface ===

    virtual FGameplayTag GetMiniGameID_Implementation() const override;
    virtual bool GetMiniGameConfig_Implementation(FMiniGameData& OutConfig) const override;
    virtual void OnMiniGameStarted_Implementation(const FGameplayTag& MiniGameID) override;
    virtual void OnMiniGameEnded_Implementation(const FGameplayTag& MiniGameID, bool bSuccess, bool bBonus) override;
    virtual TArray<FSnapPointData> GetSnapPoints_Implementation() const override;
    virtual TArray<FGameplayTag> GetStoredCode_Implementation() const override;
    virtual float GetDifficultyModifier_Implementation() const override;
    virtual bool IsMiniGameAvailable_Implementation() const override;

    // === VAULT STATE ===

    /** Check if vault is open */
    UFUNCTION(BlueprintPure, Category = "Vault")
    bool IsVaultOpen() const { return bIsOpen; }

    /** Get remaining attempts */
    UFUNCTION(BlueprintPure, Category = "Vault")
    int32 GetRemainingAttempts() const { return RemainingAttempts; }

    /** Manually set vault code (runtime) */
    UFUNCTION(BlueprintCallable, Category = "Vault")
    void SetVaultCode(const TArray<FGameplayTag>& NewCode);

    /** Lock the vault */
    UFUNCTION(BlueprintCallable, Category = "Vault")
    void LockVault();

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "Vault|Events")
    FOnVaultStateChanged OnVaultStateChanged;

protected:
    virtual void BeginPlay() override;

    // === CONFIG ===

    /** MiniGame ID to use */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Config")
    FGameplayTag MiniGameID;

    /** Camera mode for numpad interaction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Config")
    FGameplayTag CameraModeTag;

    /** The vault code (sequence of Input.Numpad.* tags) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Config")
    TArray<FGameplayTag> VaultCode;

    /** Maximum failed attempts before lockout (0 = unlimited) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Config")
    int32 MaxAttempts = 3;

    /** Lockout duration in seconds (0 = permanent until reset) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Config")
    float LockoutDuration = 0.0f;

    /** Difficulty (0 = hardest, 1 = easiest) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Difficulty = 0.5f;

    // === STATE ===

    UPROPERTY(BlueprintReadOnly, Category = "Vault|State")
    bool bIsOpen = false;

    UPROPERTY(BlueprintReadOnly, Category = "Vault|State")
    bool bIsLockedOut = false;

    UPROPERTY(BlueprintReadOnly, Category = "Vault|State")
    int32 RemainingAttempts = 3;

    UPROPERTY(BlueprintReadOnly, Category = "Vault|State")
    float LockoutTimeRemaining = 0.0f;

    // === INTERNAL ===

    /** Open the vault */
    void OpenVault();

    /** Handle lockout */
    void TriggerLockout();

    /** Clear lockout */
    void ClearLockout();

    /** Timer handle for lockout */
    FTimerHandle LockoutTimerHandle;
};