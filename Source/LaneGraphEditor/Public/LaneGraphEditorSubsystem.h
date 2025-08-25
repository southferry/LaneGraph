// LaneGraphEditorSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Subsystems/SubsystemCollection.h"
#include "LaneNode.h"
#include "NodeDataAsset.h"

#include "LaneGraphEditorSubsystem.generated.h"

UCLASS()
class LANEGRAPHEDITOR_API ULaneGraphEditorSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "LaneGraph")
    void BuildGraph();

    UFUNCTION()
    void HandleDirtyStreetSpline();


private:
    UFUNCTION(BlueprintCallable)
    TArray<AStreetSplineActor*> GetStreetSplineActors();

    UPROPERTY()
    bool bIsDirty = false;

    void AddToolbarIndicator();

    TSharedPtr<STextBlock> DirtyIndicatorText;

    TSharedRef<SWidget> MakeDirtyIndicatorWidget();

    void UpdateDirtyIndicator();


};
