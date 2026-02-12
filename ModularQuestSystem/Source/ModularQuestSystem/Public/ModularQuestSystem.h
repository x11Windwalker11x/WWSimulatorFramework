// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * Modular Quest System Module
 *
 * Quest lifecycle management for tycoon gameplay:
 * - Quest tracking and progression via ObjectiveTrackerSubsystem
 * - Per-player quest log with replication
 * - Quest chains and prerequisites
 * - EventBus integration for cross-plugin reward distribution
 */
class FModularQuestSystemModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
