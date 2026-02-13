#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MasterSaveGame.h"
#include "InventorySaveModule.h"
#include "AbilitiesSaveModule.h"
#include "CharacterSaveModule.h"
#include "UserSettingsSaveModule.h"
#include "WorldStateSaveModule.h"
#include "Delegates/ModularSaveGameSystem/SaveDelegates.h"
#include "MasterSaveSubsystem.generated.h"

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMasterSaveComplete, bool, bSuccess, const FString&, SaveSlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMasterLoadComplete, bool, bSuccess, const FString&, SaveSlotName);

/**
 * Subsystem that manages the master save game with modular components
 * This replaces the previous InventorySaveSubsystem with a more flexible approach
 */
UCLASS(BlueprintType)
class MODULARSAVEGAMESYSTEM_API UMasterSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== Core Save/Load Operations ==========
	
	/**
	 * Save the master save game (includes all modules)
	 * @param SaveSlotName - Name of the save slot
	 * @param bAsync - Whether to save asynchronously
	 * @return true if save initiated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	bool SaveGame(const FString& SaveSlotName = TEXT(""), bool bAsync = false);

	/**
	 * Load the master save game (includes all modules)
	 * @param SaveSlotName - Name of the save slot to load
	 * @return true if load successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	bool LoadGame(const FString& SaveSlotName = TEXT(""));

	/**
	 * Quick save using current cached data
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	bool QuickSave(bool bAsync = false);

	/**
	 * Quick load from the most recent save
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	bool QuickLoad();

	/**
	 * Create a new master save game with all modules initialized
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	UMasterSaveGame* CreateNewMasterSave(const FString& SaveSlotName = TEXT(""));

	// ========== Module Access ==========
	
	/**
	 * Get or create a specific save module (C++ only - type-safe)
	 */
	template<typename T>
	T* GetOrCreateModule()
	{
		static_assert(TIsDerivedFrom<T, UModularSaveData>::IsDerived, "T must derive from UModularSaveData");
		
		if (!CurrentSaveGame)
		{
			CurrentSaveGame = CreateNewMasterSave();
		}

		T* Module = CurrentSaveGame->GetModule<T>();
		if (!Module)
		{
			Module = NewObject<T>(CurrentSaveGame);
			CurrentSaveGame->AddOrUpdateModule(Module);
		}

		return Module;
	}

	/**
	 * Get or create a specific save module (Blueprint friendly)
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save", meta = (DeterminesOutputType = "ModuleClass"))
	UModularSaveData* GetOrCreateModuleByClass(TSubclassOf<UModularSaveData> ModuleClass);

	/**
	 * Get the current master save game
	 */
	UFUNCTION(BlueprintPure, Category = "Master Save")
	UMasterSaveGame* GetCurrentSaveGame() const { return CurrentSaveGame; }

	// ========== Convenience Functions for Common Modules ==========
	
	/**
	 * Get inventory module (convenience function)
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	UInventorySaveModule* GetInventoryModule();

	/**
	 * Get abilities module (convenience function)
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	UAbilitiesSaveModule* GetAbilitiesModule();

	/**
	 * Get character module (convenience function)
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	UCharacterSaveModule* GetCharacterModule();

	// ========== Utility Operations ==========
	
	/**
	 * Check if a save file exists
	 */
	UFUNCTION(BlueprintPure, Category = "Master Save")
	bool DoesSaveExist(const FString& SaveSlotName) const;

	/**
	 * Delete a save file
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	bool DeleteSave(const FString& SaveSlotName);

	/**
	 * Get all available save slot names
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	TArray<FString> GetAllSaveSlots() const;

	/**
	 * Get save file metadata
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	bool GetSaveMetadata(const FString& SaveSlotName, FDateTime& OutTimestamp, 
		int32& OutVersion, FString& OutDescription) const;

	/**
	 * Generate a unique player ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	static FString GeneratePlayerID(int32 PlayerIndex = 0);

	/**
	 * Set the current save slot name (for autosaves)
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	void SetCurrentSaveSlot(const FString& SaveSlotName);

	/**
	 * Get the current save slot name
	 */
	UFUNCTION(BlueprintPure, Category = "Master Save")
	FString GetCurrentSaveSlot() const { return CurrentSaveSlotName; }

	// ========== Save Game Metadata ==========
	
	/**
	 * Update save game metadata (description, level, playtime)
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	void UpdateSaveMetadata(const FString& Description, const FString& LevelName, float PlayTime);

	// ========== World State Save/Load ==========

	/** Save all dirty saveable actors/components in the specified level */
	UFUNCTION(BlueprintCallable, Category = "Master Save|World State")
	bool SaveWorldState(const FString& LevelName);

	/** Load and restore all saved actor/component states for the specified level */
	UFUNCTION(BlueprintCallable, Category = "Master Save|World State")
	bool LoadWorldState(const FString& LevelName);

	// ========== Events ==========

	UPROPERTY(BlueprintAssignable, Category = "Master Save")
	FOnMasterSaveComplete OnSaveComplete;

	UPROPERTY(BlueprintAssignable, Category = "Master Save")
	FOnMasterLoadComplete OnLoadComplete;

	UPROPERTY(BlueprintAssignable, Category = "Master Save|World State")
	FOnWorldStateLoaded OnWorldStateLoaded;

protected:
	// Internal save/load operations
	bool SaveGameToSlot(UMasterSaveGame* SaveGame, const FString& SaveSlotName, bool bAsync);
	UMasterSaveGame* LoadGameFromSlot(const FString& SaveSlotName);

	// Async callbacks
	void OnAsyncSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess);
	void OnAsyncLoadComplete(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedSave);

private:
	// Current active save game
	UPROPERTY()
	UMasterSaveGame* CurrentSaveGame;

	// Current save slot name for quick save/load
	FString CurrentSaveSlotName;

	// Default save slot name
	static const FString DEFAULT_SAVE_SLOT;

	// User index for save system (always 0 for local saves)
	static const int32 USER_INDEX = 0;

	// Helper to get save slot name
	FString GetSaveSlotName(const FString& ProvidedName) const;

	UPROPERTY(BlueprintCallable, Category = "Master Save")
	UUserSettingsSaveModule* GetUserSettingsModule();

	/**
	 * Get world state module (convenience function)
	 */
	UFUNCTION(BlueprintCallable, Category = "Master Save")
	UWorldStateSaveModule* GetWorldStateModule();
};