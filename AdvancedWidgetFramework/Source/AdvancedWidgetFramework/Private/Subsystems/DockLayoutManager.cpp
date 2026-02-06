// DockLayoutManager.cpp
// Dockable/Composable Layout Engine implementation

#include "Subsystems/DockLayoutManager.h"
#include "Subsystems/AdvancedWidgetFramework/WidgetManagerBase.h"
#include "Interfaces/AdvancedWidgetFramework/DockableWidgetInterface.h"
#include "Components/DockZoneComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UDockLayoutManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Cache WidgetManagerBase ref (Rule #41)
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer)
	{
		CachedWidgetManager = LocalPlayer->GetSubsystem<UWidgetManagerBase>();

		// Register as dock interceptor
		if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
		{
			WM->OnWidgetDockRequested.BindUObject(this, &UDockLayoutManager::HandleDockIntercept);
		}
	}
}

void UDockLayoutManager::Deinitialize()
{
	// Unbind from WidgetManagerBase
	if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
	{
		WM->OnWidgetDockRequested.Unbind();
	}

	// Notify all docked widgets they are being undocked
	for (FDockedWidgetEntry& Entry : DockedWidgets)
	{
		if (UUserWidget* Widget = Entry.Widget.Get())
		{
			if (Widget->Implements<UDockableWidgetInterface>())
			{
				IDockableWidgetInterface::Execute_OnUndocked(Widget);
			}
		}
	}

	DockedWidgets.Empty();
	DockZones.Empty();

	Super::Deinitialize();
}

// ============================================================================
// DOCK ZONE MANAGEMENT
// ============================================================================

void UDockLayoutManager::RegisterDockZone(const FDockZoneConfig& Config)
{
	if (!Config.IsValid()) return;

	// Check if already registered
	if (FindZone(Config.ZoneTag)) return;

	FDockZoneEntry NewZone;
	NewZone.Config = Config;

	DockZones.Add(NewZone);
}

void UDockLayoutManager::RegisterDockZoneWithComponent(const FDockZoneConfig& Config, UDockZoneComponent* Component)
{
	if (!Config.IsValid()) return;

	// Check if already registered
	if (FindZone(Config.ZoneTag)) return;

	FDockZoneEntry NewZone;
	NewZone.Config = Config;
	NewZone.OwningComponent = Component;

	DockZones.Add(NewZone);
}

void UDockLayoutManager::UnregisterDockZone(FGameplayTag ZoneTag)
{
	if (!ZoneTag.IsValid()) return;

	FDockZoneEntry* Zone = FindZone(ZoneTag);
	if (!Zone) return;

	// Undock all widgets in this zone
	TArray<FName> WidgetIDs = Zone->DockedWidgetIDs;
	for (const FName& ID : WidgetIDs)
	{
		if (FDockedWidgetEntry* Entry = FindDockedEntryByID(ID))
		{
			if (UUserWidget* Widget = Entry->Widget.Get())
			{
				UndockWidget(Widget);
			}
		}
	}

	// Remove the zone
	for (int32 i = DockZones.Num() - 1; i >= 0; --i)
	{
		if (DockZones[i].Config.ZoneTag == ZoneTag)
		{
			DockZones.RemoveAtSwap(i);
			break;
		}
	}
}

bool UDockLayoutManager::IsDockZoneRegistered(FGameplayTag ZoneTag) const
{
	return FindZone(ZoneTag) != nullptr;
}

// ============================================================================
// DOCK / UNDOCK
// ============================================================================

