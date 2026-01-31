#pragma once

#include "CoreMinimal.h"
#include "MasterSaveGame.h"
#include "CharacterSaveModule.generated.h"

/**
 * Character transform and state data
 */
USTRUCT(BlueprintType)
struct FCharacterTransformData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FString CurrentLevelName;

	FCharacterTransformData()
		: Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, Scale(FVector::OneVector)
		, CurrentLevelName(TEXT(""))
	{}

	FTransform ToTransform() const
	{
		return FTransform(Rotation, Location, Scale);
	}

	void FromTransform(const FTransform& Transform)
	{
		Location = Transform.GetLocation();
		Rotation = Transform.GetRotation().Rotator();
		Scale = Transform.GetScale3D();
	}
};

/**
 * Character stats data
 */
USTRUCT(BlueprintType)
struct FCharacterStatsData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float Health = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float Stamina = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float MaxStamina = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float Mana = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	float MaxMana = 100.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 Level = 1;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 Experience = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	int32 Currency = 0;

	FCharacterStatsData()
		: Health(100.0f)
		, MaxHealth(100.0f)
		, Stamina(100.0f)
		, MaxStamina(100.0f)
		, Mana(100.0f)
		, MaxMana(100.0f)
		, Level(1)
		, Experience(0)
		, Currency(0)
	{}
};

/**
 * Complete character save data
 */
USTRUCT(BlueprintType)
struct FPlayerCharacterData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FString CharacterName;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FCharacterTransformData Transform;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FCharacterStatsData Stats;

	// Optional: Equipment worn
	UPROPERTY(SaveGame, BlueprintReadWrite)
	TMap<FName, FName> EquippedItems; // Slot -> ItemID

	// Optional: Active status effects
	UPROPERTY(SaveGame, BlueprintReadWrite)
	TArray<FName> ActiveStatusEffects;

	// Optional: Quest progression
	UPROPERTY(SaveGame, BlueprintReadWrite)
	TMap<FName, int32> QuestProgress; // QuestID -> Progress

	FPlayerCharacterData()
		: PlayerID(TEXT(""))
		, CharacterName(TEXT(""))
	{}
};

/**
 * Modular save data component for character transform and state
 */
UCLASS(BlueprintType)
class MODULARSAVEGAMESYSTEM_API UCharacterSaveModule : public UModularSaveData
{
	GENERATED_BODY()

public:
	UCharacterSaveModule();

	// ========== Character Data ==========
	
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Character")
	TMap<FString, FPlayerCharacterData> PlayerCharacters;

	// ========== Utility Functions ==========
	
	UFUNCTION(BlueprintCallable, Category = "Character Save")
	FPlayerCharacterData& GetOrCreatePlayerCharacter(const FString& PlayerID);

	UFUNCTION(BlueprintPure, Category = "Character Save")
	bool HasPlayerData(const FString& PlayerID) const;

	UFUNCTION(BlueprintCallable, Category = "Character Save")
	bool RemovePlayerData(const FString& PlayerID);

	UFUNCTION(BlueprintPure, Category = "Character Save")
	TArray<FString> GetAllPlayerIDs() const;

	// Convenience functions for quick access
	UFUNCTION(BlueprintCallable, Category = "Character Save")
	bool SaveCharacterTransform(const FString& PlayerID, const FTransform& Transform, const FString& LevelName);

	UFUNCTION(BlueprintCallable, Category = "Character Save")
	bool LoadCharacterTransform(const FString& PlayerID, FTransform& OutTransform, FString& OutLevelName);

	UFUNCTION(BlueprintCallable, Category = "Character Save")
	bool SaveCharacterStats(const FString& PlayerID, const FCharacterStatsData& Stats);

	UFUNCTION(BlueprintCallable, Category = "Character Save")
	bool LoadCharacterStats(const FString& PlayerID, FCharacterStatsData& OutStats);

	// ========== UModularSaveData Overrides ==========
	
	virtual void ClearData() override;
	virtual bool ValidateData_Implementation() override;
	virtual void MigrateData_Implementation(int32 FromVersion, int32 ToVersion) override;

private:
	static const int32 CURRENT_MODULE_VERSION = 1;
};