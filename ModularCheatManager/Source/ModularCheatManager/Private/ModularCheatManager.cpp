#include "ModularCheatManager.h"
#include "Modules/ModuleManager.h"
#include "DeveloperWhitelistAsset.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
IMPLEMENT_MODULE(FDefaultModuleImpl, ModularCheatManager)


///////////////////////////
// ServerAssignPermission
///////////////////////////
void UModularCheatManager::ServerAssignPermission_Implementation()
{
	if (!DeveloperWhitelist)
	{
		UE_LOG(LogTemp, Warning, TEXT("DeveloperWhitelist not set! Defaulting to None."));
		CurrentPermission = ECheatPermissionLevel::GodMode;
		return;
	}

	APlayerController* PC = GetOuterAPlayerController();
	APlayerState* PS = PC->PlayerState;
	

	FString PlayerName = PS->GetPlayerName();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("No PlayerState found!"));
		CurrentPermission = ECheatPermissionLevel::GodMode;
		return;
	}
	CurrentPermission = DeveloperWhitelist->GetPermissionForPlayer(PlayerName);

	UE_LOG(LogTemp, Log, TEXT("Assigned cheat permission %s to player %s"),
		*UEnum::GetValueAsString(CurrentPermission), *PlayerName);
}

///////////////////////////
// HasCheatPermission
///////////////////////////
bool UModularCheatManager::HasCheatPermission(ECheatPermissionLevel RequiredLevel) const
{
	return static_cast<uint8>(CurrentPermission) >= static_cast<uint8>(RequiredLevel);
}

///////////////////////////
// Cheat_GiveItem
///////////////////////////
void UModularCheatManager::Cheat_GiveItem(FName ItemID)
{
	if (!HasCheatPermission(ECheatPermissionLevel::Developer) || !HasCheatPermission(ECheatPermissionLevel::GodMode))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player does not have permission to use Cheat_GiveItem."));
		return;
	}

	// Example: Add item to player inventory (pseudo code)
	// ACharacter_Master* MyCharacter = Cast<ACharacter_Master>(GetOuterAPlayerController()->GetPawn());
	// if (MyCharacter)
	// {
	//     MyCharacter->Inventory->AddItem(ItemID, 1);
	// }

	UE_LOG(LogTemp, Log, TEXT("Cheat_GiveItem executed for item: %s"), *ItemID.ToString());
}
