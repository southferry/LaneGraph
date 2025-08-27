// LaneGraphEditorSubsystem.cpp

#include "LaneGraphEditorSubsystem.h"

#include "LaneNode.h"
#include "StreetSplineActor.h"
#include "NodeDataAsset.h"
#include "DataAssetHelper.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"

#include "LevelEditor.h"
#include "ToolMenus.h"

#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include "Editor.h"

void ULaneGraphEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    AddToolbarIndicator();
    // If one street spline says its dirty, kick off notification
    AStreetSplineActor::OnStreetSplineDirtyEvent.AddUObject(this, &ULaneGraphEditorSubsystem::HandleDirtyStreetSpline);
}

TArray<AStreetSplineActor*> ULaneGraphEditorSubsystem::GetStreetSplineActors()
{
    UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

    ULevel* Level = EditorWorld->GetCurrentLevel(); 

    TArray<AStreetSplineActor*> SSActors;

    for (AActor* Actor : Level->Actors)
    {
        if (AStreetSplineActor* SSActor = Cast<AStreetSplineActor>(Actor))
        {
            SSActors.Add(SSActor);
        }
    }
    return SSActors;
}

void ULaneGraphEditorSubsystem::BuildGraph()
{
    UE_LOG(LogTemp, Warning, TEXT("LG Editor Subsystem Active, Processing Nodes..."));
    double StartTime = FPlatformTime::Seconds();
    
    //first pass, get all
    TArray<ULaneNode*> Nodes;
    for (AStreetSplineActor* SSActor : GetStreetSplineActors())
    {
        TArray<ULaneNode*> LaneNodes = SSActor->GenerateLaneNodes();
        Nodes.Append(LaneNodes);
    }

    //second pass, process neighbors from other splines, and form into TMap
    for (ULaneNode* LN : Nodes)
    {
        //Get Neighbors
        //WARNING: N^2, Need to optimize for large datasets
        LN->Neighbors.Empty();
        for (ULaneNode* Candidate : Nodes)
        {
            float Distance = FVector::Distance(Candidate->Position, LN->Position);
            if (Distance <= LN->NeighborDistance && Candidate->Id != LN->Id) 
                LN->addNeighbor(Candidate->Id);
        }
    }


    double EndTime = FPlatformTime::Seconds();
    double ElapsedSeconds = EndTime - StartTime;

    UE_LOG(LogTemp, Warning, TEXT("Operation executed. Compiled %i Nodes in %f ms."), Nodes.Num(), ElapsedSeconds * 1000);

    /*
    ASSET SAVE
    */

    FString MapName = "default";
    if (GEditor && GEditor->GetEditorWorldContext().World())
    {
        UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
        MapName = FPackageName::GetShortName(EditorWorld->GetOutermost()->GetName());
    }

    const FString AssetPath = FString::Printf(TEXT("%s%s_%s"),
        *UNodeDataAsset::ASSET_PATH,
        *UNodeDataAsset::OBJECT_PREFIX,
        *MapName);

    FDataAssetHelper::CreateOrUpdateDataAsset<UNodeDataAsset>(
        AssetPath,
        [Nodes](UNodeDataAsset* Asset)
        {
            Asset->Nodes.Empty();
            for (ULaneNode* Node : Nodes)
            {
                ULaneNode* LaneNodeCopy = DuplicateObject<ULaneNode>(Node, Asset);
                Asset->Nodes.Emplace(LaneNodeCopy->Id, LaneNodeCopy);
            }
        }
    );

    bIsDirty = false;
    UpdateDirtyIndicator();
}

void ULaneGraphEditorSubsystem::HandleDirtyStreetSpline()
{
    // mark your state
    bIsDirty = true;
    UpdateDirtyIndicator();

}

void ULaneGraphEditorSubsystem::AddToolbarIndicator()
{
    if (!UToolMenus::IsToolMenuUIEnabled())
        return;

    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");

    FToolMenuSection& Section = Menu->AddSection("LaneGraphIndicator", FText::FromString("LaneGraph"));
    Section.AddEntry(FToolMenuEntry::InitWidget(
        "LaneGraphDirtyIndicator",
        MakeDirtyIndicatorWidget(),
        FText::FromString("LaneGraph Status")
    ));
}

TSharedRef<SWidget> ULaneGraphEditorSubsystem::MakeDirtyIndicatorWidget()
{
    SAssignNew(DirtyIndicatorText, STextBlock)
        .Text(FText::FromString("LaneGraph Data Status: Unkown (Bake Advisable)"))
        .ColorAndOpacity(FLinearColor::White);

    return DirtyIndicatorText.ToSharedRef();
}

void ULaneGraphEditorSubsystem::UpdateDirtyIndicator()
{
    if (DirtyIndicatorText.IsValid())
    {
        DirtyIndicatorText->SetText(FText::FromString(bIsDirty ? "LaneGraph Data Status: Dirty" : ""));
        DirtyIndicatorText->SetColorAndOpacity(FLinearColor::Red);
    }
}