bool UDockLayoutManager::DockWidget(UUserWidget* Widget, FGameplayTag ZoneTag, EDockPosition Position)
{
	if (!Widget || !ZoneTag.IsValid()) return false;

	// Widget must implement IDockableWidgetInterface
	if (!Widget->Implements<UDockableWidgetInterface>())
	{
		UE_LOG(LogTemp, Warning, TEXT("DockLayoutManager: Widget %s does not implement IDockableWidgetInterface"), *Widget->GetName());
		return false;
	}

	// Find the target zone
	FDockZoneEntry* Zone = FindZone(ZoneTag);
	if (!Zone)
	{
		UE_LOG(LogTemp, Warning, TEXT("DockLayoutManager: Zone %s not registered"), *ZoneTag.ToString());
		return false;
	}

	// Get widget config via interface
	FDockableWidgetConfig WidgetConfig = IDockableWidgetInterface::Execute_GetDockConfig(Widget);
	FName DockableID = IDockableWidgetInterface::Execute_GetDockableID(Widget);
	FGameplayTag WidgetTypeTag = IDockableWidgetInterface::Execute_GetDockWidgetTypeTag(Widget);

	if (DockableID.IsNone()) return false;

	// Check zone accepts this widget type
	if (!IsWidgetAcceptedByZone(*Zone, WidgetTypeTag))
	{
		return false;
	}

	// Check if already docked somewhere - undock first
	if (FDockedWidgetEntry* Existing = FindDockedEntry(Widget))
	{
		UndockWidget(Widget);
	}

	// Check zone capacity and resolve conflicts if needed
	if (Zone->Config.MaxOccupants > 0 && Zone->DockedWidgetIDs.Num() >= Zone->Config.MaxOccupants)
	{
		if (!ResolveConflict(*Zone, Widget))
		{
			return false; // Conflict not resolved, can't dock
		}
	}

	// Create docked entry
	FDockedWidgetEntry NewEntry;
	NewEntry.Widget = Widget;
	NewEntry.DockableID = DockableID;
	NewEntry.WidgetTypeTag = WidgetTypeTag;
	NewEntry.ZoneTag = ZoneTag;
	NewEntry.Position = Position;
	NewEntry.SizeRatio = 1.0f;
	NewEntry.Config = WidgetConfig;

	// If zone is tabbed (multiple widgets), assign tab index
	if (Zone->DockedWidgetIDs.Num() > 0 && Zone->Config.ConflictPolicy == EDockConflictPolicy::Tab)
	{
		NewEntry.TabIndex = Zone->DockedWidgetIDs.Num();
	}

	DockedWidgets.Add(NewEntry);
	Zone->DockedWidgetIDs.Add(DockableID);

	// Notify the widget
	IDockableWidgetInterface::Execute_OnDocked(Widget, ZoneTag, Position);

	// Fire delegates
	OnWidgetDocked.Broadcast(Widget, ZoneTag, DockableID);
	NotifyLayoutChanged();

	return true;
}

bool UDockLayoutManager::UndockWidget(UUserWidget* Widget)
{
	if (!Widget) return false;

	FDockedWidgetEntry* Entry = FindDockedEntry(Widget);
	if (!Entry) return false;

	FName DockableID = Entry->DockableID;
	FGameplayTag ZoneTag = Entry->ZoneTag;

	// Remove from zone tracking
	if (FDockZoneEntry* Zone = FindZone(ZoneTag))
	{
		Zone->DockedWidgetIDs.Remove(DockableID);

		// Recalculate tab indices for remaining widgets
		if (Zone->Config.ConflictPolicy == EDockConflictPolicy::Tab)
		{
			int32 TabIdx = 0;
			for (const FName& ID : Zone->DockedWidgetIDs)
			{
				if (FDockedWidgetEntry* DockedEntry = FindDockedEntryByID(ID))
				{
					DockedEntry->TabIndex = TabIdx++;
				}
			}

			// Clamp active tab
			if (Zone->ActiveTabIndex >= Zone->DockedWidgetIDs.Num())
			{
				Zone->ActiveTabIndex = FMath::Max(0, Zone->DockedWidgetIDs.Num() - 1);
			}
		}
	}

	// Remove from docked widgets
	for (int32 i = DockedWidgets.Num() - 1; i >= 0; --i)
	{
		if (DockedWidgets[i].Widget.Get() == Widget)
		{
			DockedWidgets.RemoveAtSwap(i);
			break;
		}
	}

	// Notify the widget
	if (Widget->Implements<UDockableWidgetInterface>())
	{
		IDockableWidgetInterface::Execute_OnUndocked(Widget);
	}

	// Fire delegates
	OnWidgetUndocked.Broadcast(Widget, DockableID);
	NotifyLayoutChanged();

	return true;
}

bool UDockLayoutManager::MoveWidget(UUserWidget* Widget, FGameplayTag NewZoneTag, EDockPosition NewPosition)
{
	if (!Widget || !NewZoneTag.IsValid()) return false;

	// Undock from current zone and dock into new one
	UndockWidget(Widget);
	return DockWidget(Widget, NewZoneTag, NewPosition);
}

