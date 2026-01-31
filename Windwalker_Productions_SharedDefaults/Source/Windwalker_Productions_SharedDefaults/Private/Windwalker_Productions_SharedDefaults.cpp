// Copyright Epic Games, Inc. All Rights Reserved.

#include "Windwalker_Productions_SharedDefaults.h"
#include "Logging/InteractableInventoryLogging.h"

#define LOCTEXT_NAMESPACE "FWindwalker_Productions_SharedDefaultsModule"

void FWindwalker_Productions_SharedDefaultsModule::StartupModule()
{
    UE_LOG(LogDebugSystem, Log, TEXT("Windwalker Productions SharedDefaults module started."));
}

void FWindwalker_Productions_SharedDefaultsModule::ShutdownModule()
{
    UE_LOG(LogDebugSystem, Log, TEXT("Windwalker Productions SharedDefaults module shutdown."));
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FWindwalker_Productions_SharedDefaultsModule, Windwalker_Productions_SharedDefaults)