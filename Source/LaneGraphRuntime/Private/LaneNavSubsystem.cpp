#include "LaneNavSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NodeDataAsset.h"

void ULaneNavSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Bind delegates
    FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ULaneNavSubsystem::HandlePostWorldInit);
    FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &ULaneNavSubsystem::HandleWorldTearDown);
}

void ULaneNavSubsystem::Deinitialize()
{
    // Always clean up delegates
    FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
    FWorldDelegates::OnWorldBeginTearDown.RemoveAll(this);

    Super::Deinitialize();
}

void ULaneNavSubsystem::TestNavData()
{
    if (CurrentNavData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d entries!"), CurrentNavData->Nodes.Num());
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("No Level Data Found!"));
    }
}

void ULaneNavSubsystem::HandlePostWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
    if (World && World->IsGameWorld())
    {
        FString MapName = FPackageName::GetShortName(World->GetOutermost()->GetName());
        MapName.RemoveFromStart(World->StreamingLevelsPrefix);
        const FString PackageName = FString::Printf(TEXT("%s_%s"), *UNodeDataAsset::OBJECT_PREFIX, *MapName);
        const FString AssetPath = FString::Printf(TEXT("%s%s.%s"),
            *UNodeDataAsset::ASSET_PATH,
            *PackageName,
            *PackageName);

        CurrentNavData = LoadObject<UNodeDataAsset>(nullptr, *AssetPath);
    }
}

void ULaneNavSubsystem::HandleWorldTearDown(UWorld* World)
{
    CurrentNavData = nullptr;
}