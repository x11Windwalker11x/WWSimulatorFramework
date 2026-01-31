#include "Utilities/Helpers/Tags/WWSharedFunctionLibrary.h"

#include "Windwalker_Productions_SharedDefaults.h"
#include "Debug/DebugSubsystem.h"

// ========================================================================
// TAG MATCHING
// ========================================================================

bool UWWSharedFunctionLibrary::DoesTagMatchContainer(const FGameplayTagContainer& Container, FGameplayTag GivenTag)
{
    if (!GivenTag.IsValid() || Container.IsEmpty())
    {
        return false;
    }
    
    FGameplayTagQuery Query = FGameplayTagQuery::MakeQuery_MatchAnyTags(Container);
    FGameplayTagContainer TestContainer;
    TestContainer.AddTag(GivenTag);
    
    return Query.Matches(TestContainer);
}

bool UWWSharedFunctionLibrary::DoTagsMatchBidirectional(FGameplayTag TagA, FGameplayTag TagB)
{
    if (!TagA.IsValid() || !TagB.IsValid())
    {
        return false;
    }
    
    return TagA.MatchesTag(TagB) || TagB.MatchesTag(TagA);
}

bool UWWSharedFunctionLibrary::DoesContainerHaveExactTag(const FGameplayTagContainer& Container, FGameplayTag Tag)
{
    if (!Tag.IsValid())
    {
        return false;
    }
    
    return Container.HasTagExact(Tag);
}

bool UWWSharedFunctionLibrary::DoesTagMatchAllInContainer(const FGameplayTagContainer& Container, FGameplayTag GivenTag)
{
    if (!GivenTag.IsValid() || Container.IsEmpty())
    {
        return false;
    }
    
    for (const FGameplayTag& ContainerTag : Container)
    {
        if (!GivenTag.MatchesTag(ContainerTag))
        {
            return false;
        }
    }
    
    return true;
}

// ========================================================================
// CONTAINER OPERATIONS
// ========================================================================

bool UWWSharedFunctionLibrary::DoContainersOverlap(const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB)
{
    if (ContainerA.IsEmpty() || ContainerB.IsEmpty())
    {
        return false;
    }
    
    for (const FGameplayTag& TagA : ContainerA)
    {
        for (const FGameplayTag& TagB : ContainerB)
        {
            if (TagA.MatchesTag(TagB) || TagB.MatchesTag(TagA))
            {
                return true;
            }
        }
    }
    
    return false;
}

bool UWWSharedFunctionLibrary::DoesContainerIncludeAll(const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB)
{
    if (ContainerB.IsEmpty())
    {
        return true;
    }
    
    if (ContainerA.IsEmpty())
    {
        return false;
    }
    
    for (const FGameplayTag& TagB : ContainerB)
    {
        bool bFound = false;
        for (const FGameplayTag& TagA : ContainerA)
        {
            if (TagB.MatchesTag(TagA))
            {
                bFound = true;
                break;
            }
        }
        
        if (!bFound)
        {
            return false;
        }
    }
    
    return true;
}

TArray<FGameplayTag> UWWSharedFunctionLibrary::FilterTagsByParent(const FGameplayTagContainer& Container, FGameplayTag ParentTag)
{
    TArray<FGameplayTag> Result;
    
    if (!ParentTag.IsValid())
    {
        return Result;
    }
    
    for (const FGameplayTag& Tag : Container)
    {
        if (Tag.MatchesTag(ParentTag))
        {
            Result.Add(Tag);
        }
    }
    
    return Result;
}

FGameplayTagContainer UWWSharedFunctionLibrary::RemoveTagsByParent(const FGameplayTagContainer& Container, FGameplayTag ParentTag)
{
    FGameplayTagContainer Result;
    
    if (!ParentTag.IsValid())
    {
        return Container;
    }
    
    for (const FGameplayTag& Tag : Container)
    {
        if (!Tag.MatchesTag(ParentTag))
        {
            Result.AddTag(Tag);
        }
    }
    
    return Result;
}

// ========================================================================
// QUERY OPERATIONS
// ========================================================================

FGameplayTagQuery UWWSharedFunctionLibrary::BuildQueryMatchAny(const FGameplayTagContainer& Container)
{
    return FGameplayTagQuery::MakeQuery_MatchAnyTags(Container);
}

FGameplayTagQuery UWWSharedFunctionLibrary::BuildQueryMatchAll(const FGameplayTagContainer& Container)
{
    return FGameplayTagQuery::MakeQuery_MatchAllTags(Container);
}

FGameplayTagQuery UWWSharedFunctionLibrary::BuildQueryMatchNone(const FGameplayTagContainer& Container)
{
    return FGameplayTagQuery::MakeQuery_MatchNoTags(Container);
}

// ========================================================================
// TAG ANALYSIS
// ========================================================================

