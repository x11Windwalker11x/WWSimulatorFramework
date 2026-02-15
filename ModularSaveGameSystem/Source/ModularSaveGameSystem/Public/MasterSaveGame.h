#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/ModularSaveGameSystem/SaveData.h"
#include "GameFramework/SaveGame.h"
#include "MasterSaveGame.generated.h"

/**
 * Base class for modular save data components
 * Each specialized save system (inventory, abilities, etc.) inherits from this
 */
UCLASS(Abstract, Blueprintable)
class MODULARSAVEGAMESYSTEM_API UModularSaveData : public UObject
{
	GENERATED_BODY()

public:
	// Save version for this specific module
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save Info")
	int32 ModuleVersion;

	// Module identifier
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save Info")
	FString ModuleName;

	// Timestamp for this module's last update
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save Info")
	FDateTime LastModified;

	UModularSaveData()
		: ModuleVersion(1)
		, ModuleName(TEXT("BaseModule"))
	{
		UpdateTimestamp();
	}

	// Override in derived classes to handle version migration
	UFUNCTION(BlueprintNativeEvent, Category = "Save Data")
	void MigrateData(int32 FromVersion, int32 ToVersion);
	virtual void MigrateData_Implementation(int32 FromVersion, int32 ToVersion) {}

	// Override to validate data after loading
	UFUNCTION(BlueprintNativeEvent, Category = "Save Data")
	bool ValidateData();
	virtual bool ValidateData_Implementation() { return true; }

	// Override to clear all data
	UFUNCTION(BlueprintCallable, Category = "Save Data")
	virtual void ClearData() {}

	void UpdateTimestamp() { LastModified = FDateTime::Now(); }
};

/**
 * Master SaveGame object that contains all modular save components
 * This is the single .sav file that gets saved/loaded
 */
UCLASS(BlueprintType)
class MODULARSAVEGAMESYSTEM_API UMasterSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UMasterSaveGame();

	// ========== Core Save Info ==========
	
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save Info")
	FString SaveSlotName;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save Info")
	FDateTime SaveTimestamp;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save Info")
	int32 MasterSaveVersion;

	// Optional: Screenshot or save description
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save Info")
	FString SaveDescription;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save Info")
	FString LevelName;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Save Info")
	float PlayTimeSeconds;

	// ========== Modular Save Components ==========
	
	// Store all save modules by class name for type-safe retrieval
	UPROPERTY(SaveGame)
	TMap<FString, UModularSaveData*> SaveModules;

	/** Subsystem-type saveable records (global, not per-level) */
	UPROPERTY(SaveGame)
	TMap<FString, FSaveRecord> SubsystemSaveRecords;

	// ========== Module Management ==========
	
	/**
	 * Add or update a save module
	 * @param ModuleData - The module to add/update
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	bool AddOrUpdateModule(UModularSaveData* ModuleData);

	/**
	 * Get a save module by class type (C++ only)
	 * @return The module if found, nullptr otherwise
	 */
	template<typename T>
	T* GetModule() const
	{
		static_assert(TIsDerivedFrom<T, UModularSaveData>::IsDerived, "T must derive from UModularSaveData");
		
		FString ClassName = T::StaticClass()->GetName();
		if (UModularSaveData* const* FoundModule = SaveModules.Find(ClassName))
		{
			return Cast<T>(*FoundModule);
		}
		return nullptr;
	}

	/**
	 * Get a save module by class (Blueprint friendly)
	 * @param ModuleClass - The class of the module to retrieve
	 * @return The module if found, nullptr otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save", meta = (DeterminesOutputType = "ModuleClass"))
	UModularSaveData* GetModuleByClass(TSubclassOf<UModularSaveData> ModuleClass) const;

	/**
	 * Check if a module exists
	 */
	UFUNCTION(BlueprintPure, Category = "Master Save")
	bool HasModule(TSubclassOf<UModularSaveData> ModuleClass) const;

	/**
	 * Remove a module
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	bool RemoveModule(TSubclassOf<UModularSaveData> ModuleClass);

	/**
	 * Get all modules of a specific type
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	TArray<UModularSaveData*> GetAllModules() const;

	/**
	 * Create and add a new module
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	UModularSaveData* CreateAndAddModule(TSubclassOf<UModularSaveData> ModuleClass);

	// ========== Utility Functions ==========
	
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	void UpdateTimestamp();

	UFUNCTION(BlueprintCallable, Category = "Master Save")
	void ClearAllModules();

	UFUNCTION(BlueprintPure, Category = "Master Save")
	int32 GetModuleCount() const { return SaveModules.Num(); }

	// Validate all modules
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	bool ValidateAllModules();

	// Migrate all modules if needed
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	void MigrateAllModules();

	// Get save file size estimate (in bytes)
	UFUNCTION(BlueprintPure, Category = "Master Save")
	int32 GetEstimatedSizeBytes() const;

private:
	static const int32 CURRENT_MASTER_VERSION = 1;

	// Helper to get class name key
	static FString GetModuleKey(TSubclassOf<UModularSaveData> ModuleClass);
};