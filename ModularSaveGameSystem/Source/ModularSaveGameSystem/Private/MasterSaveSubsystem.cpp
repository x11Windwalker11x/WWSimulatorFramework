#include "MasterSaveSubsystem.h"

#include "UserSettingsSaveModule.h"
#include "Kismet/GameplayStatics.h"

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