bool UDockLayoutManager::FloatWidget(UUserWidget* Widget, FVector2D ScreenPosition, FVector2D Size)
{
	if (!Widget) return false;

	// Check widget supports floating
	if (Widget->Implements<UDockableWidgetInterface>())
	{
		FDockableWidgetConfig Config = IDockableWidgetInterface::Execute_GetDockConfig(Widget);
		if (!Config.bCanFloat) return false;
	}

	// If docked, undock first
	FDockedWidgetEntry* Entry = FindDockedEntry(Widget);
	if (Entry)
	{
		// Store float position before undocking
		FGameplayTag OldZone = Entry->ZoneTag;
		UndockWidget(Widget);
	}

	// Find or create the floating "zone" concept
	// Floating widgets are tracked as docked entries with Position == Float
	// but not associated with any real zone
	if (Widget->Implements<UDockableWidgetInterface>())
	{
		FDockableWidgetConfig WidgetConfig = IDockableWidgetInterface::Execute_GetDockConfig(Widget);
		FName DockableID = IDockableWidgetInterface::Execute_GetDockableID(Widget);

		FDockedWidgetEntry FloatEntry;
		FloatEntry.Widget = Widget;
		FloatEntry.DockableID = DockableID;
		FloatEntry.Position = EDockPosition::Float;
		FloatEntry.FloatPosition = ScreenPosition;
		FloatEntry.FloatSize = Size;
		FloatEntry.Config = WidgetConfig;

		DockedWidgets.Add(FloatEntry);

		// Notify widget
		IDockableWidgetInterface::Execute_OnDocked(Widget, FGameplayTag(), EDockPosition::Float);
		NotifyLayoutChanged();

		return true;
	}

	return false;
}

// ============================================================================
// SPLIT AND TAB
// ============================================================================

bool UDockLayoutManager::SplitZone(FGameplayTag ZoneTag, UUserWidget* Widget, EDockSplitOrientation Orientation, float SplitRatio)
{
	if (!ZoneTag.IsValid() || !Widget) return false;

	FDockZoneEntry* Zone = FindZone(ZoneTag);
	if (!Zone) return false;

	// Set the split configuration
	Zone->SplitConfig.Orientation = Orientation;
	Zone->SplitConfig.SplitRatio = FMath::Clamp(SplitRatio, 0.1f, 0.9f);

	// Adjust existing widgets to first half
	for (const FName& ID : Zone->DockedWidgetIDs)
	{
		if (FDockedWidgetEntry* Entry = FindDockedEntryByID(ID))
		{
			Entry->SizeRatio = SplitRatio;
			if (Entry->Widget.IsValid() && Entry->Widget->Implements<UDockableWidgetInterface>())
			{
				IDockableWidgetInterface::Execute_OnDockLayoutChanged(Entry->Widget.Get(), Entry->Position, SplitRatio);
			}
		}
	}

	// Dock the new widget with the remaining space
	bool bResult = DockWidget(Widget, ZoneTag, Zone->Config.DefaultPosition);
	if (bResult)
	{
		if (FDockedWidgetEntry* NewEntry = FindDockedEntry(Widget))
		{
			NewEntry->SizeRatio = 1.0f - SplitRatio;
		}
	}

	return bResult;
}

void UDockLayoutManager::SetActiveTab(FGameplayTag ZoneTag, int32 TabIndex)
{
	FDockZoneEntry* Zone = FindZone(ZoneTag);
	if (!Zone) return;

	if (TabIndex >= 0 && TabIndex < Zone->DockedWidgetIDs.Num())
	{
		Zone->ActiveTabIndex = TabIndex;
		NotifyLayoutChanged();
	}
}

int32 UDockLayoutManager::GetActiveTab(FGameplayTag ZoneTag) const
{
	const FDockZoneEntry* Zone = FindZone(ZoneTag);
	return Zone ? Zone->ActiveTabIndex : -1;
}

TArray<UUserWidget*> UDockLayoutManager::GetTabWidgets(FGameplayTag ZoneTag) const
{
	TArray<UUserWidget*> Result;
	const FDockZoneEntry* Zone = FindZone(ZoneTag);
	if (!Zone) return Result;

	for (const FName& ID : Zone->DockedWidgetIDs)
	{
		for (const FDockedWidgetEntry& Entry : DockedWidgets)
		{
			if (Entry.DockableID == ID && Entry.Widget.IsValid())
			{
				Result.Add(Entry.Widget.Get());
				break;
			}
		}
	}

	return Result;
}

// ============================================================================
// LAYOUT PERSISTENCE
// ============================================================================

