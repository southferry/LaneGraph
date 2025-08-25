#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"

class FDataAssetHelper
{
public:
    // Create or update a data asset at the given path
    template<typename T>
    static T* CreateOrUpdateDataAsset(const FString& AssetPath, const TFunction<void(T*)>& UpdateFunc)
    {
        if (!T::StaticClass()->IsChildOf(UDataAsset::StaticClass()))
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateOrUpdateDataAsset called with a class that is not a UDataAsset"));
            return nullptr;
        }

        // Split the asset path into package path and asset name
        FString PackageName = AssetPath;
        FString AssetName;
        PackageName.Split(TEXT("/"), nullptr, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

        // Try to load existing asset
        T* ExistingAsset = LoadObject<T>(nullptr, *AssetPath);
        if (ExistingAsset)
        {
            UpdateFunc(ExistingAsset);
            ExistingAsset->MarkPackageDirty();
            UE_LOG(LogTemp, Log, TEXT("Updated existing asset: %s"), *AssetPath);
            return ExistingAsset;
        }

        // Create package
        UPackage* Package = CreatePackage(*PackageName);
        if (!Package)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create package for asset: %s"), *AssetPath);
            return nullptr;
        }

        // Create the asset object
        T* NewAsset = NewObject<T>(Package, *AssetName, RF_Public | RF_Standalone);
        if (!NewAsset)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create new asset object: %s"), *AssetPath);
            return nullptr;
        }

        // Apply updates via lambda
        UpdateFunc(NewAsset);

        // Register asset
        FAssetRegistryModule::AssetCreated(NewAsset);
        Package->MarkPackageDirty();

        // === Convert Unreal package path to disk path correctly ===
        FString RelativePath = PackageName;
        RelativePath.RemoveFromStart(TEXT("/Game/")); // Remove virtual root
        FString FullPath = FPaths::ProjectContentDir() / RelativePath + TEXT(".uasset");

        // Save package
        FSavePackageArgs SaveArgs;
        SaveArgs.SaveFlags = SAVE_None;
        SaveArgs.bForceByteSwapping = false;
        SaveArgs.bWarnOfLongFilename = false;

        bool bSaved = UPackage::SavePackage(Package, NewAsset, *FullPath, SaveArgs);
        if (bSaved)
        {
            UE_LOG(LogTemp, Log, TEXT("Successfully created and saved asset: %s"), *FullPath);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to save asset: %s"), *FullPath);
        }

        return NewAsset;
    }
};