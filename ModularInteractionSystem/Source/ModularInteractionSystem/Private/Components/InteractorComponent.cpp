// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InteractorComponent.h"
#include "InteractionSubsystem.h"
#include "Components/InteractableComponent.h"
#include "Debug/DebugSubsystem.h"
#include "Interface/InteractionSystem/InteractorInterface.h"
#include "Windwalker_Productions_SharedDefaults.h"
#include "GenericPlatform/GenericPlatformChunkInstall.h"
#include "UI/Widget_PreInteraction.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/ModularInteractionSystem/InteractorInterface.h"


UInteractorComponent::UInteractorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    OwnerPawn = nullptr;

    
    // Create sphere components with anti-culling fixes
    PreInteractionSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("PreInteractionSphereComponent"));
    PreInteractionSphereComponent->InitSphereRadius(InteractionDefaultsConstants::PreInteractionDistanceLoad);
    // ✅ FIX: Force sphere to always detect overlaps, even behind camera (disable occlusion culling)
    PreInteractionSphereComponent->SetGenerateOverlapEvents(true);
    PreInteractionSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PreInteractionSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    PreInteractionSphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    PreInteractionSphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    PreInteractionSphereComponent->bNeverDistanceCull = true;
    
    InteractionSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphereComponent"));
    InteractionSphereComponent->InitSphereRadius(InteractionDefaultsConstants::InteractionDistanceUI);
    // ✅ FIX: Disable culling for interaction sphere too
    InteractionSphereComponent->SetGenerateOverlapEvents(true);
    InteractionSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphereComponent->bNeverDistanceCull = true;
    
    PreInteractionComponentUI = CreateDefaultSubobject<USphereComponent>(TEXT("PreInteractionComponentUI"));
    PreInteractionComponentUI->InitSphereRadius(InteractionDefaultsConstants::PreInteractionDistanceUI);
    // ✅ FIX: UI spheres also need anti-culling
    PreInteractionComponentUI->SetGenerateOverlapEvents(true);
    PreInteractionComponentUI->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PreInteractionComponentUI->bNeverDistanceCull = true;
    
    InteractionComponentUI = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionComponentUI"));
    InteractionComponentUI->InitSphereRadius(InteractionDefaultsConstants::InteractionDistanceUI);
    // ✅ FIX: UI spheres also need anti-culling
    InteractionComponentUI->SetGenerateOverlapEvents(true);
    InteractionComponentUI->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionComponentUI->bNeverDistanceCull = true;
    
    // Load widget classes
    Widget_InteractionPromptClass = LoadWidgetClass(
        InteractionDefaultsConstants::Widget_InteractionPromptPath
    );
        
    Widget_PreInteractionPromptClass = LoadWidgetClass(
        InteractionDefaultsConstants::Widget_PreInteractionPromptPath
    );
}

void UInteractorComponent::BeginPlay()
{
    Super::BeginPlay();    
    // Set OwnerPawn immediately
    OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        // Try to get controller immediately
        OwnerPlayerController = Cast<APlayerController>(OwnerPawn->GetController());
        
        if (!OwnerPlayerController)
        {
            // Controller not ready yet - set up retry timer
            GetWorld()->GetTimerManager().SetTimer(
                ControllerCheckTimer,
                this,
                &UInteractorComponent::TryGetPlayerController,
                0.1f,  // Check every 0.1 seconds
                true   // Loop
            );
            
            UE_LOG(LogTemp, Warning, TEXT("⏳ InteractorComponent: Waiting for PlayerController on %s"), 
                *OwnerPawn->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("✅ InteractorComponent: PlayerController ready for %s"), 
                *OwnerPawn->GetName());
        }
    }

    
    UE_LOG(LogTemp, Log, TEXT("✅ InteractorComponent: OwnerPawn set to %s"), *OwnerPawn->GetName());
    
    PreInteractionComponentUI->SetIsReplicated(true);
    InteractionComponentUI->SetIsReplicated(true);
    
    DebugTag_Interaction = FGameplayTag::RequestGameplayTag(FName("Debug.Interaction"));
    // Initialize widget pointers
    CurrentInteractionPrompt = nullptr;
    CurrentPreInteractionPrompt = nullptr;

    if (OwnerPawn)
    {
        if (OwnerPawn->GetClass()->ImplementsInterface(UInteractorInterface::StaticClass()))
        {
            PhysicsGrabComponent = IInteractorInterface::Execute_GetPhysicsGrabComponent(OwnerPawn);
        }
        
    }

    UWorld* World = GetWorld();
    if (!World) return;

    // Get subsystems
    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        InteractionSubsystem = GameInstance->GetSubsystem<UInteractionSubsystem>();
        DebugSubsystem = GameInstance->GetSubsystem<UDebugSubsystem>();
        //WidgetSubsystem = GameInstance->GetSubsystem<UWidgetSubsystem>();
    }

    // Setup widgets again if failed to construct in constructor
    if (!Widget_InteractionPromptClass)
    {
        Widget_InteractionPromptClass = LoadClass<UUserWidget>(
            nullptr,
            InteractionDefaultsConstants::Widget_InteractionPromptPath
        );
        
        if (Widget_InteractionPromptClass)
        {
            UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, TEXT("✅ Loaded InteractionPrompt widget class"), false, DebugSubsystem->GetDebugVerbosity(DebugTag_Interaction));
        }
        else
        {
            UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, TEXT("❌ Failed to load InteractionPrompt widget class"), true, EDebugVerbosity::Error);
        }
    }

    if (!Widget_PreInteractionPromptClass)
    {
        Widget_PreInteractionPromptClass = LoadWidgetClass(
            InteractionDefaultsConstants::Widget_PreInteractionPromptPath
        );
    }

    if (!GetOwner() || !GetOwner()->GetRootComponent()) return;

    // Setup all sphere components
    if (PreInteractionSphereComponent)
    {
        float Radius = InteractionSubsystem ? InteractionSubsystem->GetPreInteractionDistance() 
                                            : InteractionDefaultsConstants::PreInteractionDistanceLoad;
        SetupInteractionSphereComponent(PreInteractionSphereComponent, Radius, DebugTag_Interaction, true);
        
        PreInteractionSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &UInteractorComponent::OnSphereOverlapBegin);
        PreInteractionSphereComponent->OnComponentEndOverlap.AddDynamic(this, &UInteractorComponent::OnSphereOverlapEnd);

        //Debug PreInteractionSphereComponent
        DebugOverlapSphereCheck(PreInteractionSphereComponent);
    }

    if (InteractionSphereComponent)
    {
        float Radius = InteractionSubsystem ? InteractionSubsystem->GlobalFullInteractionDistanceUI 
                                            : InteractionDefaultsConstants::InteractionDistanceUI;
        SetupInteractionSphereComponent(InteractionSphereComponent, Radius, DebugTag_Interaction, true);
        
        InteractionSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &UInteractorComponent::OnSphereOverlapBegin);
        InteractionSphereComponent->OnComponentEndOverlap.AddDynamic(this, &UInteractorComponent::OnSphereOverlapEnd);
    }

    if (PreInteractionComponentUI)
    {
        float Radius = InteractionSubsystem ? InteractionSubsystem->GlobalPreInteractionDistanceUI 
                                            : InteractionDefaultsConstants::PreInteractionDistanceUI;
        SetupInteractionSphereComponent(PreInteractionComponentUI, Radius, DebugTag_Interaction, true, true);
        
        PreInteractionComponentUI->OnComponentBeginOverlap.AddDynamic(this, &UInteractorComponent::OnPreInteractionComponentBeginOverlap);
        PreInteractionComponentUI->OnComponentEndOverlap.AddDynamic(this, &UInteractorComponent::OnPreInteractionComponentEndOverlap);
    }

    if (InteractionComponentUI)
    {
        float Radius = InteractionSubsystem ? InteractionSubsystem->GlobalFullInteractionDistanceUI 
                                            : InteractionDefaultsConstants::InteractionDistanceUI;
        SetupInteractionSphereComponent(InteractionComponentUI, Radius, DebugTag_Interaction, true, true);
        
        InteractionComponentUI->OnComponentBeginOverlap.AddDynamic(this, &UInteractorComponent::OnFullInteractionComponentBeginOverlap);
        InteractionComponentUI->OnComponentEndOverlap.AddDynamic(this, &UInteractorComponent::OnFullInteractionComponentEndOverlap);
    }
    //Setup Timer for updatefocus based on camera focus
    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (PC && PC->IsLocalController())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FocusUpdateTimerHandle,
            this,
            &UInteractorComponent::UpdateFocus,
            FocusUpdateInterval,
            true);
        //Add Debug Log
        FString InfoText = FString::Printf(TEXT("InteractorComponent: Timer for UpdateFocus is initialized and set for local player:%s"), *PC->GetName());
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, InfoText);
    }
    
    //Need to check Immediate Focused interactable
    UpdateFocus();

    // ✅ NEW: Start prediction cleanup timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            PredictionCleanupTimerHandle,
            this,
            &UInteractorComponent::CleanupTimedOutPredictions,
            0.5f,  // Check every 0.5 seconds
            true   // Loop
        );
    }

    
}

