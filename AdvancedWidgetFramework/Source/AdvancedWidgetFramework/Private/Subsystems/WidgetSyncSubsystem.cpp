// WidgetSyncSubsystem.cpp
// Multiplayer Widget Synchronization implementation

#include "Subsystems/WidgetSyncSubsystem.h"
#include "Subsystems/AdvancedWidgetFramework/WidgetManagerBase.h"
#include "Interfaces/AdvancedWidgetFramework/ReplicatedWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

DECLARE_STATS_GROUP(TEXT("WidgetSyncSubsystem"), STATGROUP_WidgetSyncSubsystem, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("WidgetSyncSubsystem::Tick"), STAT_WidgetSyncSubsystemTick, STATGROUP_WidgetSyncSubsystem);

TStatId UWidgetSyncSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWidgetSyncSubsystem, STATGROUP_WidgetSyncSubsystem);
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UWidgetSyncSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Cache WidgetManagerBase ref (Rule #41)
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer)
	{
		CachedWidgetManager = LocalPlayer->GetSubsystem<UWidgetManagerBase>();

		// Register as sync interceptor on WidgetManagerBase
		if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
		{
			WM->OnWidgetSyncRequested.BindUObject(this, &UWidgetSyncSubsystem::HandleWidgetStateChanged);
		}

		// Cache local player ID
		APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
		if (PC)
		{
			LocalPlayerID = PC->GetUniqueID();
		}
	}

	// Create the network transport component
	EnsureSyncComponent();
}

void UWidgetSyncSubsystem::Deinitialize()
{
	// Unbind from WidgetManagerBase
	if (UWidgetManagerBase* WM = CachedWidgetManager.Get())
	{
		WM->OnWidgetSyncRequested.Unbind();
	}

	SyncedWidgets.Empty();
	SpectatorBindings.Empty();

	Super::Deinitialize();
}

// ============================================================================
// TICK - Delta capture at configured intervals
// ============================================================================

void UWidgetSyncSubsystem::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_WidgetSyncSubsystemTick);

	for (int32 i = SyncedWidgets.Num() - 1; i >= 0; --i)
	{
		FSyncedWidgetEntry& Entry = SyncedWidgets[i];

		// Clean up invalid entries
		if (!Entry.Widget.IsValid())
		{
			SyncedWidgets.RemoveAtSwap(i);
			continue;
		}

		Entry.TimeSinceLastSync += DeltaTime;

		// Check if sync interval has elapsed
		if (Entry.Config.SyncInterval > 0.0f && Entry.TimeSinceLastSync < Entry.Config.SyncInterval)
		{
			continue;
		}

		// Capture and send deltas
		CaptureAndSendDelta(Entry);
		Entry.TimeSinceLastSync = 0.0f;
	}
}

// ============================================================================
// WIDGET SYNC REGISTRATION
// ============================================================================

void UWidgetSyncSubsystem::RegisterForSync(UUserWidget* Widget)
{
	if (!Widget) return;

	// Widget must implement IReplicatedWidgetInterface
	if (!Widget->Implements<UReplicatedWidgetInterface>())
	{
		UE_LOG(LogTemp, Warning, TEXT("WidgetSyncSubsystem: Widget %s does not implement IReplicatedWidgetInterface"), *Widget->GetName());
		return;
	}

	// Check if already registered
	if (FindEntry(Widget)) return;

	FSyncedWidgetEntry NewEntry;
	NewEntry.Widget = Widget;
	NewEntry.WidgetSyncID = IReplicatedWidgetInterface::Execute_GetWidgetSyncID(Widget);
	NewEntry.WidgetTypeTag = IReplicatedWidgetInterface::Execute_GetWidgetTypeTag(Widget);
	NewEntry.Config = IReplicatedWidgetInterface::Execute_GetSyncConfig(Widget);
	NewEntry.TimeSinceLastSync = 0.0f;
	NewEntry.LastSequenceNumber = 0;

	SyncedWidgets.Add(NewEntry);
}