FDockLayout UDockLayoutManager::CaptureLayout(FName LayoutName) const
{
	FDockLayout Layout;
	Layout.LayoutName = LayoutName;

	for (const FDockedWidgetEntry& Entry : DockedWidgets)
	{
		if (!Entry.Widget.IsValid()) continue;

		FDockLayoutEntry LayoutEntry;
		LayoutEntry.DockableID = Entry.DockableID;
		LayoutEntry.ZoneTag = Entry.ZoneTag;
		LayoutEntry.Position = Entry.Position;
		LayoutEntry.SizeRatio = Entry.SizeRatio;
		LayoutEntry.TabIndex = Entry.TabIndex;
		LayoutEntry.FloatPosition = Entry.FloatPosition;
		LayoutEntry.FloatSize = Entry.FloatSize;

		Layout.Entries.Add(LayoutEntry);
	}

	return Layout;
}

bool UDockLayoutManager::ApplyLayout(const FDockLayout& Layout)
{
	if (!Layout.IsValid()) return false;

	// Undock all current widgets
	TArray<UUserWidget*> CurrentWidgets;
	for (const FDockedWidgetEntry& Entry : DockedWidgets)
	{
		if (Entry.Widget.IsValid())
		{
			CurrentWidgets.Add(Entry.Widget.Get());
		}
	}

	for (UUserWidget* Widget : CurrentWidgets)
	{
		UndockWidget(Widget);
	}

	// Apply layout entries
	for (const FDockLayoutEntry& LayoutEntry : Layout.Entries)
	{
		// Find the widget by dockable ID among the previously undocked widgets
		for (UUserWidget* Widget : CurrentWidgets)
		{
			if (!Widget->Implements<UDockableWidgetInterface>()) continue;

			FName WidgetID = IDockableWidgetInterface::Execute_GetDockableID(Widget);
			if (WidgetID == LayoutEntry.DockableID)
			{
				if (LayoutEntry.Position == EDockPosition::Float)
				{
					FloatWidget(Widget, LayoutEntry.FloatPosition, LayoutEntry.FloatSize);
				}
				else if (LayoutEntry.ZoneTag.IsValid())
				{
					DockWidget(Widget, LayoutEntry.ZoneTag, LayoutEntry.Position);

					// Restore size ratio
					if (FDockedWidgetEntry* Entry = FindDockedEntry(Widget))
					{
						Entry->SizeRatio = LayoutEntry.SizeRatio;
						Entry->TabIndex = LayoutEntry.TabIndex;
					}
				}
				break;
			}
		}
	}

	return true;
}

void UDockLayoutManager::ResetToDefaultLayout()
{
	// Collect all docked widgets
	TArray<UUserWidget*> AllWidgets;
	for (const FDockedWidgetEntry& Entry : DockedWidgets)
	{
		if (Entry.Widget.IsValid())
		{
			AllWidgets.Add(Entry.Widget.Get());
		}
	}

	// Undock all
	for (UUserWidget* Widget : AllWidgets)
	{
		UndockWidget(Widget);
	}

	// Re-dock each widget using its preferred zone/position from config
	for (UUserWidget* Widget : AllWidgets)
	{
		if (!Widget->Implements<UDockableWidgetInterface>()) continue;

		FDockableWidgetConfig Config = IDockableWidgetInterface::Execute_GetDockConfig(Widget);
		if (Config.PreferredZoneTag.IsValid())
		{
			DockWidget(Widget, Config.PreferredZoneTag, Config.PreferredPosition);
		}
	}
}

// ============================================================================
// QUERIES
// ============================================================================

bool UDockLayoutManager::IsWidgetDocked(UUserWidget* Widget) const
{
	return FindDockedEntry(Widget) != nullptr;
}

FGameplayTag UDockLayoutManager::GetWidgetDockZone(UUserWidget* Widget) const
{
	if (const FDockedWidgetEntry* Entry = FindDockedEntry(Widget))
	{
		return Entry->ZoneTag;
	}
	return FGameplayTag();
}

TArray<UUserWidget*> UDockLayoutManager::GetWidgetsInZone(FGameplayTag ZoneTag) const
{
	TArray<UUserWidget*> Result;

	const FDockZoneEntry* Zone = FindZone(ZoneTag);
	if (!Zone) return Result;

	for (const FName& ID : Zone->DockedWidgetIDs)
	{
		for (const FDockedWidgetEntry& Entry : DockedWidgets)
		{
			if (Entry.DockableID == ID && Entry.Widget.IsValid())
			{
				Result.Add(Entry.Widget.Get());
				break;
			}
		}
	}

	return Result;
}