void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInteractorComponent::TryGetPlayerController()
{
    if (!OwnerPawn)
    {
        GetWorld()->GetTimerManager().ClearTimer(ControllerCheckTimer);
        return;
    }
    
    OwnerPlayerController = Cast<APlayerController>(OwnerPawn->GetController());
    
    if (OwnerPlayerController)
    {
        GetWorld()->GetTimerManager().ClearTimer(ControllerCheckTimer);
        UE_LOG(LogTemp, Log, TEXT("✅ InteractorComponent: PlayerController NOW ready for %s"), 
            *OwnerPawn->GetName());
    }
}

bool UInteractorComponent::TryGrabFocusedActor()
{
    if (!PhysicsGrabComponent.IsValid()) return false;
    
    AActor* Target = FocusedActor.Get();
    if (!Target) return false;
    
    if (!ShouldGrab(Target)) return false;
    
    // If already holding something, release it
    if (PhysicsGrabComponent->IsHoldingObject())
    {
        PhysicsGrabComponent->Release();
        return true;
    }
    
    // Try to grab
    return PhysicsGrabComponent->TryGrab();

}

bool UInteractorComponent::ShouldGrab(AActor* Actor) const
{
    if (!Actor) return false;
    if (!PhysicsGrabComponent.IsValid()) return false;
    
    // Check if implements physical interaction AND can be grabbed
    if (Actor->GetClass()->ImplementsInterface(UPhysicalInteractionInterface::StaticClass()))
    {
        return IPhysicalInteractionInterface::Execute_CanBeGrabbed(Actor);
    }
    
    return false;
}

UDebugSubsystem* UInteractorComponent::GetDebugSubsystem()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UDebugSubsystem>();
        }
    }
    return nullptr;
}

void UInteractorComponent::SetupInteractionSphereComponent(
    USphereComponent* SphereComponent,
    float Radius,
    const FGameplayTag& DebugTag,
    bool bCheckForInteractables,
    bool bIsforUI
    )
{
    if (!SphereComponent) return;

    // Attach and position
    if (GetOwner() && GetOwner()->GetRootComponent())
    {
        SphereComponent->AttachToComponent(
            GetOwner()->GetRootComponent(),
            FAttachmentTransformRules::SnapToTargetIncludingScale
        );
        SphereComponent->SetRelativeLocation(FVector::ZeroVector);
        SphereComponent->SetRelativeRotation(FRotator::ZeroRotator);
    }

    // Configure collision
    if (!bIsforUI)
    {
        SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SphereComponent->SetCollisionObjectType(ECC_Pawn);
        SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
        SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
        SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
        SphereComponent->SetGenerateOverlapEvents(true);
    }
    else
    {

        // ✅ FIX: UI spheres need collision enabled to track overlaps for prediction!
        // Don't disable based on IsLocallyControlled
        
        SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SphereComponent->SetCollisionObjectType(ECC_Pawn);
        SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
        SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
        SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
        SphereComponent->SetGenerateOverlapEvents(true);
        //SphereComponent->SetAbsolute(false,false,false);
        
    }
    
    // Register and set radius
    SphereComponent->RegisterComponent();
    SphereComponent->SetSphereRadius(Radius);
    //Removed ShapeColorParam DebugDataAsset in DebugSubsystem will handle it!
    //SphereComponent->ShapeColor = Color;

    // ✅ Setup debug visualization - this handles color, visibility, and logging
    UDebugSubsystem::SetupCollisionVisualization(this, SphereComponent, DebugTag, true);

    // Check for initial overlaps
    if (bCheckForInteractables)
    {
        TArray<AActor*> InitialOverlaps;
        SphereComponent->GetOverlappingActors(InitialOverlaps);

        for (AActor* Actor : InitialOverlaps)
        {
            if (Actor && Actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass())
                && IInteractableInterface::Execute_GetInteractionEnabled(Actor)
                && IInteractableInterface::Execute_IsCurrentlyInteractable(Actor))
            {
                OverlappingInteractables.Add(Actor);
                IInteractableInterface::Execute_OnPlayerNearbyInit(Actor, Cast<APawn>(GetOwner()), true);
                
                UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
                    FString::Printf(TEXT("%s initially overlapping with: %s"), 
                    *SphereComponent->GetName(), *Actor->GetName()), false, EDebugVerbosity::Info);
            }
        }
    }

    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("%s radius: %f"), *SphereComponent->GetName(), SphereComponent->GetUnscaledSphereRadius()),
        false, EDebugVerbosity::Info);
}

TSubclassOf<UUserWidget> UInteractorComponent::LoadWidgetClass(const TCHAR* WidgetPath)
{
    ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(WidgetPath);
    
    if (WidgetClassFinder.Succeeded())
    {
        return WidgetClassFinder.Class;
    }
    
    UE_LOG(LogDebugSystem, Warning, TEXT("Failed to load widget: %s"), WidgetPath);
    return nullptr;
}

void UInteractorComponent::Client_SetWidgetVisibility_Implementation(UWidget* Widget, const ESlateVisibility Visibility)
{
    Widget->SetVisibility(Visibility);
    FString VisString = StaticEnum<ESlateVisibility>()->GetNameStringByValue((int64)Visibility);

    UE_LOG(LogDebugSystem, Log, TEXT("Visibility of Widget %s is set to %s "), *Widget->GetName(), *VisString);    
}

void UInteractorComponent::Client_DestroyWidget_Implementation(UWidget* Widget)
{
    Widget->RemoveFromParent();
}

