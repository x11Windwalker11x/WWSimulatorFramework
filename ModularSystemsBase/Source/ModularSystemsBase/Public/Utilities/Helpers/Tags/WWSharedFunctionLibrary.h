#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "WWSharedFunctionLibrary.generated.h"

/**
 * Shared function library for GameplayTag operations
 * Accessible across all plugins
 */
UCLASS()
class SHAREDDEFAULTS_API UWWSharedFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ========================================================================
    // TAG MATCHING (Hierarchy-Aware)
    // ========================================================================
    
    /**
     * Check if given tag matches any tag in container (hierarchy-aware)
     * Example: Camera.Mode.FPS.Custom matches container with Camera.Mode.FPS
     * @return True if given tag is child of or equal to any tag in container
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "matches contains hierarchy"))
    static bool DoesTagMatchContainer(const FGameplayTagContainer& Container, FGameplayTag GivenTag);
    
    /**
     * Check if two tags match in either direction (bidirectional)
     * Example: Camera.Mode.FPS matches Camera.Mode.FPS.Custom (and vice versa)
     * @return True if tags are in same hierarchy (parent-child relationship)
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "matches hierarchy bidirectional"))
    static bool DoTagsMatchBidirectional(FGameplayTag TagA, FGameplayTag TagB);
    
    /**
     * Check if container has exact tag match (no hierarchy)
     * Example: Only matches if container has exact tag, not parent
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "exact match contains"))
    static bool DoesContainerHaveExactTag(const FGameplayTagContainer& Container, FGameplayTag Tag);
    
    /**
     * Check if given tag matches all tags in container (AND logic)
     * @return True if given tag matches every tag in container
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "matches all and"))
    static bool DoesTagMatchAllInContainer(const FGameplayTagContainer& Container, FGameplayTag GivenTag);
    
    // ========================================================================
    // CONTAINER OPERATIONS
    // ========================================================================
    
    /**
     * Check if two containers have any matching tags (hierarchy-aware)
     * @return True if any tag from ContainerA matches any tag from ContainerB
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "overlap intersection any"))
    static bool DoContainersOverlap(const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB);
    
    /**
     * Check if ContainerA contains all tags from ContainerB (hierarchy-aware)
     * @return True if ContainerA has all tags from ContainerB (or their parents)
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "contains all subset"))
    static bool DoesContainerIncludeAll(const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB);
    
    /**
     * Get all tags from container that match given parent tag
     * Example: Get all Camera.Mode.* tags from container
     * @return Array of matching tags
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "filter find children"))
    static TArray<FGameplayTag> FilterTagsByParent(const FGameplayTagContainer& Container, FGameplayTag ParentTag);
    
    /**
     * Remove all tags from container that match given parent tag
     * Example: Remove all Camera.Mode.* tags
     * @return New container without matching tags
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "remove filter exclude"))
    static FGameplayTagContainer RemoveTagsByParent(const FGameplayTagContainer& Container, FGameplayTag ParentTag);
    
    // ========================================================================
    // QUERY OPERATIONS
    // ========================================================================
    
    /**
     * Build query that matches ANY tag in container (OR logic)
     * Use with Query.Matches(TestContainer)
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "build create any or"))
    static FGameplayTagQuery BuildQueryMatchAny(const FGameplayTagContainer& Container);
    
    /**
     * Build query that matches ALL tags in container (AND logic)
     * Use with Query.Matches(TestContainer)
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "build create all and"))
    static FGameplayTagQuery BuildQueryMatchAll(const FGameplayTagContainer& Container);
    
    /**
     * Build query that matches NONE of the tags in container (NOT logic)
     * Use with Query.Matches(TestContainer)
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Query", meta = (Keywords = "build create none not exclude"))
    static FGameplayTagQuery BuildQueryMatchNone(const FGameplayTagContainer& Container);
    
    // ========================================================================
    // TAG ANALYSIS
    // ========================================================================
    
    /**
     * Get parent tag (one level up)
     * Example: Camera.Mode.FPS.Custom → Camera.Mode.FPS
     * @return Parent tag, or empty tag if no parent
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "parent hierarchy up"))
    static FGameplayTag GetParentTag(FGameplayTag Tag);
    
    /**
     * Get root tag (top level)
     * Example: Camera.Mode.FPS.Custom → Camera
     * @return Root tag
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "root top hierarchy"))
    static FGameplayTag GetRootTag(FGameplayTag Tag);
    
    /**
     * Get tag depth (number of levels)
     * Example: Camera.Mode.FPS → 3 levels
     * @return Depth count (0 if invalid tag)
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "depth level count"))
    static int32 GetTagDepth(FGameplayTag Tag);
    
    /**
     * Check if tag is a direct child of parent (one level down only)
     * Example: Camera.Mode.FPS is direct child of Camera.Mode (TRUE)
     *          Camera.Mode.FPS.Custom is NOT direct child of Camera.Mode (FALSE)
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "direct child immediate"))
    static bool IsDirectChildOf(FGameplayTag ChildTag, FGameplayTag ParentTag);
    
    // ========================================================================
    // CONTAINER UTILITIES
    // ========================================================================
    
    /**
     * Get count of tags in container
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "count number size"))
    static int32 GetTagCount(const FGameplayTagContainer& Container);
    
    /**
     * Check if container is empty
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "empty null"))
    static bool IsContainerEmpty(const FGameplayTagContainer& Container);
    
    /**
     * Convert container to array
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "convert array list"))
    static TArray<FGameplayTag> ContainerToArray(const FGameplayTagContainer& Container);
    
    /**
     * Convert array to container
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "convert container"))
    static FGameplayTagContainer ArrayToContainer(const TArray<FGameplayTag>& TagArray);
    
    /**
     * Merge two containers (union)
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "merge combine union"))
    static FGameplayTagContainer MergeContainers(const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB);

    // SharedFunctionLibrary.h

    // In the CONTAINER UTILITIES section:

    /**
     * Build container from array of tags
     * Alias for ArrayToContainer (more descriptive name)
     */
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "build create container array"))
    static FGameplayTagContainer BuildContainerFromArray(const TArray<FGameplayTag>& TagArray);
    UFUNCTION(BlueprintPure, Category = "GameplayTags|Utility", meta = (Keywords = "Find BestMatching Tag"))
    static FGameplayTag FindBestMatchingTag(const UObject* WorldContextObject, const TArray<FGameplayTag>& T_Container, const FGameplayTag& Tag);

};

