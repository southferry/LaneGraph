// LaneGraphEditorSubsystem.cpp
#include "LaneGraphEditorSubsystem.h"
#include "StreetSplineActor.h"

void ULaneGraphEditorSubsystem::BuildGraph()
{
    Nodes.Empty();

    // Iterate over your custom StreetSplineActors in the level
    /*for (TActorIterator<AStreetSplineActor> Splines(GetWorld()); Splines; ++It)
    {
        Splines->GenerateLaneNodes(Nodes); // push nodes into array
    }*/

    // Optionally assign IDs, neighbors, etc. here
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
    return Nodes.Find(Id); 
}

void ULaneGraphEditorSubsystem::TestSubsystemIsActive()
{
    UE_LOG(LogTemp, Warning, TEXT("SubSubsystem Active"));
}