void UInteractorComponent::UpdateFocus()
{
    if (!InteractionSubsystem) return;

    if (!OwnerPawn) return;

    AActor* BestActorCandidate = InteractionSubsystem->GetBestInteractableForPlayer(OwnerPawn);
    SetFocusedActor(BestActorCandidate);
}

void UInteractorComponent::SetFocusedActor(AActor* NewFocus)
{
    // ✅ Log EVERY call to see what's happening
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("⚙️ SetFocusedActor called - Old: %s, New: %s"), 
            FocusedActor.IsValid() ? *FocusedActor->GetName() : TEXT("NULL"),
            NewFocus ? *NewFocus->GetName() : TEXT("NULL")),
        false, EDebugVerbosity::Info);

    // ✅ Early exit if same actor
    if (FocusedActor.Get() == NewFocus) 
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("⏭️ Same actor - skipping (early exit)"),
            false, EDebugVerbosity::Info);
        return;
    }

    // ========================================================================
    // STEP 1: Hide widgets for OLD focused actor
    // ========================================================================
    if (FocusedActor.IsValid())
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("🔵 Unfocusing: %s"), *FocusedActor->GetName()),
            false, EDebugVerbosity::Warning);

        // Hide both widget types for old focused actor
        Client_HideInteractionPrompt(FocusedActor.Get());
        Client_HidePreInteractionPrompt(FocusedActor.Get());

        // Fire unfocus event
        if (InteractionSubsystem)
        {
            InteractionSubsystem->OnInteractableUnfocused.Broadcast(FocusedActor.Get());
        }
    }

    // ========================================================================
    // STEP 2: Update focused actor
    // ========================================================================
    FocusedActor = NewFocus;

    // ========================================================================
    // STEP 3: Show widgets for NEW focused actor
    // ========================================================================
    if (FocusedActor.IsValid())
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("🟢 Focusing NEW actor: %s"), *FocusedActor->GetName()),
            false, EDebugVerbosity::Warning);

        // ✅ Check which sphere contains the actor
        bool bInInteractionUI = InteractionUIOverlappingActors.Contains(FocusedActor.Get());
        bool bInPreInteractionUI = PreInteractionUIOverlappingActors.Contains(FocusedActor.Get());

        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("   In InteractionUI sphere: %s"), bInInteractionUI ? TEXT("YES") : TEXT("NO")),
            false, EDebugVerbosity::Warning);
        
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("   In PreInteractionUI sphere: %s"), bInPreInteractionUI ? TEXT("YES") : TEXT("NO")),
            false, EDebugVerbosity::Warning);

        // ✅ Determine which widget to show based on which sphere the actor is in
        if (bInInteractionUI)
        {
            // In close range (interaction sphere) - show full interaction prompt
            Client_ShowInteractionPrompt(FocusedActor.Get());
            
            UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
                TEXT("✅ Called Client_ShowInteractionPrompt (close range)"),
                false, EDebugVerbosity::Warning);
        }
        else if (bInPreInteractionUI)
        {
            // In pre-interaction range - show dot/indicator only
            Client_ShowPreInteractionPrompt(FocusedActor.Get());
            
            UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
                TEXT("✅ Called Client_ShowPreInteractionPrompt (far range)"),
                false, EDebugVerbosity::Warning);
        }
        else
        {
            UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
                TEXT("⚠️ Actor NOT in any UI sphere - no widget shown"),
                true, EDebugVerbosity::Error);
        }

        // Fire focus event
        if (InteractionSubsystem)
        {
            InteractionSubsystem->OnInteractableFocused.Broadcast(FocusedActor.Get());
        }
    }
    else
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("🔵 No actor focused - all widgets hidden"),
            false, EDebugVerbosity::Warning);
    }
}

void UInteractorComponent::Client_ShowInteractionPrompt_Implementation(AActor* Interactable)
{
    // ✅ FIX: Check OwnerPawn first!
    if (!IsValid(OwnerPawn))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Client_ShowInteractionPrompt: OwnerPawn is NULL"));
        return;
    }

    if (!IsValid(Widget_InteractionPromptClass))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Client_ShowInteractionPrompt: Widget_InteractionPromptClass is NULL"));
        return;
    }
    
    // Get player controller
    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!IsValid(PC))
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ Client_ShowInteractionPrompt: No valid PlayerController for %s"), *OwnerPawn->GetName());
        return;
    }
    
    // Already showing? Just update it
    if (CurrentInteractionPrompt && CurrentInteractionPrompt->IsInViewport())
    {
        if (UWidget_InteractionPrompt* Prompt = Cast<UWidget_InteractionPrompt>(CurrentInteractionPrompt))
        {
            Prompt->SetPromptText(FText::FromString("Interact"));
        }
        return;
    }
    
    // Create new widget
    CurrentInteractionPrompt = CreateWidget<UUserWidget>(PC, Widget_InteractionPromptClass);
    if (CurrentInteractionPrompt)
    {
        CurrentInteractionPrompt->AddToViewport(100);
        
        if (UWidget_InteractionPrompt* Prompt = Cast<UWidget_InteractionPrompt>(CurrentInteractionPrompt))
        {
            Prompt->SetPromptText(FText::FromString("Interact"));
        }
        
        UE_LOG(LogTemp, Log, TEXT("✅ Client_ShowInteractionPrompt: Widget created"));
    }
}

void UInteractorComponent::Client_HideInteractionPrompt_Implementation(AActor* Interactable)
{
    if (!IsValid(OwnerPawn))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Client_ShowPreInteractionPrompt called with null OwnerPawn"));
        return;
    }

    // Sometimes controller isn’t yet replicated to the client at this moment
    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!IsValid(PC))
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ Client_ShowPreInteractionPrompt: No valid PlayerController yet for %s"), *OwnerPawn->GetName());
        return;
    }

    if (!IsValid(Widget_PreInteractionPromptClass))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Widget_PreInteractionPromptClass invalid"));
        return;
    }
    
    if (CurrentInteractionPrompt && CurrentInteractionPrompt->IsInViewport())
    {
        CurrentInteractionPrompt->RemoveFromParent();
        CurrentInteractionPrompt = nullptr;
    }
}


void UInteractorComponent::Client_HidePreInteractionPrompt_Implementation(AActor* Interactable)
{
    if (!IsValid(OwnerPawn))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Client_ShowPreInteractionPrompt called with null OwnerPawn"));
        return;
    }

    // Sometimes controller isn’t yet replicated to the client at this moment
    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!IsValid(PC))
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ Client_ShowPreInteractionPrompt: No valid PlayerController yet for %s"), *OwnerPawn->GetName());
        return;
    }

    if (!IsValid(Widget_PreInteractionPromptClass))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Widget_PreInteractionPromptClass invalid"));
        return;
    }
    
    if (CurrentPreInteractionPrompt && CurrentPreInteractionPrompt->IsInViewport())
    {
        CurrentPreInteractionPrompt->RemoveFromParent();
        CurrentPreInteractionPrompt = nullptr;
    }
}


