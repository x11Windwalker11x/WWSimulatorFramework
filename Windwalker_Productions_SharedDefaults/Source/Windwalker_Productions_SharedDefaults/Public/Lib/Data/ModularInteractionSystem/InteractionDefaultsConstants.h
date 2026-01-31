#pragma once

#include "CoreMinimal.h"

namespace InteractionDefaultsConstants
{
	// Distances
	static constexpr float PreInteractionDistanceLoad = 2000.f;
	static constexpr float PreInteractionDistanceUI = 300.f;
	static constexpr float InteractionDistanceUI = 150.f;
	static constexpr float DefaultInteractableOverlapRadius = 25.f;
	//radius for debug draws with sphere params ie drawdebugsphereradius
	static constexpr float DefaultDebugDrawRadius = 30.f;

    
	// Settings
	static constexpr bool bEnableGlobalInteractionNotifications = false;
	static constexpr int32 MaxSimultaneouslyLoadedItems = 20; //Tells the engine how many items to be loaded for a client (as gfx and sfx)
	static constexpr float GlobalUIUpdateRate = 0.1f;
	//For updating focused interactables pool based on camerafocus 
	static constexpr float GlobalInteractionFocusUpdateRate = 0.2f;
	static constexpr float GlobalInteractionFocusUpdateRate_Player = 0.1f;
	//True by default for optimizing
	static constexpr bool bUseBatchedTraces = true;

	// Widgets
	inline constexpr const TCHAR* Widget_InteractionPromptPath = TEXT("/AdvancedWidgetFramework/WDGT_DefaultInteractionPrompt.WDGT_DefaultInteractionPrompt_C");
	inline constexpr const TCHAR* Widget_PreInteractionPromptPath = TEXT("/AdvancedWidgetFramework/WDGT_DefaultPreInteractionPrompt.WDGT_DefaultPreInteractionPrompt_C");
    extern WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API const FText Widget_DefaultInteractionText;
	extern WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API const FColor Widget_DefaultPreInteractionColor;
	static constexpr  float Widget_DefaultPreInteractionDotsize = 1.0f;
    
	// Widget Pool Settings
	static constexpr int32 PoolSize_InteractionPrompts = 0; //TODO: make it use the def size which is 5
	static constexpr int32 PoolSize_PreInteractionDots = 0; //TODO: make it use the def size which is 10
	static constexpr int32 ZOrder_PreInteractionDots = 998;
	static constexpr int32 ZOrder_InteractionPrompts = ZOrder_PreInteractionDots + 1;

    
	// Pool Names - API export needed for cross-module linking
	extern WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API const FName PoolName_InteractionPrompts;
	extern WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API const FName PoolName_PreInteractionDots;
}