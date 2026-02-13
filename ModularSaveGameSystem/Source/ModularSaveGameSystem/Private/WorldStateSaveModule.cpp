// Copyright Windwalker Productions. All Rights Reserved.

#include "WorldStateSaveModule.h"

UWorldStateSaveModule::UWorldStateSaveModule()
{
	ModuleName = TEXT("WorldStateSaveModule");
	ModuleVersion = CURRENT_MODULE_VERSION;
}

void UWorldStateSaveModule::SaveActorState(const FActorSaveEnvelope& Envelope)
{
	if (!Envelope.IsValid())
	{
		return;
	}

	FActorSaveEnvelopeMap& LevelMap = WorldActors.FindOrAdd(Envelope.LevelName);
	LevelMap.Actors.Add(Envelope.ActorSaveID, Envelope);
	UpdateTimestamp();
}

FActorSaveEnvelope UWorldStateSaveModule::LoadActorState(const FString& LevelName, const FString& ActorSaveID) const
{
	const FActorSaveEnvelopeMap* LevelMap = WorldActors.Find(LevelName);
	if (!LevelMap)
	{
		return FActorSaveEnvelope();
	}

	const FActorSaveEnvelope* Envelope = LevelMap->Actors.Find(ActorSaveID);
	if (!Envelope)
	{
		return FActorSaveEnvelope();
	}

	return *Envelope;
}

TArray<FActorSaveEnvelope> UWorldStateSaveModule::GetAllActorsForLevel(const FString& LevelName) const
{
	TArray<FActorSaveEnvelope> Result;

	const FActorSaveEnvelopeMap* LevelMap = WorldActors.Find(LevelName);
	if (LevelMap)
	{
		LevelMap->Actors.GenerateValueArray(Result);
	}

	return Result;
}

bool UWorldStateSaveModule::RemoveActorState(const FString& LevelName, const FString& ActorSaveID)
{
	FActorSaveEnvelopeMap* LevelMap = WorldActors.Find(LevelName);
	if (!LevelMap)
	{
		return false;
	}

	const int32 Removed = LevelMap->Actors.Remove(ActorSaveID);
	if (Removed > 0)
	{
		// Clean up empty level entries
		if (LevelMap->Actors.Num() == 0)
		{
			WorldActors.Remove(LevelName);
		}
		UpdateTimestamp();
		return true;
	}

	return false;
}

bool UWorldStateSaveModule::HasActorState(const FString& LevelName, const FString& ActorSaveID) const
{
	const FActorSaveEnvelopeMap* LevelMap = WorldActors.Find(LevelName);
	if (!LevelMap)
	{
		return false;
	}

	return LevelMap->Actors.Contains(ActorSaveID);
}

void UWorldStateSaveModule::ClearLevel(const FString& LevelName)
{
	if (WorldActors.Remove(LevelName) > 0)
	{
		UpdateTimestamp();
	}
}

TArray<FString> UWorldStateSaveModule::GetSavedLevelNames() const
{
	TArray<FString> Result;
	WorldActors.GetKeys(Result);
	return Result;
}

void UWorldStateSaveModule::ClearData()
{
	WorldActors.Empty();
	UpdateTimestamp();
}

bool UWorldStateSaveModule::ValidateData_Implementation()
{
	// Remove any invalid envelopes
	for (auto& LevelPair : WorldActors)
	{
		TArray<FString> InvalidKeys;
		for (const auto& ActorPair : LevelPair.Value.Actors)
		{
			if (!ActorPair.Value.IsValid())
			{
				InvalidKeys.Add(ActorPair.Key);
			}
		}
		for (const FString& Key : InvalidKeys)
		{
			LevelPair.Value.Actors.Remove(Key);
		}
	}

	return true;
}

void UWorldStateSaveModule::MigrateData_Implementation(int32 FromVersion, int32 ToVersion)
{
	// Future: handle version migration
}
