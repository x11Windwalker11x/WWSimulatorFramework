// DockZoneComponent.h
// Placeable dock zone target component
// Layer: L2 (AdvancedWidgetFramework)

#pragma once

#include "CoreMinimal.h"
#include "Lib/Data/AdvancedWidgetFramework/DockableLayoutData.h"
#include "Components/ActorComponent.h"
#include "DockZoneComponent.generated.h"

class UDockLayoutManager;

/**
 * UDockZoneComponent
 *
 * Lightweight component that marks an actor as a dock zone target.
 * Attach to any actor to create a dock zone.
 * Registers/unregisters with UDockLayoutManager automatically.
 *
 * Usage:
 * - Add to any actor (HUD, world space widget host, etc.)
 * - Configure FDockZoneConfig in editor
 * - Widgets implementing IDockableWidgetInterface can dock here
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ADVANCEDWIDGETFRAMEWORK_API UDockZoneComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDockZoneComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Dock zone configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dock Zone")
	FDockZoneConfig ZoneConfig;

	/** Get the zone tag for this component */
	UFUNCTION(BlueprintPure, Category = "Dock Zone")
	FGameplayTag GetZoneTag() const { return ZoneConfig.ZoneTag; }

	/** Get the zone config */
	UFUNCTION(BlueprintPure, Category = "Dock Zone")
	FDockZoneConfig GetZoneConfig() const { return ZoneConfig; }

private:
	/** Cached layout manager ref (Rule #41) */
	TWeakObjectPtr<UDockLayoutManager> CachedLayoutManager;
};
