// LaneGraphEditorSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "LaneNode.h"
#include "NodeDataAsset.h"
#include "LaneGraphEditorSubsystem.generated.h"

UCLASS()
class LANEGRAPHEDITOR_API ULaneGraphEditorSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "LaneGraph")
    void BuildGraph();

    UFUNCTION(BlueprintCallable, Category = "LaneGraph")
    TArray<FGuid> FindPath(FGuid StartNode, FGuid GoalNode);

    UFUNCTION(BlueprintCallable, Category = "LaneGraph")
    TArray<FVector> FindPathPositions(FGuid StartNode, FGuid GoalNode);

    UFUNCTION(BlueprintCallable, Category = "LaneGraph")
    ULaneNode* GetNode(FGuid Id);


private:
    UFUNCTION(BlueprintCallable)
    TArray<AStreetSplineActor*> GetStreetSplineActors();

    UFUNCTION(BlueprintCallable)
    void SaveNodeData();
    
    UPROPERTY()
    TMap<FGuid, ULaneNode*> Nodes;

};
