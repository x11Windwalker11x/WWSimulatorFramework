#include "MasterSaveGame.h"

UMasterSaveGame::UMasterSaveGame()
    : SaveSlotName(TEXT("DefaultSlot"))
    , MasterSaveVersion(CURRENT_MASTER_VERSION)
    , PlayTimeSeconds(0.0f)
{
    UpdateTimestamp();
}

bool UMasterSaveGame::AddOrUpdateModule(UModularSaveData* ModuleData)
{
    if (!ModuleData)
    {
        return false;
    }

    FString ClassName = ModuleData->GetClass()->GetName();
    ModuleData->UpdateTimestamp();
    SaveModules.Add(ClassName, ModuleData);
    
    return true;
}

UModularSaveData* UMasterSaveGame::GetModuleByClass(TSubclassOf<UModularSaveData> ModuleClass) const
{
    if (!ModuleClass)
    {
        return nullptr;
    }

    FString ClassName = ModuleClass->GetName();
    if (UModularSaveData* const* FoundModule = SaveModules.Find(ClassName))
    {
        return *FoundModule;
    }

    return nullptr;
}

bool UMasterSaveGame::HasModule(TSubclassOf<UModularSaveData> ModuleClass) const
{
    if (!ModuleClass)
    {
        return false;
    }

    FString ClassName = ModuleClass->GetName();
    return SaveModules.Contains(ClassName);
}

bool UMasterSaveGame::RemoveModule(TSubclassOf<UModularSaveData> ModuleClass)
{
    if (!ModuleClass)
    {
        return false;
    }

    FString ClassName = ModuleClass->GetName();
    return SaveModules.Remove(ClassName) > 0;
}

TArray<UModularSaveData*> UMasterSaveGame::GetAllModules() const
{
    TArray<UModularSaveData*> Modules;
    SaveModules.GenerateValueArray(Modules);
    return Modules;
}

UModularSaveData* UMasterSaveGame::CreateAndAddModule(TSubclassOf<UModularSaveData> ModuleClass)
{
    if (!ModuleClass)
    {
        return nullptr;
    }

    UModularSaveData* NewModule = NewObject<UModularSaveData>(this, ModuleClass);
    if (NewModule)
    {
        AddOrUpdateModule(NewModule);
    }

    return NewModule;
}

void UMasterSaveGame::UpdateTimestamp()
{
    SaveTimestamp = FDateTime::Now();
}

void UMasterSaveGame::ClearAllModules()
{
    for (auto& Pair : SaveModules)
    {
        if (Pair.Value)
        {
            Pair.Value->ClearData();
        }
    }
    SaveModules.Empty();
}

bool UMasterSaveGame::ValidateAllModules()
{
    bool bAllValid = true;
    
    for (auto& Pair : SaveModules)
    {
        if (Pair.Value)
        {
            if (!Pair.Value->ValidateData())
            {
                bAllValid = false;
            }
        }
    }

    return bAllValid;
}

void UMasterSaveGame::MigrateAllModules()
{
    for (auto& Pair : SaveModules)
    {
        if (Pair.Value)
        {
            if (Pair.Value->ModuleVersion < CURRENT_MASTER_VERSION)
            {
                Pair.Value->MigrateData(Pair.Value->ModuleVersion, CURRENT_MASTER_VERSION);
                Pair.Value->ModuleVersion = CURRENT_MASTER_VERSION;
            }
        }
    }
}

int32 UMasterSaveGame::GetEstimatedSizeBytes() const
{
    int32 EstimatedSize = sizeof(UMasterSaveGame);
    EstimatedSize += SaveModules.Num() * 1024;
    return EstimatedSize;
}

FString UMasterSaveGame::GetModuleKey(TSubclassOf<UModularSaveData> ModuleClass)
{
    return ModuleClass ? ModuleClass->GetName() : TEXT("");
}