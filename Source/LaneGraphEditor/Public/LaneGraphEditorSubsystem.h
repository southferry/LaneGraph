// LaneGraphEditorSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "LaneGraphEditorSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FLaneNode
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FGuid Id;

    UPROPERTY(BlueprintReadOnly)
    FVector Position;

    UPROPERTY(BlueprintReadOnly)
    TArray<FGuid> Neighbors;

    UPROPERTY(BlueprintReadOnly)
    TArray<FGuid> HighPenalty;

    UPROPERTY(BlueprintReadOnly)
    TArray<FGuid> MediumPenalty;
};

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
    const FLaneNode& GetNode(FGuid Id) const { return Nodes[Id]; }

    UFUNCTION(BlueprintCallable, Category = "LaneGraph")
    void TestSubsystemIsActive();

private:
    TMap<FGuid, FLaneNode> Nodes;

};
