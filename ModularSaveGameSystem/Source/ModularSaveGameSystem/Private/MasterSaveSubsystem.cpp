#include "MasterSaveSubsystem.h"

#include "UserSettingsSaveModule.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SaveSystem/SaveableRegistrySubsystem.h"
#include "Interfaces/ModularSaveGameSystem/SaveableInterface.h"
#include "Lib/Data/Tags/WW_TagLibrary.h"
#include "Engine/World.h"
#include "EngineUtils.h"

const FString UMasterSaveSubsystem::DEFAULT_SAVE_SLOT = TEXT("DefaultSave");

void UMasterSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentSaveGame = nullptr;
    CurrentSaveSlotName = DEFAULT_SAVE_SLOT;
}

void UMasterSaveSubsystem::Deinitialize()
{
    CurrentSaveGame = nullptr;
    
    Super::Deinitialize();
}

bool UMasterSaveSubsystem::SaveGame(const FString& SaveSlotName, bool bAsync)
{
    if (!CurrentSaveGame)
    {
        return false;
    }

    FString SlotName = GetSaveSlotName(SaveSlotName);
    CurrentSaveGame->SaveSlotName = SlotName;
    CurrentSaveGame->UpdateTimestamp();

    return SaveGameToSlot(CurrentSaveGame, SlotName, bAsync);
}

bool UMasterSaveSubsystem::LoadGame(const FString& SaveSlotName)
{
    FString SlotName = GetSaveSlotName(SaveSlotName);
    
    CurrentSaveGame = LoadGameFromSlot(SlotName);
    
    if (CurrentSaveGame)
    {
        CurrentSaveSlotName = SlotName;
        CurrentSaveGame->ValidateAllModules();
        CurrentSaveGame->MigrateAllModules();
        OnLoadComplete.Broadcast(true, SlotName);
        return true;
    }

    OnLoadComplete.Broadcast(false, SlotName);
    return false;
}

bool UMasterSaveSubsystem::QuickSave(bool bAsync)
{
    return SaveGame(CurrentSaveSlotName, bAsync);
}

bool UMasterSaveSubsystem::QuickLoad()
{
    return LoadGame(CurrentSaveSlotName);
}

UMasterSaveGame* UMasterSaveSubsystem::CreateNewMasterSave(const FString& SaveSlotName)
{
    FString SlotName = GetSaveSlotName(SaveSlotName);
    
    CurrentSaveGame = Cast<UMasterSaveGame>(UGameplayStatics::CreateSaveGameObject(UMasterSaveGame::StaticClass()));
    
    if (CurrentSaveGame)
    {
        CurrentSaveGame->SaveSlotName = SlotName;
        CurrentSaveSlotName = SlotName;
        CurrentSaveGame->UpdateTimestamp();
    }

    return CurrentSaveGame;
}

UModularSaveData* UMasterSaveSubsystem::GetOrCreateModuleByClass(TSubclassOf<UModularSaveData> ModuleClass)
{
    if (!ModuleClass)
    {
        return nullptr;
    }

    if (!CurrentSaveGame)
    {
        CurrentSaveGame = CreateNewMasterSave();
    }

    UModularSaveData* Module = CurrentSaveGame->GetModuleByClass(ModuleClass);
    if (!Module)
    {
        Module = CurrentSaveGame->CreateAndAddModule(ModuleClass);
    }

    return Module;
}

UInventorySaveModule* UMasterSaveSubsystem::GetInventoryModule()
{
    return GetOrCreateModule<UInventorySaveModule>();
}

UAbilitiesSaveModule* UMasterSaveSubsystem::GetAbilitiesModule()
{
    return GetOrCreateModule<UAbilitiesSaveModule>();
}

UCharacterSaveModule* UMasterSaveSubsystem::GetCharacterModule()
{
    return GetOrCreateModule<UCharacterSaveModule>();
}

bool UMasterSaveSubsystem::DoesSaveExist(const FString& SaveSlotName) const
{
    FString SlotName = GetSaveSlotName(SaveSlotName);
    return UGameplayStatics::DoesSaveGameExist(SlotName, USER_INDEX);
}

