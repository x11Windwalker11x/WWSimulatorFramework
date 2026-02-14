// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * Weather Time Manager Plugin Module
 *
 * Time-of-day progression and weather state management:
 * - Day/night cycle with configurable speed
 * - Weather transitions with delegate notifications
 * - Sky provider interface for visual sync
 * - Widget base classes for time/weather HUD
 */
class FWeatherTimeManagerModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