void UInteractorComponent::Client_ShowPreInteractionPrompt_Implementation(AActor* Interactable)
{
    if (!IsValid(OwnerPawn))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Client_ShowPreInteractionPrompt called with null OwnerPawn"));
        return;
    }

    // Sometimes controller isn’t yet replicated to the client at this moment
    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!IsValid(PC))
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ Client_ShowPreInteractionPrompt: No valid PlayerController yet for %s"), *OwnerPawn->GetName());
        return;
    }

    if (!IsValid(Widget_PreInteractionPromptClass))
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Widget_PreInteractionPromptClass invalid"));
        return;
    }
    
    if (!Widget_PreInteractionPromptClass) return;
    
    // Already showing? Leave it
    if (CurrentPreInteractionPrompt && CurrentPreInteractionPrompt->IsInViewport())
    {
        return;
    }
    
    // Create new widget    
    CurrentPreInteractionPrompt = CreateWidget<UUserWidget>(PC, Widget_PreInteractionPromptClass);
    if (CurrentPreInteractionPrompt)
    {
        CurrentPreInteractionPrompt->AddToViewport(90); // Z-order lower than interaction prompt
        
        if (UWidget_PreInteraction* PrePrompt = Cast<UWidget_PreInteraction>(CurrentPreInteractionPrompt))
        {
            PrePrompt->SetDotSize(5.0f); // Or your default size
        }
    }
}


void UInteractorComponent::TryInteract()
{
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, 
        TEXT("🎮 TryInteract() CALLED"), false, EDebugVerbosity::Info);

    if (!bCanInteract)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, 
            TEXT("❌ bCanInteract = false"), false, EDebugVerbosity::Warning);
        return;
    }

    if (!OwnerPawn)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, 
            TEXT("❌ No OwnerPawn"), false, EDebugVerbosity::Error);
        return;
    }
    AActor* Target = FocusedActor.Get();
    AActor* CurrentBestCandidate = nullptr;
    bool bUsingFallback = false;
    
    // ========================================================================
    // Physics Grab Component - Implement this Mechanic First
    // ========================================================================
    
    if (PhysicsGrabComponent.IsValid() && PhysicsGrabComponent->IsHoldingObject())
    {
        PhysicsGrabComponent->Release();
        //return
        //uncomment above if interact only releases the current object.
    }
    if (ShouldGrab(Target))
    {
        TryGrabFocusedActor();
        return;
    }

    // ========================================================================
    // PRIMARY PATH - Use InteractionSubsystem
    // ========================================================================
    if (InteractionSubsystem)
    {
        CurrentBestCandidate = InteractionSubsystem->GetBestInteractableForPlayer(OwnerPawn);
        SetFocusedActor(CurrentBestCandidate);
    }
    
    // ========================================================================
    // FALLBACK PATH - Check overlapping interactables directly
    // ========================================================================
    if (!CurrentBestCandidate)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("⚠️ No candidate from subsystem, checking fallback overlaps..."),
            false, EDebugVerbosity::Warning);
        
        for (AActor* OverlapActor : OverlappingInteractables)
        {
            if (!OverlapActor || !IsValid(OverlapActor)) continue;
            
            if (OverlapActor->Implements<UInteractableInterface>())
            {
                if (IInteractableInterface::Execute_GetInteractionEnabled(OverlapActor) &&
                    IInteractableInterface::Execute_IsCurrentlyInteractable(OverlapActor))
                {
                    CurrentBestCandidate = OverlapActor;
                    bUsingFallback = true;
                    
                    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
                        FString::Printf(TEXT("✅ FALLBACK: Found overlapping interactable: %s"), 
                            *OverlapActor->GetName()),
                        false, EDebugVerbosity::Warning);
                    break;
                }
            }
        }
    }
    
    if (!CurrentBestCandidate)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("❌ No valid interactable candidate found (primary or fallback)"),
            true, EDebugVerbosity::Warning);
        return;
    }
    
    // ========================================================================
    // CLIENT PATH - WITH PREDICTION
    // ========================================================================
    if (!OwnerPawn->HasAuthority() && OwnerPawn->IsLocallyControlled())
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("📱 CLIENT: Processing %s interaction with %s"), 
                bUsingFallback ? TEXT("FALLBACK") : TEXT("predicted"),
                *CurrentBestCandidate->GetName()),
            false, EDebugVerbosity::Info);
        
        if (bEnableClientPrediction)
        {
            int32 PredictionID = NextPredictionID++;
            
            FVector ViewLocation = FVector::ZeroVector;
            if (OwnerPlayerController)
            {
                FRotator ViewRotation;
                OwnerPlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
            }
            
            UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
                FString::Printf(TEXT("🔮 Executing PREDICTED interaction (ID: %d) from position: %s"), 
                    PredictionID, *ViewLocation.ToString()),
                false, EDebugVerbosity::Warning);
            
            ExecutePredictedEffects(CurrentBestCandidate, PredictionID);
            Server_Interact_Predicted(CurrentBestCandidate, ViewLocation, PredictionID);
        }
        else
        {
            Client_HideInteractionPrompt(CurrentBestCandidate);
            Client_HidePreInteractionPrompt(CurrentBestCandidate);
            Server_Interact(CurrentBestCandidate);
        }
        
        return;
    }

    // ========================================================================
    // SERVER PATH - NO PREDICTION NEEDED
    // ========================================================================
    if (OwnerPawn->HasAuthority())
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, 
            FString::Printf(TEXT("🖥️ SERVER: Executing %s interaction locally"),
                bUsingFallback ? TEXT("FALLBACK") : TEXT("normal")),
            false, EDebugVerbosity::Info);
        
        Server_Interact(CurrentBestCandidate);
        return;
    }
}
void UInteractorComponent::ValidateAndExecutePredictedInteraction(AActor* ServerFoundTarget, int32 PredictionID)
{
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("🖥️ SERVER: Validating prediction %d for target: %s"), 
            PredictionID, ServerFoundTarget ? *ServerFoundTarget->GetName() : TEXT("NULL")),
        false, EDebugVerbosity::Info);

    // ========================================================================
    // VALIDATION 1: Basic Checks
    // ========================================================================
    
    if (!OwnerPawn || !OwnerPawn->HasAuthority())
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("❌ SERVER: No authority or no pawn"),
            false, EDebugVerbosity::Warning);
        
        if (PredictionID >= 0)
            Client_RejectPrediction(PredictionID);
        return;
    }
    
    if (!ServerFoundTarget)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("❌ SERVER: No target found in trace"),
            false, EDebugVerbosity::Warning);
        
        if (PredictionID >= 0)
            Client_RejectPrediction(PredictionID);
        return;
    }

    // ========================================================================
    // VALIDATION 2: Interface Check
    // ========================================================================
    
    if (!ServerFoundTarget->Implements<UInteractableInterface>())
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("❌ SERVER: Actor %s doesn't implement IInteractableInterface"), 
                *ServerFoundTarget->GetName()),
            false, EDebugVerbosity::Warning);
        
        if (PredictionID >= 0)
            Client_RejectPrediction(PredictionID);
        return;
    }

    // ========================================================================
    // VALIDATION 3: Distance Check
    // ========================================================================
    
    const float MaxDist = 300.f;
    float DistSq = FVector::DistSquared(OwnerPawn->GetActorLocation(), ServerFoundTarget->GetActorLocation());
    
    if (DistSq > FMath::Square(MaxDist))
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("❌ SERVER: Target out of range (%.2f > %.2f)"), 
                FMath::Sqrt(DistSq), MaxDist),
            false, EDebugVerbosity::Warning);
        
        if (PredictionID >= 0)
            Client_RejectPrediction(PredictionID);
        return;
    }

    // ========================================================================
    // VALIDATION 4: Interactable State
    // ========================================================================
    
    if (!IInteractableInterface::Execute_GetInteractionEnabled(ServerFoundTarget))
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("❌ SERVER: Interaction disabled"),
            false, EDebugVerbosity::Warning);
        
        if (PredictionID >= 0)
            Client_RejectPrediction(PredictionID);
        return;
    }
    
    if (!IInteractableInterface::Execute_IsCurrentlyInteractable(ServerFoundTarget))
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("❌ SERVER: Not currently interactable"),
            false, EDebugVerbosity::Warning);
        
        if (PredictionID >= 0)
            Client_RejectPrediction(PredictionID);
        return;
    }

    // ========================================================================
    // VALIDATION PASSED - EXECUTE INTERACTION
    // ========================================================================
    
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("✅ SERVER: Validation passed - executing interaction on %s"), 
            *ServerFoundTarget->GetName()),
        false, EDebugVerbosity::Info);
    
    // Get player controller for interaction
    if (!OwnerPlayerController)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("❌ SERVER: No player controller"),
            false, EDebugVerbosity::Warning);
        
        if (PredictionID >= 0)
            Client_RejectPrediction(PredictionID);
        return;
    }
    
    // Execute the actual interaction
    IInteractableInterface::Execute_OnInteract(ServerFoundTarget, OwnerPlayerController);
    
    UE_LOG(LogTemp, Warning, TEXT("🗑️ SERVER: Successfully executed interaction on %s"), 
        *ServerFoundTarget->GetName());
    
    // Clear focus and hide widgets
    if (GetFocusedActor() == ServerFoundTarget)
    {
        SetFocusedActor(nullptr);
        UpdateFocus();
    }
    
    Multicast_HideWidgetsForActor(ServerFoundTarget);
    
    // ========================================================================
    // CONFIRM PREDICTION TO CLIENT
    // ========================================================================
    
    if (PredictionID >= 0)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("✅ SERVER: Confirming prediction %d"), PredictionID),
            false, EDebugVerbosity::Info);
        
        Client_ConfirmPrediction(PredictionID);
    }
}