FGameplayTag UWWSharedFunctionLibrary::GetParentTag(FGameplayTag Tag)
{
    if (!Tag.IsValid())
    {
        return FGameplayTag();
    }
    
    FString TagString = Tag.ToString();
    int32 LastDotIndex;
    
    if (TagString.FindLastChar('.', LastDotIndex))
    {
        FString ParentString = TagString.Left(LastDotIndex);
        return FGameplayTag::RequestGameplayTag(FName(*ParentString), false);
    }
    
    return FGameplayTag();
}

FGameplayTag UWWSharedFunctionLibrary::GetRootTag(FGameplayTag Tag)
{
    if (!Tag.IsValid())
    {
        return FGameplayTag();
    }
    
    FString TagString = Tag.ToString();
    int32 FirstDotIndex;
    
    if (TagString.FindChar('.', FirstDotIndex))
    {
        FString RootString = TagString.Left(FirstDotIndex);
        return FGameplayTag::RequestGameplayTag(FName(*RootString), false);
    }
    
    return Tag;
}

int32 UWWSharedFunctionLibrary::GetTagDepth(FGameplayTag Tag)
{
    if (!Tag.IsValid())
    {
        return 0;
    }
    
    FString TagString = Tag.ToString();
    int32 Depth = 1;
    
    for (int32 i = 0; i < TagString.Len(); i++)
    {
        if (TagString[i] == '.')
        {
            Depth++;
        }
    }
    
    return Depth;
}

bool UWWSharedFunctionLibrary::IsDirectChildOf(FGameplayTag ChildTag, FGameplayTag ParentTag)
{
    if (!ChildTag.IsValid() || !ParentTag.IsValid())
    {
        return false;
    }
    
    // Child must match parent
    if (!ChildTag.MatchesTag(ParentTag))
    {
        return false;
    }
    
    // Child depth must be exactly parent depth + 1
    int32 ChildDepth = GetTagDepth(ChildTag);
    int32 ParentDepth = GetTagDepth(ParentTag);
    
    return (ChildDepth == ParentDepth + 1);
}

// ========================================================================
// CONTAINER UTILITIES
// ========================================================================

int32 UWWSharedFunctionLibrary::GetTagCount(const FGameplayTagContainer& Container)
{
    return Container.Num();
}

bool UWWSharedFunctionLibrary::IsContainerEmpty(const FGameplayTagContainer& Container)
{
    return Container.IsEmpty();
}

TArray<FGameplayTag> UWWSharedFunctionLibrary::ContainerToArray(const FGameplayTagContainer& Container)
{
    TArray<FGameplayTag> Result;
    
    for (const FGameplayTag& Tag : Container)
    {
        Result.Add(Tag);
    }
    
    return Result;
}

FGameplayTagContainer UWWSharedFunctionLibrary::ArrayToContainer(const TArray<FGameplayTag>& TagArray)
{
    FGameplayTagContainer Result;
    
    for (const FGameplayTag& Tag : TagArray)
    {
        Result.AddTag(Tag);
    }
    
    return Result;
}

FGameplayTagContainer UWWSharedFunctionLibrary::MergeContainers(const FGameplayTagContainer& ContainerA, const FGameplayTagContainer& ContainerB)
{
    FGameplayTagContainer Result = ContainerA;
    Result.AppendTags(ContainerB);
    return Result;
}

// SharedFunctionLibrary.cpp

FGameplayTagContainer UWWSharedFunctionLibrary::BuildContainerFromArray(const TArray<FGameplayTag>& TagArray)
{
    FGameplayTagContainer Result;
    
    for (const FGameplayTag& Tag : TagArray)
    {
        if (Tag.IsValid())
        {
            Result.AddTag(Tag);
        }
    }
    
    return Result;
}

FGameplayTag UWWSharedFunctionLibrary::FindBestMatchingTag(const UObject* WorldContextObject, const TArray<FGameplayTag>& T_TagContainer,
    const FGameplayTag& QueryTag)
{
    if (!QueryTag.IsValid())
    {
        UDebugSubsystem::PrintDebug(WorldContextObject, WWDebugTags::Debug(), TEXT("QueryTag is invalid"),false , EDebugVerbosity::Warning);
        return FGameplayTag::EmptyTag;
    }
    if (T_TagContainer.IsEmpty())
    {
        UDebugSubsystem::PrintDebug(WorldContextObject, WWDebugTags::Debug(), TEXT("T_TagContainer is invalid"),false , EDebugVerbosity::Warning);
        return FGameplayTag::EmptyTag;
    }
    
    for (FGameplayTag Tag = QueryTag; Tag.IsValid(); Tag = Tag.RequestDirectParent() )
    {
        if (T_TagContainer.Contains(Tag))
        {
            return Tag;
        }
    }
    return FGameplayTag();
}
