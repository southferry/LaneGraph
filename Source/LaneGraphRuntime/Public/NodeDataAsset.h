// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LaneNode.h"
#include "NodeDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class LANEGRAPHRUNTIME_API UNodeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UNodeDataAsset();

	inline static const FString DATA_STORAGE_PREFIX = TEXT("/Game/__Generated/Teleograph/Cache/LaneGraph/LaneGraphData");

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly)
	TMap<FGuid, ULaneNode*> Nodes;
	
};