bool UInteractorComponent::Server_Interact_Validate(AActor* ClientFocusedActor)
{
    // Reject null or destroyed actors
    if (!ClientFocusedActor || !IsValid(ClientFocusedActor))
    {
        return false;
    }
    
    // Reject if too far (basic distance check)
    if (OwnerPawn)
    {
        float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), ClientFocusedActor->GetActorLocation());
        if (Distance > InteractionDefaultsConstants::PreInteractionDistanceUI) // 1.5x tolerance for lag
        {
            return false;
        }
    }
    
    return true;
}

bool UInteractorComponent::Server_Interact_Predicted_Validate(AActor* ClientFocusedActor,
    FVector_NetQuantize10 ClientViewLocation, int32 PredictionID)
{
    // Same validation
    if (!ClientFocusedActor || !IsValid(ClientFocusedActor))
    {
        return false;
    }
    
    // Validate distance from client's reported location
    float Distance = FVector::Dist(FVector(ClientViewLocation), ClientFocusedActor->GetActorLocation());
    if (Distance > InteractionDefaultsConstants::PreInteractionDistanceUI)
    {
        return false;
    }
    
    return true;
}

void UInteractorComponent::Server_Interact_Implementation(AActor* ClientFocusedActor)
{
    if (!OwnerPawn)
    {
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
          TEXT("❌ SERVER_Interact: No OwnerPawn"), 
          true, EDebugVerbosity::Error);
       return;
    }

    if (!OwnerPawn->HasAuthority())
    {
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
          TEXT("🚫 SERVER_Interact called on non-authority instance"), 
          false, EDebugVerbosity::Warning);
       return;
    }

    if (!ClientFocusedActor || !IsValid(ClientFocusedActor))
    {
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
          TEXT("❌ SERVER_Interact: Invalid ClientFocusedActor"), 
          false, EDebugVerbosity::Warning);
       return;
    }

    if (!ClientFocusedActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
    {
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
          FString::Printf(TEXT("❌ SERVER_Interact: %s does not implement InteractableInterface"), 
             *ClientFocusedActor->GetName()), 
          false, EDebugVerbosity::Warning);
       return;
    }

    const float MaxDist = 300.f;
    if (FVector::DistSquared(OwnerPawn->GetActorLocation(), ClientFocusedActor->GetActorLocation()) > FMath::Square(MaxDist))
    {
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
          FString::Printf(TEXT("❌ SERVER_Interact: %s out of range"), *ClientFocusedActor->GetName()), 
          false, EDebugVerbosity::Warning);
       return;
    }

    if (!IInteractableInterface::Execute_GetInteractionEnabled(ClientFocusedActor)
       || !IInteractableInterface::Execute_IsCurrentlyInteractable(ClientFocusedActor))
    {
       UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
          FString::Printf(TEXT("❌ SERVER_Interact: %s not interactable"), *ClientFocusedActor->GetName()), 
          false, EDebugVerbosity::Warning);
       return;
    }

    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
       FString::Printf(TEXT("✅ SERVER_Interact: Executing OnInteract on %s"), 
          *ClientFocusedActor->GetName()), 
       false, EDebugVerbosity::Warning);

    // ✅ AGGRESSIVE DEBUG BEFORE EXECUTE
    UE_LOG(LogTemp, Error, TEXT("🔥🔥🔥 BEFORE Execute_OnInteract on %s 🔥🔥🔥"), *ClientFocusedActor->GetName());
    
    // Check if function exists
    UFunction* OnInteractFunc = ClientFocusedActor->FindFunction(FName("OnInteract"));
    if (OnInteractFunc)
    {
       UE_LOG(LogTemp, Error, TEXT("✅ OnInteract function EXISTS on %s"), *ClientFocusedActor->GetName());
    }
    else
    {
       UE_LOG(LogTemp, Error, TEXT("❌ OnInteract function NOT FOUND on %s"), *ClientFocusedActor->GetName());
    }

    // ✅ Execute interaction (this may destroy the actor)
    IInteractableInterface::Execute_OnInteract(ClientFocusedActor, OwnerPawn->GetController());

    // ✅ AGGRESSIVE DEBUG AFTER EXECUTE
    UE_LOG(LogTemp, Error, TEXT("🔥🔥🔥 AFTER Execute_OnInteract on %s 🔥🔥🔥"), *ClientFocusedActor->GetName());

    // 💡 CRITICAL FIX: Immediately clear focus and find the next interactable if the one we just used
    // was the focused actor. This prevents the stale reference issue if the actor was destroyed.
    if (FocusedActor.Get() == ClientFocusedActor)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("🔄 SERVER: Clearing focused actor after successful interaction."));
            
        // 1. Clear the reference by unfocusing the destroyed/used actor
        SetFocusedActor(nullptr); 
        
        // 2. Force an immediate check for a new focus (don't wait for the timer)
        UpdateFocus();
    }
    
    // ✅ Tell OTHER clients to hide their widgets
    Multicast_HideWidgetsForActor(ClientFocusedActor);
}

