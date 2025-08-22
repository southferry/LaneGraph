// Fill out your copyright notice in the Description page of Project Settings.

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "StreetSplineActor.generated.h"

UCLASS(PrioritizeCategories = "Street StreetNetwork")
class LANEGRAPHRUNTIME_API AStreetSplineActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AStreetSplineActor();

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USplineComponent* StreetSpline;

	// Configurable: how often to place lane points
	UPROPERTY(EditAnywhere, Category = "Street")
	float PointDensitySpacing = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Street|Lanes")
	int32 RightLaneCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Street|Lanes")
	int32 LeftLaneCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Street|Lanes")
	float LaneWidth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Street|Lanes")
	float MedianWidth = 0.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITORONLY_DATA
	// A root scene component just to hold debug helpers
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	USceneComponent* DebugRoot;

	UPROPERTY()
	UStaticMesh* DebugSphereMesh;

	UPROPERTY()
	UStaticMesh* DebugConeMesh;

	UPROPERTY()
	UMaterialInterface* DebugBaseMaterial;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* MIDGreen;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* MIDRed;
#endif

private:

	float GetLaneSpacing(int32 LaneNumber);

#if WITH_EDITORONLY_DATA
	void DebugText(FString message);
	void DrawLanePoint(FVector Loc, FVector Tan, bool Right);
#endif

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(CallInEditor, Category = "Street")
	void BakeAllStreets();

};
