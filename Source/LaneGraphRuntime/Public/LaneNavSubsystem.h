// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NodeDataAsset.h"

#include "LaneNavSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class LANEGRAPHRUNTIME_API ULaneNavSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable)
    bool TestNavData();

    UFUNCTION(BlueprintCallable)
    ULaneNode* GetClosestNode(FVector loc);

    UFUNCTION(BlueprintCallable)
    void DrawDebugNodes(float Duration);

    UFUNCTION(BlueprintCallable)
    TArray<FGuid> GetPathPoints(FGuid Start, FGuid End);

    UFUNCTION(BlueprintCallable)
    ULaneNode* GetNode(FGuid Id);

    UFUNCTION(BlueprintCallable)
    TArray<FVector> GetPositionsByIds(TArray<FGuid> Ids);


    UPROPERTY(BlueprintReadWrite)
    float HighPenalty = 5000.f;
    
    UPROPERTY(BlueprintReadWrite)
    float MediumPenalty = 10000.f;

    UPROPERTY(BlueprintReadWrite)
    float LowPenalty = 100.f;

private:
    // Must exactly match the delegate signatures
    void HandlePostWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
    void HandleWorldTearDown(UWorld* World);

    float Heuristic(const ULaneNode* A, const ULaneNode* B) const;
    float CalculatePenalty(const ULaneNode* From, const FGuid To) const;

	UPROPERTY()
	UNodeDataAsset* CurrentNavData;
};
