// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
/**
 * Universal Spawn Manager Module
 * 
 * Provides centralized spawn management for all world actors:
 * - Pickups (items dropped in world)
 * - AI characters (enemies, NPCs)
 * - Props and interactables
 * - Projectiles and effects
 * 
 * Features:
 * - Object pooling for performance
 * - Automatic cleanup/despawn
 * - Spatial queries (find nearby actors)
 * - Lifecycle management
 */
class FModularSpawnSystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
