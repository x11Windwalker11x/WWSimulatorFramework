// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * Modular Economy Plugin Module
 *
 * Financial backbone for tycoon gameplay:
 * - Balance tracking with transaction history
 * - Recurring billing entries (rent, wages)
 * - Resource consumption tracking (electricity, water, gas)
 * - Time-based billing cycles
 * - Device integration via IEconomyInterface
 */
class FModularEconomyPluginModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
