#include "CoreMinimal.h"
#include "CrafterComponent_Master.h"
#include "GameplayTags.h"
#include "Interfaces/ModularInventorySystem/CraftingInterface.h"
#include "CrafterComponent.generated.h"

UCLASS(ClassGroup=(Component), meta=(BlueprintSpawnableComponent))
class MODULARSYSTEMSBASE_API UCrafterComponent : public UCrafterComponent_Master, ICraftingInterface
{
	GENERATED_BODY()
    
public:
	UCrafterComponent();
	virtual void BeginPlay() override;


protected:
	
};