TArray<FGameplayTag> UDockLayoutManager::GetRegisteredZones() const
{
	TArray<FGameplayTag> Result;
	for (const FDockZoneEntry& Zone : DockZones)
	{
		Result.Add(Zone.Config.ZoneTag);
	}
	return Result;
}

FDockZoneConfig UDockLayoutManager::GetZoneConfig(FGameplayTag ZoneTag) const
{
	if (const FDockZoneEntry* Zone = FindZone(ZoneTag))
	{
		return Zone->Config;
	}
	return FDockZoneConfig();
}

// ============================================================================
// INTERCEPT HANDLER
// ============================================================================

bool UDockLayoutManager::HandleDockIntercept(UUserWidget* Widget, FGameplayTag TargetZoneTag)
{
	if (!Widget || !TargetZoneTag.IsValid()) return false;

	return DockWidget(Widget, TargetZoneTag);
}

// ============================================================================
// CONFLICT RESOLUTION
// ============================================================================

bool UDockLayoutManager::ResolveConflict(FDockZoneEntry& Zone, UUserWidget* IncomingWidget)
{
	switch (Zone.Config.ConflictPolicy)
	{
	case EDockConflictPolicy::Reject:
		return false; // Reject incoming widget

	case EDockConflictPolicy::Replace:
	{
		// Undock the first (oldest) widget
		if (Zone.DockedWidgetIDs.Num() > 0)
		{
			FName OldestID = Zone.DockedWidgetIDs[0];
			if (FDockedWidgetEntry* OldEntry = FindDockedEntryByID(OldestID))
			{
				if (UUserWidget* OldWidget = OldEntry->Widget.Get())
				{
					UndockWidget(OldWidget);
				}
			}
		}
		return true;
	}

	case EDockConflictPolicy::Tab:
		// Allow additional widget as a new tab - bump max temporarily
		// Tab index will be assigned in DockWidget
		return true;

	case EDockConflictPolicy::Split:
	{
		// Allow additional widget - split will be applied
		// SplitZone handles the actual splitting
		return true;
	}

	default:
		return false;
	}
}

// ============================================================================
// INTERNAL
// ============================================================================

FDockedWidgetEntry* UDockLayoutManager::FindDockedEntry(UUserWidget* Widget)
{
	if (!Widget) return nullptr;

	for (FDockedWidgetEntry& Entry : DockedWidgets)
	{
		if (Entry.Widget.Get() == Widget)
		{
			return &Entry;
		}
	}
	return nullptr;
}

const FDockedWidgetEntry* UDockLayoutManager::FindDockedEntry(UUserWidget* Widget) const
{
	if (!Widget) return nullptr;

	for (const FDockedWidgetEntry& Entry : DockedWidgets)
	{
		if (Entry.Widget.Get() == Widget)
		{
			return &Entry;
		}
	}
	return nullptr;
}

FDockedWidgetEntry* UDockLayoutManager::FindDockedEntryByID(FName DockableID)
{
	if (DockableID.IsNone()) return nullptr;

	for (FDockedWidgetEntry& Entry : DockedWidgets)
	{
		if (Entry.DockableID == DockableID)
		{
			return &Entry;
		}
	}
	return nullptr;
}

FDockZoneEntry* UDockLayoutManager::FindZone(FGameplayTag ZoneTag)
{
	if (!ZoneTag.IsValid()) return nullptr;

	for (FDockZoneEntry& Zone : DockZones)
	{
		if (Zone.Config.ZoneTag == ZoneTag)
		{
			return &Zone;
		}
	}
	return nullptr;
}

const FDockZoneEntry* UDockLayoutManager::FindZone(FGameplayTag ZoneTag) const
{
	if (!ZoneTag.IsValid()) return nullptr;

	for (const FDockZoneEntry& Zone : DockZones)
	{
		if (Zone.Config.ZoneTag == ZoneTag)
		{
			return &Zone;
		}
	}
	return nullptr;
}

bool UDockLayoutManager::IsWidgetAcceptedByZone(const FDockZoneEntry& Zone, FGameplayTag WidgetTypeTag) const
{
	// If no filter set, accept all
	if (Zone.Config.AcceptedWidgetTags.IsEmpty())
	{
		return true;
	}

	// Check if widget type matches any accepted tag
	return Zone.Config.AcceptedWidgetTags.HasTag(WidgetTypeTag);
}

void UDockLayoutManager::NotifyLayoutChanged()
{
	OnLayoutChanged.Broadcast();

	// Also notify WidgetManagerBase for external listeners
	if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
	{
		WM->OnDockLayoutChanged.Broadcast();
	}
}
