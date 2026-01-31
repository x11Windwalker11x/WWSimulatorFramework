#pragma once

namespace DebugDefaultsConstants
{
	static constexpr bool DEBUG_SHOWONSCREENMESSAGES = true;
	static constexpr bool DEBUG_SHOWTRACES = true;
	static constexpr bool DEBUG_SHOWCOLLISIONSHAPES = true;
	static constexpr float DEBUG_DRAWDURATION = 10.0f;
	// For drawing segments of complax draw shapes ie sphere or capsule.
	static constexpr float DEBUG_DrawSegments = 8.0f; 

	static constexpr uint8 DEBUG_ACTIVEDEBUGLAYERS = 14;

	static constexpr float DEBUG_ONSCREENMESSAGEDURATION = 10.0f;

	// Default/Fallback Colors
	static constexpr FLinearColor DEBUG_LAYER_DEBUGDEFAULT_LINEARCOLOR = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
	static constexpr FLinearColor DEBUG_LAYER_DEBUGDEFAULT_LINEARCOLOR_HIT = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
	
	// Debug (Purple → Light Purple)
	static constexpr FLinearColor DEBUG_LAYER_DEBUG_LINEARCOLOR = FLinearColor(0.6f, 0.2f, 0.8f, 1.0f); // Purple
	static constexpr FLinearColor DEBUG_LAYER_DEBUG_LINEARCOLOR_HIT = FLinearColor(0.8f, 0.6f, 1.0f, 1.0f); // Light Purple

	// Widget (Orange → Light Orange)
	static constexpr FLinearColor DEBUG_LAYER_WIDGET_LINEARCOLOR = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
	static constexpr FLinearColor DEBUG_LAYER_WIDGET_LINEARCOLOR_HIT = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f); // Light Orange

	// Interaction (Yellow → Light Yellow)
	static constexpr FLinearColor DEBUG_LAYER_INTERACTION_LINEARCOLOR = FLinearColor(0.9f, 0.9f, 0.0f, 1.0f); // Yellow
	static constexpr FLinearColor DEBUG_LAYER_INTERACTION_LINEARCOLOR_HIT = FLinearColor(1.0f, 1.0f, 0.8f, 1.0f); // Light Yellow

	// Inventory (Teal → Light Teal)
	static constexpr FLinearColor DEBUG_LAYER_INVENTORY_LINEARCOLOR = FLinearColor(0.0f, 0.6f, 0.6f, 1.0f); // Teal
	static constexpr FLinearColor DEBUG_LAYER_INVENTORY_LINEARCOLOR_HIT = FLinearColor(0.5f, 0.9f, 0.9f, 1.0f); // Light Teal

	// AI (Blue-Gray → Light Blue-Gray)
	static constexpr FLinearColor DEBUG_LAYER_AI_LINEARCOLOR = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f); // Blue-Gray
	static constexpr FLinearColor DEBUG_LAYER_AI_LINEARCOLOR_HIT = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f); // Light Blue-Gray

	// Combat (Magenta → Light Magenta)
	static constexpr FLinearColor DEBUG_LAYER_COMBAT_LINEARCOLOR = FLinearColor(0.9f, 0.0f, 0.9f, 1.0f); // Magenta
	static constexpr FLinearColor DEBUG_LAYER_COMBAT_LINEARCOLOR_HIT = FLinearColor(1.0f, 0.6f, 1.0f, 1.0f); // Light Magenta

	// CQC (Dark Red → Light Red)
	static constexpr FLinearColor DEBUG_LAYER_CQC_LINEARCOLOR = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f); // Dark Red
	static constexpr FLinearColor DEBUG_LAYER_CQC_LINEARCOLOR_HIT = FLinearColor(1.0f, 0.6f, 0.6f, 1.0f); // Light Red

	// Parkour (Dark Green → Light Green)
	static constexpr FLinearColor DEBUG_LAYER_PARKOUR_LINEARCOLOR = FLinearColor(0.0f, 0.6f, 0.2f, 1.0f); // Dark Green
	static constexpr FLinearColor DEBUG_LAYER_PARKOUR_LINEARCOLOR_HIT = FLinearColor(0.5f, 0.9f, 0.6f, 1.0f); // Light Green

	// Movement (Blue → Light Blue)
	static constexpr FLinearColor DEBUG_LAYER_MOVEMENT_LINEARCOLOR = FLinearColor(0.0f, 0.4f, 0.9f, 1.0f); // Blue
	static constexpr FLinearColor DEBUG_LAYER_MOVEMENT_LINEARCOLOR_HIT = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f); // Light Blue

	// Character (Cyan → Light Cyan)
	static constexpr FLinearColor DEBUG_LAYER_CHARACTER_LINEARCOLOR = FLinearColor(0.0f, 0.8f, 0.8f, 1.0f); // Cyan
	static constexpr FLinearColor DEBUG_LAYER_CHARACTER_LINEARCOLOR_HIT = FLinearColor(0.5f, 1.0f, 1.0f, 1.0f); // Light Cyan

	// Pawn (Pink → Light Pink)
	static constexpr FLinearColor DEBUG_LAYER_PAWN_LINEARCOLOR = FLinearColor(1.0f, 0.4f, 0.7f, 1.0f); // Pink
	static constexpr FLinearColor DEBUG_LAYER_PAWN_LINEARCOLOR_HIT = FLinearColor(1.0f, 0.7f, 0.9f, 1.0f); // Light Pink

	// Camera (Lime → Light Lime)
	static constexpr FLinearColor DEBUG_LAYER_CAMERA_LINEARCOLOR = FLinearColor(0.5f, 1.0f, 0.0f, 1.0f); // Lime
	static constexpr FLinearColor DEBUG_LAYER_CAMERA_LINEARCOLOR_HIT = FLinearColor(0.8f, 1.0f, 0.5f, 1.0f); // Light Lime

	// InventoryInteraction (Amber → Light Amber)
	static constexpr FLinearColor DEBUG_LAYER_INVENTORYINTERACTION_LINEARCOLOR = FLinearColor(1.0f, 0.75f, 0.0f, 1.0f); // Amber
	static constexpr FLinearColor DEBUG_LAYER_INVENTORYINTERACTION_LINEARCOLOR_HIT = FLinearColor(1.0f, 0.9f, 0.5f, 1.0f); // Light Amber

	// SaveSystem (Indigo → Light Indigo)
	static constexpr FLinearColor DEBUG_LAYER_SAVESYSTEM_LINEARCOLOR = FLinearColor(0.3f, 0.0f, 0.5f, 1.0f); // Indigo
	static constexpr FLinearColor DEBUG_LAYER_SAVESYSTEM_LINEARCOLOR_HIT = FLinearColor(0.6f, 0.4f, 0.8f, 1.0f); // Light Indigo
}