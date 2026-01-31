#include "Subsystems/EventBusSubsystem.h"
#include "Engine/GameInstance.h"

void UEventBusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UEventBusSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UEventBusSubsystem* UEventBusSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance) return nullptr;

	return GameInstance->GetSubsystem<UEventBusSubsystem>();
}

void UEventBusSubsystem::BroadcastEvent(const FGameplayEventPayload& Payload)
{
	LogEvent(Payload);
    OnGameplayEvent.Broadcast(Payload.EventTag, Payload.Instigator, Payload.Target);
}

void UEventBusSubsystem::BroadcastSimpleEvent(FGameplayTag EventTag, AActor* Instigator, float Value)
{
	FGameplayEventPayload Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = Instigator;
	Payload.Value = Value;

	BroadcastEvent(Payload);
}

bool UEventBusSubsystem::DoesEventMatchTag(const FGameplayEventPayload& Payload, FGameplayTag FilterTag) const
{
	return Payload.EventTag.MatchesTag(FilterTag);
}

void UEventBusSubsystem::LogEvent(const FGameplayEventPayload& Payload) const
{
	UE_LOG(LogTemp, Verbose, TEXT("EventBus: %s from %s"), 
		*Payload.EventTag.ToString(), 
		Payload.Instigator ? *Payload.Instigator->GetName() : TEXT("None"));
}