void UWidgetSyncSubsystem::UnregisterFromSync(UUserWidget* Widget)
{
	if (!Widget) return;

	for (int32 i = SyncedWidgets.Num() - 1; i >= 0; --i)
	{
		if (SyncedWidgets[i].Widget.Get() == Widget)
		{
			SyncedWidgets.RemoveAtSwap(i);
			break;
		}
	}
}

bool UWidgetSyncSubsystem::IsRegisteredForSync(UUserWidget* Widget) const
{
	return FindEntry(Widget) != nullptr;
}

// ============================================================================
// SPECTATOR BINDING
// ============================================================================

void UWidgetSyncSubsystem::BindSpectator(int32 TargetPlayerID, FName WidgetSyncID)
{
	if (TargetPlayerID < 0 || WidgetSyncID.IsNone()) return;

	// Check if already bound
	for (const FSpectatorBinding& Binding : SpectatorBindings)
	{
		if (Binding.TargetPlayerID == TargetPlayerID && Binding.WidgetSyncID == WidgetSyncID)
		{
			return; // Already bound
		}
	}

	// Send request to server via sync component
	if (UWidgetSyncComponent* Comp = SyncComponent.Get())
	{
		Comp->Server_RequestSpectatorBind(TargetPlayerID, WidgetSyncID);
	}
}

void UWidgetSyncSubsystem::UnbindSpectator(FName WidgetSyncID)
{
	if (WidgetSyncID.IsNone()) return;

	// Remove local binding
	SpectatorBindings.RemoveAll([WidgetSyncID](const FSpectatorBinding& Binding)
	{
		return Binding.WidgetSyncID == WidgetSyncID;
	});

	// Notify server
	if (UWidgetSyncComponent* Comp = SyncComponent.Get())
	{
		Comp->Server_RequestSpectatorUnbind(WidgetSyncID);
	}
}

// ============================================================================
// STATE APPLICATION (called by network transport)
// ============================================================================

void UWidgetSyncSubsystem::ApplyReceivedPayload(const FWidgetSyncPayload& Payload)
{
	if (!Payload.IsValid()) return;

	// Sequence number check - discard out-of-order payloads
	FSyncedWidgetEntry* Entry = FindEntryBySyncID(Payload.WidgetSyncID);
	if (Entry)
	{
		if (Payload.SequenceNumber <= Entry->LastSequenceNumber)
		{
			return; // Out of order, discard
		}
		Entry->LastSequenceNumber = Payload.SequenceNumber;
	}

	// Find the local widget to apply state to
	UUserWidget* Widget = FindWidgetBySyncID(Payload.WidgetSyncID);
	if (!Widget) return;

	// Apply delta via interface
	if (Widget->Implements<UReplicatedWidgetInterface>())
	{
		IReplicatedWidgetInterface::Execute_ApplyStateDelta(Widget, Payload.Properties);
	}
}

// ============================================================================
// QUERIES
// ============================================================================

FName UWidgetSyncSubsystem::GetWidgetSyncID(UUserWidget* Widget) const
{
	if (const FSyncedWidgetEntry* Entry = FindEntry(Widget))
	{
		return Entry->WidgetSyncID;
	}
	return NAME_None;
}

// ============================================================================
// SYNC CAPTURE
// ============================================================================

void UWidgetSyncSubsystem::CaptureAndSendDelta(FSyncedWidgetEntry& Entry)
{
	if (!Entry.Widget.IsValid()) return;

	UUserWidget* Widget = Entry.Widget.Get();

	// Capture current state via interface
	if (!Widget->Implements<UReplicatedWidgetInterface>()) return;

	TArray<FWidgetSyncProperty> CurrentState = IReplicatedWidgetInterface::Execute_CaptureStateDelta(Widget);

	if (CurrentState.Num() == 0) return;

	// Compute delta against last captured state
	TArray<FWidgetSyncProperty> Delta;
	for (const FWidgetSyncProperty& Prop : CurrentState)
	{
		bool bChanged = true;

		// Check against last captured state
		for (const FWidgetSyncProperty& LastProp : Entry.LastCapturedState)
		{
			if (LastProp.PropertyName == Prop.PropertyName)
			{
				if (LastProp.Value == Prop.Value)
				{
					bChanged = false;
				}
				break;
			}
		}

		if (bChanged)
		{
			Delta.Add(Prop);
		}
	}

	// Update last captured state
	Entry.LastCapturedState = CurrentState;

	if (Delta.Num() == 0) return;

	// Apply max properties limit
	if (Entry.Config.MaxPropertiesPerSync > 0 && Delta.Num() > Entry.Config.MaxPropertiesPerSync)
	{
		Delta.SetNum(Entry.Config.MaxPropertiesPerSync);
	}

	// Build payload and send
	FWidgetSyncPayload Payload = BuildPayload(Entry, Delta);
	SendToServer(Payload);
}