void UInteractorComponent::Multicast_HideWidgetsForActor_Implementation(AActor* Actor)
{
    if (!Actor)
        return;

    if (!OwnerPawn)
        return;

    // ✅ Only simulated proxies (other clients) should hide
    if (OwnerPawn->GetLocalRole() == ROLE_SimulatedProxy)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("📡 SIMULATED PROXY: Hiding widgets for %s"), *Actor->GetName()),
            true, EDebugVerbosity::Info);

        Client_HideInteractionPrompt(Actor);
        Client_HidePreInteractionPrompt(Actor);
    }
}

void UInteractorComponent::Client_PerformInteraction_Implementation(AActor* Actor)
{
    if (!Actor) return;
    
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("📱 CLIENT: Hiding widgets for: %s"), *Actor->GetName()),
        true, EDebugVerbosity::Warning);
    
    Client_HideInteractionPrompt(Actor);
    Client_HidePreInteractionPrompt(Actor);
}

void UInteractorComponent::Multicast_PerformInteraction_Implementation(AActor* Actor)
{
    if (!OwnerPawn || !Actor)
        return;

    if (OwnerPawn->GetLocalRole() == ROLE_SimulatedProxy)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("📡 Simulated proxy updated visuals for %s"), 
                *Actor->GetName()), 
            true, EDebugVerbosity::Info);

        Client_HideInteractionPrompt(Actor);
        Client_HidePreInteractionPrompt(Actor);
    }
}

TArray<AActor*> UInteractorComponent::GetOverlappingInteractables() const
{
    return OverlappingInteractables.Array();
}

void UInteractorComponent::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass())) return;

    if (OverlappedComp == PreInteractionSphereComponent)
    {
        OverlappingInteractables.Add(OtherActor);
        IInteractableInterface::Execute_OnPlayerNearbyInit(OtherActor, Cast<APawn>(GetOwner()), true);
        
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("PreInteractionSphere overlap BEGIN: %s"), *OtherActor->GetName()), false, EDebugVerbosity::Info);
    }
    else if (OverlappedComp == InteractionSphereComponent)
    {
        // Handle interaction sphere logic
    }
}

void UInteractorComponent::OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || !OtherActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass())) return;

    if (OverlappedComp == PreInteractionSphereComponent)
    {
        OverlappingInteractables.Remove(OtherActor);
        IInteractableInterface::Execute_OnPlayerNearbyInit(OtherActor, Cast<APawn>(GetOwner()), false);
        
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("PreInteractionSphere overlap END: %s"), *OtherActor->GetName()), false, EDebugVerbosity::Info);
    }
    else if (OverlappedComp == InteractionSphereComponent)
    {
        // Handle interaction sphere logic
    }
}

void UInteractorComponent::OnPreInteractionComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass())) return;

    bool IsCurrentlyInteractable = IInteractableInterface::Execute_IsCurrentlyInteractable(OtherActor) 
                                 && IInteractableInterface::Execute_GetInteractionEnabled(OtherActor);
    
    if (IsCurrentlyInteractable)
    {
        PreInteractionUIOverlappingActors.Add(OtherActor);
        IInteractableInterface::Execute_OnPreInteractionEntered(OtherActor, Cast<APawn>(GetOwner()));
        Client_ShowPreInteractionPrompt(OtherActor);
        FString InfoText = FString::Printf(TEXT("PreInteractionUI overlap BEGIN: redius:%f %s for Pawn:%s"), PreInteractionComponentUI->GetScaledSphereRadius(),*OtherActor->GetName(), *GetOwner()->GetName());
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, InfoText);

        //Step 2: hide the preinteractionui if it is already overlapping with interactionui.
        TArray<AActor*> OverlappingActors; 

        if (InteractionComponentUI)
        {
            // The arguments are: (ArrayReference, ClassFilter)
            InteractionComponentUI->GetOverlappingActors(OverlappingActors, AActor::StaticClass());

            // 3. Now you can check if the array has any results
            if (OverlappingActors.Num() > 0)
            {
                // Logic for handling overlaps goes here
                if (OverlappingActors.Contains(OtherActor))
                {
                    Client_HidePreInteractionPrompt(OtherActor);
                }
            }
        }
    }
}

void UInteractorComponent::OnFullInteractionComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepHitResult)
{
    // ✅ DEBUG: Check why overlap is firing
    if (InteractionComponentUI)
    {
        float SphereRadius = InteractionComponentUI->GetScaledSphereRadius();
        FVector SphereCenter = InteractionComponentUI->GetComponentLocation();
        FVector ActorLoc = OtherActor ? OtherActor->GetActorLocation() : FVector::ZeroVector;
        float ActualDist = FVector::Dist(SphereCenter, ActorLoc);
        
        // UE_LOG(LogInventoryInteractableSystem, Error, TEXT("🔴 OVERLAP FIRED:"));
        // UE_LOG(LogInventoryInteractableSystem, Error, TEXT("   Sphere Radius: %.2f"), SphereRadius);
        // UE_LOG(LogInventoryInteractableSystem, Error, TEXT("   Actual Distance: %.2f"), ActualDist);
        // UE_LOG(LogInventoryInteractableSystem, Error, TEXT("   Should overlap: %s"), ActualDist <= SphereRadius ? TEXT("YES") : TEXT("NO"));
    }

    if (!OtherActor || !OtherActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass())) return;

    bool IsCurrentlyInteractable = IInteractableInterface::Execute_IsCurrentlyInteractable(OtherActor) 
                                 && IInteractableInterface::Execute_GetInteractionEnabled(OtherActor);
    
    if (IsCurrentlyInteractable)
    {
        InteractionUIOverlappingActors.Add(OtherActor);
        UE_LOG(LogInventoryInteractableSystem, Log, TEXT("➕ ADDED to InteractionUIOverlappingActors: %s (Set size: %d)"), *OtherActor->GetName(), InteractionUIOverlappingActors.Num());

        //Keep focus up-to-date
        UpdateFocus();
        IInteractableInterface::Execute_OnFullInteractionEntered(OtherActor, Cast<APawn>(GetOwner()));
        FString InfoText = FString::Printf(TEXT("FullInteractionUI overlap BEGIN: radius:%f, Interactable:%s, Pawn:%s"), InteractionComponentUI->GetScaledSphereRadius(), *OtherActor->GetName(), *GetOwner()->GetName());
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            InfoText);
        
        Client_ShowInteractionPrompt(OtherActor);
        
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("Full Interaction Prompt is shown for Player:%s by interactable:%s"), *this->GetOwner()->GetName(),*OtherActor->GetName()),
            false, EDebugVerbosity::Info);
        Client_HidePreInteractionPrompt(OtherActor);
    }
}

void UInteractorComponent::OnPreInteractionComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || !OtherActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass())) return;

    PreInteractionUIOverlappingActors.Remove(OtherActor);
    IInteractableInterface::Execute_OnPreInteractionExited(OtherActor, Cast<APawn>(GetOwner()));
    
    Client_HidePreInteractionPrompt(OtherActor);
    
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("PreInteractionUI overlap END: %s"), *OtherActor->GetName()), false, EDebugVerbosity::Info);
}

