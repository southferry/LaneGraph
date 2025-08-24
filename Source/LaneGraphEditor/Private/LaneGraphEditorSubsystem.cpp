// LaneGraphEditorSubsystem.cpp

#include "LaneNode.h"
#include "StreetSplineActor.h"
#include "NodeDataAsset.h"

#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PackageTools.h"
#include "Editor.h"
#include "LaneGraphEditorSubsystem.h"

TArray<AStreetSplineActor*> ULaneGraphEditorSubsystem::GetStreetSplineActors()
{
    UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

    ULevel* Level = EditorWorld->GetCurrentLevel(); // Or get some specific level

    TArray<AStreetSplineActor*> SSActors;

    for (AActor* Actor : Level->Actors)
    {
        if (AStreetSplineActor* SSActor = Cast<AStreetSplineActor>(Actor))
        {
            SSActors.Add(SSActor);
        }
    }
    return SSActors;
}

void ULaneGraphEditorSubsystem::SaveNodeData()
{
    // Include the asset name at the end
    FString MapName = "default";
    if (GEditor && GEditor->GetEditorWorldContext().World())
    {
        UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
        MapName = EditorWorld->GetMapName();
    }
    const FString AssetPath = FString::Printf(TEXT("%s-%s"), 
        *UNodeDataAsset::DATA_STORAGE_PREFIX, 
        *MapName);

    // Create package
    UPackage* Package = CreatePackage(*AssetPath);

    // Create the asset inside that package
    UNodeDataAsset* Asset = NewObject<UNodeDataAsset>(
        Package,
        UNodeDataAsset::StaticClass(),
        *FPaths::GetBaseFilename(AssetPath), // "MyLaneGraph"
        RF_Public | RF_Standalone
    );

    Asset->Nodes.Empty();
    // Copy your node data
    for (const TPair<FGuid, ULaneNode*>& Element : Nodes)
    {
        ULaneNode* LaneNodeCopy = DuplicateObject<ULaneNode>(Element.Value, Asset);
        TTuple<FGuid, ULaneNode*> NodeCopy(LaneNodeCopy->Id, LaneNodeCopy);
        Asset->Nodes.Add(NodeCopy);
    }

    // Notify AssetRegistry
    FAssetRegistryModule::AssetCreated(Asset);

    // Mark dirty
    Asset->MarkPackageDirty();

    // Build the .uasset filename
    FString PackageFileName = FPackageName::LongPackageNameToFilename(
        AssetPath,
        FPackageName::GetAssetPackageExtension()
    );
#if WITH_EDITOR
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
    SaveArgs.Error = GError;
    SaveArgs.bWarnOfLongFilename = true;
    SaveArgs.bForceByteSwapping = false; // Set to true if needed
    SaveArgs.SaveFlags = SAVE_None;
    SaveArgs.bSlowTask = true;
    bool bSaved = UPackage::SavePackage(
        Package,
        Asset,
        *PackageFileName,
        SaveArgs
    );
    if (bSaved)
    {
        UE_LOG(LogTemp, Log, TEXT("Saved baked asset to %s"), *PackageFileName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save baked asset to %s"), *PackageFileName);
    }
#endif
}

void ULaneGraphEditorSubsystem::BuildGraph()
{
    UE_LOG(LogTemp, Warning, TEXT("LG Editor Subsystem Active, Processing Nodes..."));
    double StartTime = FPlatformTime::Seconds();
    
    //first pass, get all
    TArray<ULaneNode*> RawNodes;
    for (AStreetSplineActor* SSActor : GetStreetSplineActors())
    {
        TArray<ULaneNode*> LaneNodes = SSActor->GenerateLaneNodes();
        for (ULaneNode* LaneNode : LaneNodes)
        {
            RawNodes.Add(LaneNode);
        }
    }

    Nodes.Empty();

    //second pass, process neighbors from other splines, and form into TMap
    for (ULaneNode* LN : RawNodes)
    {
        //Get Neighbors
        //WARNING: N^2, Need to optimize for large datasets
        LN->Neighbors.Empty();
        for (ULaneNode* Candidate : RawNodes)
        {
            float Distance = FVector::Distance(Candidate->Position, LN->Position);
            if (Distance <= LN->NeighborDistance && Candidate->Id != LN->Id) 
                LN->addNeighbor(Candidate->Id);
        }
        
        TTuple<FGuid, ULaneNode*> Node(LN->Id, LN);
        Nodes.Add(Node);
        //Debug
        /*UE_LOG(LogTemp, Warning, TEXT("Node ID: %s ## Node Vector: %s ## Original SS ID: %s ## Point Spacing: %f"),
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
        }*/
    }


    double EndTime = FPlatformTime::Seconds();
    double ElapsedSeconds = EndTime - StartTime;

    UE_LOG(LogTemp, Warning, TEXT("Operation executed. Compiled %i Nodes in %f ms."), RawNodes.Num(), ElapsedSeconds * 1000);

    SaveNodeData();

}

TArray<FGuid> ULaneGraphEditorSubsystem::FindPath(FGuid StartNode, FGuid GoalNode)
{
    TArray<FGuid> Path;
    return Path;
    // Basic A* like in my previous sketch
    // Returns sequence of node Ids
}

TArray<FVector> ULaneGraphEditorSubsystem::FindPathPositions(FGuid StartNode, FGuid GoalNode)
{
    TArray<FVector> PathPoints;
    return PathPoints;
    //todo
    /*TArray<int32> PathIds = FindPath(StartNode, GoalNode);
    TArray<FVector> PathPoints;
    for (FGuid Id : PathIds)
    {
        PathPoints.Add(Nodes[Id].Position);
    }
    return PathPoints;*/
}

ULaneNode* ULaneGraphEditorSubsystem::GetNode(FGuid Id)
{ 
    return *Nodes.Find(Id); 
}
