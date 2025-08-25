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
    void TestNavData();

private:
    // Must exactly match the delegate signatures
    void HandlePostWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
    void HandleWorldTearDown(UWorld* World);

	UPROPERTY()
	UNodeDataAsset* CurrentNavData;
};