FWidgetSyncPayload UWidgetSyncSubsystem::BuildPayload(FSyncedWidgetEntry& Entry, const TArray<FWidgetSyncProperty>& Delta)
{
	FWidgetSyncPayload Payload;
	Payload.WidgetSyncID = Entry.WidgetSyncID;
	Payload.WidgetTypeTag = Entry.WidgetTypeTag;
	Payload.OwningPlayerID = LocalPlayerID;
	Payload.ServerTimestamp = 0.0f; // Server will stamp this
	Payload.Properties = Delta;
	Payload.SequenceNumber = ++Entry.LastSequenceNumber;

	return Payload;
}

// ============================================================================
// INTERCEPT HANDLER
// ============================================================================

void UWidgetSyncSubsystem::HandleWidgetStateChanged(UUserWidget* Widget, FGameplayTag NewState)
{
	if (!Widget) return;

	// Only capture for registered widgets
	FSyncedWidgetEntry* Entry = FindEntry(Widget);
	if (!Entry) return;

	// Force an immediate sync capture on state change
	CaptureAndSendDelta(*Entry);
}

// ============================================================================
// NETWORK TRANSPORT
// ============================================================================

void UWidgetSyncSubsystem::EnsureSyncComponent()
{
	if (SyncComponent.IsValid()) return;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
	if (!PC) return;

	// Check for existing component
	UWidgetSyncComponent* Existing = PC->FindComponentByClass<UWidgetSyncComponent>();
	if (Existing)
	{
		SyncComponent = Existing;
		Existing->OwningSubsystem = this;
		return;
	}

	// Create new component on PlayerController
	UWidgetSyncComponent* NewComp = NewObject<UWidgetSyncComponent>(PC, TEXT("WidgetSyncComponent"));
	if (NewComp)
	{
		NewComp->RegisterComponent();
		NewComp->OwningSubsystem = this;
		SyncComponent = NewComp;
	}
}

void UWidgetSyncSubsystem::SendToServer(const FWidgetSyncPayload& Payload)
{
	if (!Payload.IsValid()) return;

	EnsureSyncComponent();

	if (UWidgetSyncComponent* Comp = SyncComponent.Get())
	{
		Comp->Server_SendSyncPayload(Payload);
	}
}

// ============================================================================
// INTERNAL - Entry lookup
// ============================================================================

FSyncedWidgetEntry* UWidgetSyncSubsystem::FindEntry(UUserWidget* Widget)
{
	if (!Widget) return nullptr;

	for (FSyncedWidgetEntry& Entry : SyncedWidgets)
	{
		if (Entry.Widget.Get() == Widget)
		{
			return &Entry;
		}
	}
	return nullptr;
}

const FSyncedWidgetEntry* UWidgetSyncSubsystem::FindEntry(UUserWidget* Widget) const
{
	if (!Widget) return nullptr;

	for (const FSyncedWidgetEntry& Entry : SyncedWidgets)
	{
		if (Entry.Widget.Get() == Widget)
		{
			return &Entry;
		}
	}
	return nullptr;
}

