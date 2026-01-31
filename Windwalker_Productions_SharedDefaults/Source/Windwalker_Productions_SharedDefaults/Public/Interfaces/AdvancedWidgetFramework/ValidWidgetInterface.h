#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ValidWidgetInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UValidWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class IValidWidgetInterface
{
	GENERATED_BODY()

public:
	// Simple marker interface - no functions needed
};