void UInteractorComponent::OnFullInteractionComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || !OtherActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass())) return;

    InteractionUIOverlappingActors.Remove(OtherActor);
    //Keep focus up-to-date
    UpdateFocus();
    IInteractableInterface::Execute_OnFullInteractionExited(OtherActor, Cast<APawn>(GetOwner()));
    
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("FullInteractionUI overlap END: %s"), *OtherActor->GetName()), false, EDebugVerbosity::Info);
    
    Client_HideInteractionPrompt(OtherActor);
    //Step 2: Show PreinteractionWidgets for all other interactables within range upon endoverlap...
    TArray<AActor*> CurrentPreInteractionUIOverlappingActors = GetPreInteractionUIOverlappingActors();
    if (CurrentPreInteractionUIOverlappingActors.Num()>0)
    {
        for (int32 i = 0; i < CurrentPreInteractionUIOverlappingActors.Num(); i++)
        {
            if (i <= InteractionDefaultsConstants::MaxSimultaneouslyLoadedItems && i<<CurrentPreInteractionUIOverlappingActors.Num())
            {
                Client_ShowPreInteractionPrompt(CurrentPreInteractionUIOverlappingActors[i]);
            }
        }
        
    }
        
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("Full Interaction Prompt is Hidden for Player:%s by interactable:%s"), *this->GetOwner()->GetName(),*OtherActor->GetName()),
        false, EDebugVerbosity::Info);
}

// InteractorComponent.cpp

void UInteractorComponent::DebugOverlapSphereCheck(UPrimitiveComponent* SphereCollisionComponent)
{
    if (!SphereCollisionComponent)
    {
        UE_LOG(LogDebugSystem, Error, TEXT("❌ SphereCollisionComponent is NULL!"));
        return;
    }
    
    // Immediate check
    TArray<AActor*> ImmediateCheckActors;
    SphereCollisionComponent->GetOverlappingActors(ImmediateCheckActors);
    
    UE_LOG(LogDebugSystem, Error, TEXT("═══════════════════════════════════════"));
    UE_LOG(LogDebugSystem, Error, TEXT("🔴 IMMEDIATE CHECK: %d actors"), ImmediateCheckActors.Num());
    
    for (int32 i = 0; i < FMath::Min(10, ImmediateCheckActors.Num()); i++)
    {
        if (ImmediateCheckActors[i])
        {
            UE_LOG(LogDebugSystem, Error, TEXT("   %d. %s"), i+1, *ImmediateCheckActors[i]->GetName());
        }
    }
    
    // Store count for lambda (must capture by value!)
    int32 ImmediateCount = ImmediateCheckActors.Num();
    
    // Delayed check
    
    GetWorld()->GetTimerManager().SetTimer(
        TH_DelayedDebugTimer,
        [this, SphereCollisionComponent, ImmediateCount]() // Capture count by value
        {
            TArray<AActor*> DelayedCheck;
            SphereCollisionComponent->GetOverlappingActors(DelayedCheck);
            
            UE_LOG(LogDebugSystem, Error, TEXT("═══════════════════════════════════════"));
            UE_LOG(LogDebugSystem, Error, TEXT("🟢 DELAYED CHECK (0.5s later): %d actors"), DelayedCheck.Num());
            
            for (int32 i = 0; i < FMath::Min(10, DelayedCheck.Num()); i++)
            {
                if (DelayedCheck[i])
                {
                    UE_LOG(LogDebugSystem, Error, TEXT("   %d. %s"), i+1, *DelayedCheck[i]->GetName());
                }
            }
            
            int32 Difference = DelayedCheck.Num() - ImmediateCount;
            UE_LOG(LogDebugSystem, Error, TEXT("═══════════════════════════════════════"));
            UE_LOG(LogDebugSystem, Error, TEXT("❌ DIFFERENCE: %d actors were MISSING!"), Difference);
            UE_LOG(LogDebugSystem, Error, TEXT("═══════════════════════════════════════"));
        },
        0.5f,
        false
    );
}


APawn* UInteractorComponent::OnPossess(APawn* InPawn)
{    
    if (Cast<APawn>(InPawn))
    {
        OwnerPawn = InPawn;
        return InPawn;
    }
    else return nullptr;
}

void UInteractorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up widgets
    if (CurrentInteractionPrompt && CurrentInteractionPrompt->IsInViewport())
    {
        CurrentInteractionPrompt->RemoveFromParent();
    }
    
    if (CurrentPreInteractionPrompt && CurrentPreInteractionPrompt->IsInViewport())
    {
        CurrentPreInteractionPrompt->RemoveFromParent();
    }
    // Cleanup timer
    if (FocusUpdateTimerHandle.IsValid())
        GetWorld()->GetTimerManager().ClearTimer(FocusUpdateTimerHandle);
    
    // ✅ NEW: Clear prediction timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PredictionCleanupTimerHandle);
    }
    
    // Clear pending predictions
    PendingPredictions.Empty();
    
    Super::EndPlay(EndPlayReason);

}

void UInteractorComponent::ExecutePredictedEffects(AActor* Target, int32 PredictionID)
{
    if (!Target)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, 
            TEXT("❌ ExecutePredictedEffects: Null target"), 
            true, EDebugVerbosity::Error);
        return;
    }

    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, 
        FString::Printf(TEXT("⚡ PREDICTED EFFECTS: %s (ID: %d)"), 
            *Target->GetName(), PredictionID),
        false, EDebugVerbosity::Warning);

    // ========================================================================
    // CACHE STATE FOR POTENTIAL ROLLBACK
    // ========================================================================
    
    FPredictedInteraction Prediction;
    Prediction.Target = Target;
    Prediction.PredictionTime = GetWorld()->GetTimeSeconds();
    Prediction.bConfirmed = false;
    
    // Cache current state
    Prediction.CachedState.bWasVisible = !Target->IsHidden();
    Prediction.CachedState.CachedTransform = Target->GetActorTransform();
    
    // Cache interactable state if implements interface
    if (Target->Implements<UInteractableInterface>())
    {
        Prediction.CachedState.bWasInteractable = 
            IInteractableInterface::Execute_IsCurrentlyInteractable(Target);
        
        // Let interactable cache custom state
        IInteractableInterface::Execute_CachePredictionState(Target, Prediction.CachedState);
    }
    
    // Store prediction
    PendingPredictions.Add(PredictionID, Prediction);

    // ========================================================================
    // EXECUTE PREDICTED EFFECTS (INSTANT FEEDBACK!)
    // ========================================================================
    
    // 1. Hide UI prompts (you were already doing this!)
    Client_HideInteractionPrompt(Target);
    Client_HidePreInteractionPrompt(Target);
    
    // 2. Play visual/audio effects on interactable
    if (Target->Implements<UInteractableInterface>())
    {
        // Call special "predicted" version that only does cosmetic effects
        IInteractableInterface::Execute_OnInteractPredicted(Target, OwnerPawn->GetController());
    }
    
    // 3. Play client-side feedback (optional - add your own effects here)
    // Example: Controller rumble, screen flash, etc.
    if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
    {
        // Add haptic feedback, camera shake, etc.
        // PC->ClientPlayForceFeedback(...);
    }

    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction, 
        FString::Printf(TEXT("✅ Predicted effects executed (ID: %d)"), PredictionID),
        false, EDebugVerbosity::Info);
}

