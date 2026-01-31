#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "TagHelpers.generated.h"

/**
 * Static utilities for GameplayTag operations.
 * Moved from SharedDefaults - contains logic.
 */
UCLASS()
class MODULARSYSTEMSBASE_API UTagHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // === TAG MATCHING ===
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static bool DoesTagMatchContainer(const FGameplayTagContainer& Container, FGameplayTag GivenTag);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static bool DoTagsMatchBidirectional(FGameplayTag TagA, FGameplayTag TagB);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static bool DoesContainerHaveExactTag(const FGameplayTagContainer& Container, FGameplayTag Tag);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static bool DoesTagMatchAllInContainer(const FGameplayTagContainer& Container, FGameplayTag GivenTag);

    // === CONTAINER OPERATIONS ===
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static bool DoContainersOverlap(const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static bool DoesContainerIncludeAll(const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static TArray<FGameplayTag> FilterTagsByParent(const FGameplayTagContainer& Container, FGameplayTag ParentTag);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static FGameplayTagContainer RemoveTagsByParent(const FGameplayTagContainer& Container, FGameplayTag ParentTag);

    // === QUERY BUILDING ===
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static FGameplayTagQuery BuildQueryMatchAny(const FGameplayTagContainer& Container);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static FGameplayTagQuery BuildQueryMatchAll(const FGameplayTagContainer& Container);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static FGameplayTagQuery BuildQueryMatchNone(const FGameplayTagContainer& Container);

    // === TAG ANALYSIS ===
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static FGameplayTag GetParentTag(FGameplayTag Tag);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static FGameplayTag GetRootTag(FGameplayTag Tag);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static int32 GetTagDepth(FGameplayTag Tag);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static bool IsDirectChildOf(FGameplayTag ChildTag, FGameplayTag ParentTag);

    // === CONTAINER UTILITIES ===
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static int32 GetTagCount(const FGameplayTagContainer& Container);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static bool IsContainerEmpty(const FGameplayTagContainer& Container);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static TArray<FGameplayTag> ContainerToArray(const FGameplayTagContainer& Container);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static FGameplayTagContainer ArrayToContainer(const TArray<FGameplayTag>& TagArray);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static FGameplayTagContainer MergeContainers(const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB);
    
    UFUNCTION(BlueprintPure, Category = "Tag Helpers")
    static FGameplayTag FindBestMatchingTag(const UObject* WorldContextObject, const TArray<FGameplayTag>& TagArray, const FGameplayTag& Tag);

    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "build create container array"))
    static FGameplayTagContainer BuildContainerFromArray(const TArray<FGameplayTag>& TagArray);
};