bool UMasterSaveSubsystem::DeleteSave(const FString& SaveSlotName)
{
    FString SlotName = GetSaveSlotName(SaveSlotName);
    return UGameplayStatics::DeleteGameInSlot(SlotName, USER_INDEX);
}

TArray<FString> UMasterSaveSubsystem::GetAllSaveSlots() const
{
    TArray<FString> SaveSlots;
    return SaveSlots;
}

bool UMasterSaveSubsystem::GetSaveMetadata(const FString& SaveSlotName, FDateTime& OutTimestamp, int32& OutVersion, FString& OutDescription) const
{
    FString SlotName = GetSaveSlotName(SaveSlotName);
    
    if (USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(SlotName, USER_INDEX))
    {
        if (UMasterSaveGame* MasterSave = Cast<UMasterSaveGame>(LoadedSave))
        {
            OutTimestamp = MasterSave->SaveTimestamp;
            OutVersion = MasterSave->MasterSaveVersion;
            OutDescription = MasterSave->SaveDescription;
            return true;
        }
    }

    return false;
}

FString UMasterSaveSubsystem::GeneratePlayerID(int32 PlayerIndex)
{
    return FString::Printf(TEXT("Player_%d"), PlayerIndex);
}

void UMasterSaveSubsystem::SetCurrentSaveSlot(const FString& SaveSlotName)
{
    CurrentSaveSlotName = SaveSlotName.IsEmpty() ? DEFAULT_SAVE_SLOT : SaveSlotName;
}

void UMasterSaveSubsystem::UpdateSaveMetadata(const FString& Description, const FString& LevelName, float PlayTime)
{
    if (CurrentSaveGame)
    {
        CurrentSaveGame->SaveDescription = Description;
        CurrentSaveGame->LevelName = LevelName;
        CurrentSaveGame->PlayTimeSeconds = PlayTime;
        CurrentSaveGame->UpdateTimestamp();
    }
}

bool UMasterSaveSubsystem::SaveGameToSlot(UMasterSaveGame* SaveGame, const FString& SaveSlotName, bool bAsync)
{
    if (!SaveGame)
    {
        return false;
    }

    if (bAsync)
    {
        FAsyncSaveGameToSlotDelegate SaveDelegate;
        SaveDelegate.BindUObject(this, &UMasterSaveSubsystem::OnAsyncSaveComplete);
        UGameplayStatics::AsyncSaveGameToSlot(SaveGame, SaveSlotName, USER_INDEX, SaveDelegate);
        return true;
    }
    else
    {
        bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, USER_INDEX);
        OnSaveComplete.Broadcast(bSuccess, SaveSlotName);
        return bSuccess;
    }
}

UMasterSaveGame* UMasterSaveSubsystem::LoadGameFromSlot(const FString& SaveSlotName)
{
    if (USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(SaveSlotName, USER_INDEX))
    {
        return Cast<UMasterSaveGame>(LoadedSave);
    }

    return nullptr;
}

void UMasterSaveSubsystem::OnAsyncSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
    OnSaveComplete.Broadcast(bSuccess, SlotName);
}

void UMasterSaveSubsystem::OnAsyncLoadComplete(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedSave)
{
    CurrentSaveGame = Cast<UMasterSaveGame>(LoadedSave);
    bool bSuccess = (CurrentSaveGame != nullptr);
    
    if (bSuccess)
    {
        CurrentSaveSlotName = SlotName;
        CurrentSaveGame->ValidateAllModules();
        CurrentSaveGame->MigrateAllModules();
    }

    OnLoadComplete.Broadcast(bSuccess, SlotName);
}

FString UMasterSaveSubsystem::GetSaveSlotName(const FString& ProvidedName) const
{
    return ProvidedName.IsEmpty() ? CurrentSaveSlotName : ProvidedName;
}

UUserSettingsSaveModule* UMasterSaveSubsystem::GetUserSettingsModule()
{
    return GetOrCreateModule<UUserSettingsSaveModule>();
}

UWorldStateSaveModule* UMasterSaveSubsystem::GetWorldStateModule()
{
    return GetOrCreateModule<UWorldStateSaveModule>();
}

// ============================================================================
// WORLD STATE SAVE/LOAD
// ============================================================================

