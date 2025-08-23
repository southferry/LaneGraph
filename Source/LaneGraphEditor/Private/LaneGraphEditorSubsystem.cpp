// LaneGraphEditorSubsystem.cpp
#include "LaneGraphEditorSubsystem.h"
#include "StreetSplineActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "LaneNode.h"

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

    UE_LOG(LogTemp, Warning, TEXT("Operation executed. Compiled %i Nodes in %f ms."), Nodes.Num(), ElapsedSeconds * 1000);

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

void ULaneGraphEditorSubsystem::TestSubsystemIsActive()
{
    UE_LOG(LogTemp, Warning, TEXT("SubSubsystem Active"));
}