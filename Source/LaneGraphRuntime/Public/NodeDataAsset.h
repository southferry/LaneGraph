

#pragma once

#include "CoreMinimal.h"
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

	inline static const FString ASSET_PATH = TEXT("/Game/__Generated/Teleograph/Cache/LaneGraph/");
	inline static const FString OBJECT_PREFIX = TEXT("LaneGraphData");

	UNodeDataAsset();

	UPROPERTY()
	TMap<FGuid, ULaneNode*> Nodes;
	
};
