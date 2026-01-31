// CheatPermissionLevel.h or similar
#pragma once

#include "CoreMinimal.h"

#include "ECheatPermissionLevel.generated.h"

UENUM(BlueprintType)
enum class ECheatPermissionLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Tester      UMETA(DisplayName = "Tester"),
    Developer   UMETA(DisplayName = "Developer"),
    Admin       UMETA(DisplayName = "Admin"),
    GodMode     UMETA(DisplayName = "GodMode")
};
