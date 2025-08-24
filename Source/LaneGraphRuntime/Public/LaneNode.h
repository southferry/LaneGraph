// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LaneNode.generated.h"

UENUM()
enum EPenaltyLevel {
	LOW,
	MEDIUM,
	HIGH
};

UCLASS()
class LANEGRAPHRUNTIME_API ULaneNode : public UObject
{
	GENERATED_BODY()

public:
	
	ULaneNode();

	// Needed because no paramed constructors in unreal
	void Init(FVector InPos, FGuid SplineId, float SplineNeighborDistance);

	UPROPERTY(BlueprintReadOnly)
	FGuid Id;

	UPROPERTY(BlueprintReadOnly)
	FGuid OriginalSplineId;

	UPROPERTY(BlueprintReadOnly)
	float NeighborDistance;

	UPROPERTY(BlueprintReadOnly)
	FVector Position;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGuid> Neighbors;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGuid> HighPenalty;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGuid> MediumPenalty;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGuid> LowPenalty;

	UFUNCTION()
	void addPenalty(FGuid NodeId, EPenaltyLevel level);

	UFUNCTION()
	void addNeighbor(FGuid NodeId);
	
};
