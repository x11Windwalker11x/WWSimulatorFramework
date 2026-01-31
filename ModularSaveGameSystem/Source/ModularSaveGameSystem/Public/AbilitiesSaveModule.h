#pragma once

#include "CoreMinimal.h"
#include "MasterSaveGame.h"
#include "GameplayTagContainer.h"
#include "AbilitiesSaveModule.generated.h"

/**
 * Saved ability data
 */
USTRUCT(BlueprintType)
struct FAbilitySaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FName AbilityID;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 Level = 1;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float Experience = 0.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	bool bIsUnlocked = false;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	bool bIsEquipped = false;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 HotbarSlot = -1;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FGameplayTagContainer AbilityTags;

	// Custom ability data (cooldowns, charges, etc.)
	UPROPERTY(SaveGame, BlueprintReadWrite)
	TMap<FName, float> CustomFloatData;

	FAbilitySaveData()
		: AbilityID(NAME_None)
		, Level(1)
		, Experience(0.0f)
		, bIsUnlocked(false)
		, bIsEquipped(false)
		, HotbarSlot(-1)
	{}

	FAbilitySaveData(FName InAbilityID, int32 InLevel, bool bInUnlocked)
		: AbilityID(InAbilityID)
		, Level(InLevel)
		, Experience(0.0f)
		, bIsUnlocked(bInUnlocked)
		, bIsEquipped(false)
		, HotbarSlot(-1)
	{}
};

/**
 * Player ability loadout
 */
USTRUCT(BlueprintType)
struct FPlayerAbilitiesData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	TArray<FAbilitySaveData> Abilities;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 AvailableAbilityPoints = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 TotalAbilityPoints = 0;

	FPlayerAbilitiesData()
		: PlayerID(TEXT(""))
		, AvailableAbilityPoints(0)
		, TotalAbilityPoints(0)
	{}
};

/**
 * Modular save data component for abilities/skills system
 */
UCLASS(BlueprintType)
class MODULARSAVEGAMESYSTEM_API UAbilitiesSaveModule : public UModularSaveData
{
	GENERATED_BODY()

public:
	UAbilitiesSaveModule();

	// ========== Abilities Data ==========
	
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Abilities")
	TMap<FString, FPlayerAbilitiesData> PlayerAbilities;

	// ========== Utility Functions ==========
	
	UFUNCTION(BlueprintCallable, Category = "Abilities Save")
	FPlayerAbilitiesData& GetOrCreatePlayerAbilities(const FString& PlayerID);

	UFUNCTION(BlueprintPure, Category = "Abilities Save")
	bool HasPlayerData(const FString& PlayerID) const;

	UFUNCTION(BlueprintCallable, Category = "Abilities Save")
	bool RemovePlayerData(const FString& PlayerID);

	UFUNCTION(BlueprintPure, Category = "Abilities Save")
	int32 GetTotalUnlockedAbilities() const;

	UFUNCTION(BlueprintPure, Category = "Abilities Save")
	TArray<FString> GetAllPlayerIDs() const;

	// ========== UModularSaveData Overrides ==========
	
	virtual void ClearData() override;
	virtual bool ValidateData_Implementation() override;
	virtual void MigrateData_Implementation(int32 FromVersion, int32 ToVersion) override;

private:
	static const int32 CURRENT_MODULE_VERSION = 1;
};