void UInteractorComponent::Server_Interact_Predicted_Implementation(
    AActor* ClientFocusedActor, 
    FVector_NetQuantize10 ClientViewLocation,  // ← NEW parameter
    int32 PredictionID)
{
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("🖥️ SERVER: Validating prediction %d for %s (ClientPos: %s)"), 
            PredictionID, 
            ClientFocusedActor ? *ClientFocusedActor->GetName() : TEXT("NULL"),
            *ClientViewLocation.ToString()),  // ← Log client position
        false, EDebugVerbosity::Info);


    // ========================================================================
    // VALIDATION (Same as your current Server_Interact)
    // ========================================================================
    
    if (!OwnerPawn || !OwnerPawn->HasAuthority())
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("❌ SERVER_Interact_Predicted: Invalid authority"),
            true, EDebugVerbosity::Error);
        
        // Reject prediction
        Client_RejectPrediction(PredictionID);
        return;
    }

    if (!ClientFocusedActor || !IsValid(ClientFocusedActor))
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("❌ SERVER_Interact_Predicted: Invalid actor"),
            false, EDebugVerbosity::Warning);
        
        Client_RejectPrediction(PredictionID);
        return;
    }

    if (!ClientFocusedActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("❌ %s does not implement InteractableInterface"), 
                *ClientFocusedActor->GetName()),
            false, EDebugVerbosity::Warning);
        
        Client_RejectPrediction(PredictionID);
        return;
    }

    // Distance check - use CLIENT'S position at interaction time, not replicated position
    const float MaxDist = 300.f;
    float ActualDistance = FVector::Dist(ClientViewLocation, ClientFocusedActor->GetActorLocation());
    if (FVector::DistSquared(ClientViewLocation, ClientFocusedActor->GetActorLocation()) 
        > FMath::Square(MaxDist))
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("❌ %s out of range (%.1f units > %.1f max) - REJECTING"), 
                *ClientFocusedActor->GetName(), ActualDistance, MaxDist),
            false, EDebugVerbosity::Warning);
        
        Client_RejectPrediction(PredictionID);
        return;
    }

    // Interactable state check
    if (!IInteractableInterface::Execute_GetInteractionEnabled(ClientFocusedActor)
        || !IInteractableInterface::Execute_IsCurrentlyInteractable(ClientFocusedActor))
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("❌ %s not interactable"), *ClientFocusedActor->GetName()),
            false, EDebugVerbosity::Warning);
        
        Client_RejectPrediction(PredictionID);
        return;
    }

    // ========================================================================
    // VALIDATION PASSED - EXECUTE INTERACTION
    // ========================================================================
    
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("✅ SERVER: Prediction %d CONFIRMED - executing interaction"), 
            PredictionID),
        false, EDebugVerbosity::Warning);

    // Execute interaction (your existing logic)
    IInteractableInterface::Execute_OnInteract(ClientFocusedActor, OwnerPawn->GetController());

    // Clear focus if needed
    if (FocusedActor.Get() == ClientFocusedActor)
    {
        SetFocusedActor(nullptr);
        UpdateFocus();
    }
    
    // Tell other clients to hide widgets
    Multicast_HideWidgetsForActor(ClientFocusedActor);
    
    // ========================================================================
    // CONFIRM PREDICTION TO CLIENT
    // ========================================================================
    
    // Only send confirmation if client is remote (not listen server)
    if (!OwnerPawn->IsLocallyControlled())
    {
        Client_ConfirmPrediction(PredictionID);
    }
}

void UInteractorComponent::Client_ConfirmPrediction_Implementation(int32 PredictionID)
{
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("✅ CLIENT: Prediction %d CONFIRMED by server"), PredictionID),
        false, EDebugVerbosity::Warning);

    // Find prediction
    FPredictedInteraction* Prediction = PendingPredictions.Find(PredictionID);
    if (!Prediction)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            FString::Printf(TEXT("⚠️ Prediction %d not found (may have timed out)"), PredictionID),
            false, EDebugVerbosity::Warning);
        return;
    }

    // Mark as confirmed
    Prediction->bConfirmed = true;
    
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("🎉 Prediction %d successful - keeping predicted changes"), 
            PredictionID),
        false, EDebugVerbosity::Info);

    // Clean up (no rollback needed)
    PendingPredictions.Remove(PredictionID);
}

void UInteractorComponent::Client_RejectPrediction_Implementation(int32 PredictionID)
{
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("❌ CLIENT: Prediction %d REJECTED - ROLLING BACK"), 
            PredictionID),
        true, EDebugVerbosity::Error);

    // Rollback the prediction
    RollbackPrediction(PredictionID);
}

void UInteractorComponent::RollbackPrediction(int32 PredictionID)
{
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("🔄 ROLLBACK: Undoing prediction %d"), PredictionID),
        true, EDebugVerbosity::Warning);

    // Find prediction
    FPredictedInteraction* Prediction = PendingPredictions.Find(PredictionID);
    if (!Prediction)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("❌ ROLLBACK FAILED: Prediction not found"),
            true, EDebugVerbosity::Error);
        return;
    }

    AActor* Target = Prediction->Target.Get();
    if (!Target)
    {
        UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
            TEXT("❌ ROLLBACK FAILED: Target destroyed"),
            true, EDebugVerbosity::Error);
        PendingPredictions.Remove(PredictionID);
        return;
    }

    // ========================================================================
    // RESTORE CACHED STATE
    // ========================================================================
    
    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("⏪ Restoring state for %s"), *Target->GetName()),
        false, EDebugVerbosity::Info);

    // 1. Restore visibility
    Target->SetActorHiddenInGame(!Prediction->CachedState.bWasVisible);
    
    // 2. Restore transform
    Target->SetActorTransform(Prediction->CachedState.CachedTransform);
    
    // 3. Restore custom state via interface
    if (Target->Implements<UInteractableInterface>())
    {
        IInteractableInterface::Execute_RestorePredictionState(Target, Prediction->CachedState);
    }
    
    // 4. Re-show interaction prompts
    if (Prediction->CachedState.bWasInteractable)
    {
        // Only show if was previously interactable
        Client_ShowPreInteractionPrompt(Target);
    }
    
    // 5. Play rollback feedback (optional - show player their action was invalid)
    // PlayRollbackSound();
    // ShowRollbackMessage("Interaction failed - out of range");

    UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
        FString::Printf(TEXT("✅ ROLLBACK COMPLETE for prediction %d"), PredictionID),
        false, EDebugVerbosity::Warning);

    // Clean up
    PendingPredictions.Remove(PredictionID);
}

void UInteractorComponent::CleanupTimedOutPredictions()
{
    if (!GetWorld()) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TArray<int32> TimedOutPredictions;
    
    // Find timed-out predictions
    for (auto& Pair : PendingPredictions)
    {
        int32 ID = Pair.Key;
        FPredictedInteraction& Prediction = Pair.Value;
        
        float Age = CurrentTime - Prediction.PredictionTime;
        if (Age > PredictionTimeout && !Prediction.bConfirmed)
        {
            UDebugSubsystem::PrintDebug(this, DebugTag_Interaction,
                FString::Printf(TEXT("⏱️ Prediction %d timed out (%.2fs old) - rolling back"), 
                    ID, Age),
                false, EDebugVerbosity::Warning);
            
            TimedOutPredictions.Add(ID);
        }
    }
    
    // Rollback timed-out predictions
    for (int32 ID : TimedOutPredictions)
    {
        RollbackPrediction(ID);
    }
}