bool UMasterSaveSubsystem::SaveWorldState(const FString& LevelName)
{
    UWorldStateSaveModule* WorldModule = GetWorldStateModule();
    if (!WorldModule) return false;

    USaveableRegistrySubsystem* Registry = USaveableRegistrySubsystem::Get(this);
    if (!Registry) return false;

    // Get all dirty saveables sorted by priority
    TArray<UObject*> DirtySaveables = Registry->GetDirtySaveables();

    int32 SavedCount = 0;
    for (UObject* Saveable : DirtySaveables)
    {
        if (!Saveable || !IsValid(Saveable)) continue;

        // Only save actor-level saveables (components are saved by their owning actor's orchestrator)
        FGameplayTag SaveType = ISaveableInterface::Execute_GetSaveType(Saveable);
        if (!SaveType.MatchesTag(FWWTagLibrary::Save_Category_Actor())) continue;

        FSaveRecord Record;
        if (ISaveableInterface::Execute_SaveState(Saveable, Record))
        {
            // Build envelope from the record's binary data
            if (Record.BinaryData.Num() > 0)
            {
                FMemoryReader MemoryReader(Record.BinaryData, true);
                FActorSaveEnvelope Envelope;
                FActorSaveEnvelope::StaticStruct()->SerializeBin(MemoryReader, &Envelope);

                if (Envelope.IsValid())
                {
                    Envelope.LevelName = LevelName;
                    WorldModule->SaveActorState(Envelope);
                    SavedCount++;
                }
            }
        }
    }

    // Clear dirty flags on all saved objects
    if (SavedCount > 0)
    {
        Registry->MarkAllClean();
    }

    return SavedCount > 0;
}

bool UMasterSaveSubsystem::LoadWorldState(const FString& LevelName)
{
    UWorldStateSaveModule* WorldModule = GetWorldStateModule();
    if (!WorldModule) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    TArray<FActorSaveEnvelope> Envelopes = WorldModule->GetAllActorsForLevel(LevelName);
    if (Envelopes.Num() == 0)
    {
        OnWorldStateLoaded.Broadcast(LevelName);
        return true;
    }

    // Sort envelopes by the actor SaveType: actors with lower priority numbers load first
    int32 RestoredCount = 0;

    for (const FActorSaveEnvelope& Envelope : Envelopes)
    {
        if (!Envelope.IsValid()) continue;

        // Handle destroyed actors
        if (Envelope.bIsDestroyed)
        {
            bool bFoundAndDestroyed = false;
            // Find and destroy the actor
            for (TActorIterator<AActor> It(World); It; ++It)
            {
                if (It->GetPathName() == Envelope.ActorSaveID)
                {
                    It->Destroy();
                    RestoredCount++;
                    bFoundAndDestroyed = true;
                    break;
                }
            }
            OnActorStateRestored.Broadcast(Envelope.ActorSaveID, bFoundAndDestroyed);
            continue;
        }

        // Find the level-placed actor by path name
        bool bActorRestored = false;
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor->GetPathName() != Envelope.ActorSaveID) continue;

            if (!Actor->GetClass()->ImplementsInterface(USaveableInterface::StaticClass())) continue;

            // Serialize the envelope back to binary for the actor's LoadState
            TArray<uint8> BinaryData;
            FMemoryWriter MemoryWriter(BinaryData, true);
            FActorSaveEnvelope::StaticStruct()->SerializeBin(MemoryWriter, const_cast<FActorSaveEnvelope*>(&Envelope));

            FSaveRecord LoadRecord;
            LoadRecord.RecordID = FName(*Envelope.ActorSaveID);
            LoadRecord.RecordType = FWWTagLibrary::Save_Category_Actor();
            LoadRecord.BinaryData = MoveTemp(BinaryData);

            bActorRestored = ISaveableInterface::Execute_LoadState(Actor, LoadRecord);
            if (bActorRestored)
            {
                RestoredCount++;
            }
            break;
        }
        OnActorStateRestored.Broadcast(Envelope.ActorSaveID, bActorRestored);
    }

    OnWorldStateLoaded.Broadcast(LevelName);
    return RestoredCount > 0;
}