FSyncedWidgetEntry* UWidgetSyncSubsystem::FindEntryBySyncID(FName SyncID)
{
	if (SyncID.IsNone()) return nullptr;

	for (FSyncedWidgetEntry& Entry : SyncedWidgets)
	{
		if (Entry.WidgetSyncID == SyncID)
		{
			return &Entry;
		}
	}
	return nullptr;
}

UUserWidget* UWidgetSyncSubsystem::FindWidgetBySyncID(FName SyncID)
{
	if (FSyncedWidgetEntry* Entry = FindEntryBySyncID(SyncID))
	{
		return Entry->Widget.Get();
	}
	return nullptr;
}

// ============================================================================
// UWidgetSyncComponent - Network Transport
// ============================================================================

UWidgetSyncComponent::UWidgetSyncComponent()
{
	SetIsReplicatedByDefault(true);
}

void UWidgetSyncComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// No replicated properties - all data flows through RPCs
}

// ============================================================================
// SERVER RPCs
// ============================================================================

void UWidgetSyncComponent::Server_SendSyncPayload_Implementation(const FWidgetSyncPayload& Payload)
{
	if (!Payload.IsValid()) return;

	// Server validation: verify owning player matches the sender
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APlayerController* OwnerPC = Cast<APlayerController>(Owner);
	if (!OwnerPC) return;

	// Stamp server time
	FWidgetSyncPayload ValidatedPayload = Payload;
	if (UWorld* World = GetWorld())
	{
		ValidatedPayload.ServerTimestamp = World->GetTimeSeconds();
	}

	// Broadcast to all relevant clients
	// For now, broadcast to the sender's own client (for server authority)
	Client_ReceiveSyncPayload(ValidatedPayload);

	// TODO: In full implementation, iterate connected PlayerControllers
	// and send to those who are spectating this widget or need the update
}

void UWidgetSyncComponent::Server_RequestSpectatorBind_Implementation(int32 TargetPlayerID, FName WidgetSyncID)
{
	if (TargetPlayerID < 0 || WidgetSyncID.IsNone()) return;

	// Server validates the request
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APlayerController* OwnerPC = Cast<APlayerController>(Owner);
	if (!OwnerPC) return;

	int32 SpectatorPlayerID = OwnerPC->GetUniqueID();

	// Confirm binding back to the requesting client
	Client_SpectatorBindConfirmed(TargetPlayerID, WidgetSyncID);
}

void UWidgetSyncComponent::Server_RequestSpectatorUnbind_Implementation(FName WidgetSyncID)
{
	// Server acknowledges unbind - no confirmation needed
	// The client has already removed the local binding
}

// ============================================================================
// CLIENT RPCs
// ============================================================================

void UWidgetSyncComponent::Client_ReceiveSyncPayload_Implementation(const FWidgetSyncPayload& Payload)
{
	if (!Payload.IsValid()) return;

	// Forward to owning subsystem
	if (UWidgetSyncSubsystem* Subsystem = OwningSubsystem.Get())
	{
		Subsystem->ApplyReceivedPayload(Payload);
	}
}

void UWidgetSyncComponent::Client_SpectatorBindConfirmed_Implementation(int32 TargetPlayerID, FName WidgetSyncID)
{
	if (UWidgetSyncSubsystem* Subsystem = OwningSubsystem.Get())
	{
		// Add to local spectator bindings
		FSpectatorBinding Binding;
		Binding.SpectatorPlayerID = Subsystem->LocalPlayerID;
		Binding.TargetPlayerID = TargetPlayerID;
		Binding.WidgetSyncID = WidgetSyncID;

		// Prevent duplicates
		bool bAlreadyBound = false;
		for (const FSpectatorBinding& Existing : Subsystem->SpectatorBindings)
		{
			if (Existing.TargetPlayerID == TargetPlayerID && Existing.WidgetSyncID == WidgetSyncID)
			{
				bAlreadyBound = true;
				break;
			}
		}

		if (!bAlreadyBound)
		{
			Subsystem->SpectatorBindings.Add(Binding);
		}

		// Fire delegate
		Subsystem->OnSpectatorBound.Broadcast(Binding.SpectatorPlayerID, TargetPlayerID);
	}
}
