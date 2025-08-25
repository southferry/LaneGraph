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
        for (ULaneNode* LN : CurrentNavData->Nodes)
        {
            UE_LOG(LogTemp, Warning, TEXT("Node ID: %s ## Node Vector: %s ## Original SS ID: %s ## Point Spacing: %f"),
                *LN->Id.ToString(EGuidFormats::DigitsWithHyphens),
                *LN->Position.ToString(),
                *LN->OriginalSplineId.ToString(EGuidFormats::DigitsWithHyphens),
                LN->NeighborDistance);
            for (FGuid PenId : LN->MediumPenalty)
            {
                UE_LOG(LogTemp, Warning, TEXT("Penalty Node ID: %s"), *PenId.ToString(EGuidFormats::DigitsWithHyphens));
            }
            for (FGuid NeighId : LN->Neighbors)
            {
                UE_LOG(LogTemp, Warning, TEXT("Neighbor Node ID: %s"), *NeighId.ToString(EGuidFormats::DigitsWithHyphens));
            }
        }
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