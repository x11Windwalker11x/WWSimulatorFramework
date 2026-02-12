// Copyright Windwalker Productions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "QuestGiverInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UQuestGiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that can offer and receive quests (NPCs, terminals, etc.)
 * Usage: ImplementsInterface(UQuestGiverInterface::StaticClass()) + Execute_ pattern
 * Rule #30: Never Cast<> cross-plugin, always use Execute_
 */
class WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API IQuestGiverInterface
{
	GENERATED_BODY()

public:
	/** Returns the quest IDs this actor can offer */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	TArray<FName> GetAvailableQuestIDs() const;

	/** Returns true if this actor has any quests available for the given player */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	bool HasQuestsForPlayer(AActor* Player) const;

	/** Called when player accepts a quest from this giver */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	void OnQuestAcceptedFromGiver(FName QuestID, AActor* Player);

	/** Called when player turns in a completed quest to this giver */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	void OnQuestTurnedInToGiver(FName QuestID, AActor* Player);

	/** Returns the quest giver component (if any) -- mandatory getter per Rule #29 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	UActorComponent* GetQuestGiverComponent();
};
