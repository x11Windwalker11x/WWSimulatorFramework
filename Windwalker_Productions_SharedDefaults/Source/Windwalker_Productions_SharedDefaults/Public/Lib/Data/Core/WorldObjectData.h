// WorldObjectData.h
// Base struct for ALL world objects (items, interactables, devices, etc.)
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "WorldObjectData.generated.h"

/**
 * Base data shared by ALL world objects in the framework.
 * Items, Interactables, Devices all extend from this.
 * DataTable row base for unified object definitions.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWorldObjectData : public FTableRowBase
{
    GENERATED_BODY()

    // === IDENTITY ===
    
    /** Unique identifier for this object type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FName ID = NAME_None;

    /** Localized display name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FText DisplayName;

    /** Localized description */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FText Description;

    // === CLASSIFICATION ===
    
    /** Tags defining object type, category, behavior */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    FGameplayTagContainer BaseTags;

    // === VISUALS ===
    
    /** Icon for UI display */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    TSoftObjectPtr<UTexture2D> Icon;

    /** Mesh for world representation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    TSoftObjectPtr<UStaticMesh> EditorPreviewMesh_Soft;

    /** Mesh for world representation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    TSoftObjectPtr<USkeletalMesh> EditorPreviewSKMesh_Soft;

    //In-game mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    TSoftObjectPtr<UStaticMesh> Soft_SM_Mesh;
    //In-game SK_Mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    TSoftObjectPtr<USkeletalMesh> Soft_SK_Mesh;

    // === SPAWNING ===
    
    /** Actor class to spawn for this object */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TSoftClassPtr<AActor> ActorClass;

    // === VALIDATION ===
    
    bool IsValid() const
    {
        return !ID.IsNone();
    }

    bool operator==(const FWorldObjectData& Other) const
    {
        return ID == Other.ID;
    }

    friend uint32 GetTypeHash(const FWorldObjectData& Data)
    {
        return GetTypeHash(Data.ID);
    }

    
    // Utility functions (updated for soft references)
    bool HasValidMesh() const { return Soft_SM_Mesh.ToSoftObjectPath().IsValid() || Soft_SK_Mesh.ToSoftObjectPath().IsValid(); }

    
    // Check if assets are actually loaded in memory
    bool HasLoadedMesh() const { return Soft_SM_Mesh.IsValid() || Soft_SK_Mesh.IsValid(); }
};

/**
 * Base data shared by ALL DataTable Rows in the framework.
 * Items, Interactables, Devices Data Tables all extend from this.
 * DataTable row base for unified object definitions.
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FWorldObjectDataTableData : public FTableRowBase
{
    GENERATED_BODY()

    // === IDENTITY ===
    
    /** Unique identifier for this object type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FName ID = NAME_None;

    /** Localized display name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FText DisplayName;

    /** Localized description */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FText Description;

    // === CLASSIFICATION ===
    
    /** Tags defining object type, category, behavior */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    FGameplayTagContainer BaseTags;
    

    // === SPAWNING ===
    
    /** Mesh for world representation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    TSoftObjectPtr<UStaticMesh> EditorPreviewMesh_Soft;

    /** Mesh for world representation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    TSoftObjectPtr<USkeletalMesh> EditorPreviewSKMesh_Soft;

    //In-game mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    TSoftObjectPtr<UStaticMesh> Soft_SM_Mesh;
    //In-game SK_Mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
    TSoftObjectPtr<USkeletalMesh> Soft_SK_Mesh;
    
    /** Actor class to spawn for this object */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TSoftClassPtr<AActor> ActorClass;

    // === VALIDATION ===
    
    bool IsValid() const
    {
        return !ID.IsNone();
    }

    bool operator==(const FWorldObjectData& Other) const
    {
        return ID == Other.ID;
    }


};

/*
 *Main Datatable struct for pickupables: recipes, craftables, items 
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FPickupableDataTableData : public FWorldObjectDataTableData
{
    GENERATED_BODY()

    //PickUp Specific Data
    UPROPERTY(BlueprintReadWrite, Category = "Pickup | Config")
    bool bIsStolen = false;
    UPROPERTY(BlueprintReadWrite, Category = "Pickup | Config")
    float Quality = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Pickup | Config")
    float Durability = 1.0f;
    
    // === AUDIO (from old ItemData.h) ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> PickupSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> DropSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> UseSound;

    // === ANIMATION (from old ItemData.h) ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> PickupMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> DropMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> UseMontage;

    bool HasLoadedPickupAnimation() const { return PickupMontage.IsValid(); }
    bool HasLoadedPickupSound() const { return PickupSound.IsValid(); }
    bool HasPickupAnimation() const { return PickupMontage.ToSoftObjectPath().IsValid(); }
    bool HasPickupSound() const { return PickupSound.ToSoftObjectPath().IsValid(); }
};

/*
 *Mainstruct for dynamic pickupables: recipes, craftables...
 *items spawned in the world 
 */
USTRUCT(BlueprintType)
struct WINDWALKER_PRODUCTIONS_SHAREDDEFAULTS_API FPickupableData : public FWorldObjectData
{
    GENERATED_BODY()

    //Pickup Specific Data
    UPROPERTY(BlueprintReadWrite, Category = "Pickup | Config")
    bool bIsStolen = false;
    UPROPERTY(BlueprintReadWrite, Category = "Pickup | Config")
    float Quality = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Pickup | Config")
    float Durability = 1.0f;

    

};