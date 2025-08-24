// LaneGraphEditorSubsystem.cpp

#include "LaneGraphEditorSubsystem.h"

#include "LaneNode.h"
#include "StreetSplineActor.h"
#include "NodeDataAsset.h"

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

#if WITH_EDITOR
void DeleteAssetIfExists(const FString& AssetPath)
{
    UObject* ExistingAsset = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
    if (!ExistingAsset)
        return;

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(ExistingAsset));

    if (AssetData.IsValid())
    {
        TArray<FAssetData> AssetsToDelete;
        AssetsToDelete.Add(AssetData);
        ObjectTools::DeleteAssets(AssetsToDelete, /*bShowConfirmation=*/false);
    }
}
#endif

void ULaneGraphEditorSubsystem::SaveNodeData(TArray<ULaneNode*>& Nodes)
{
    // Include the asset name at the end
    FString MapName = "default";
    if (GEditor && GEditor->GetEditorWorldContext().World())
    {
        UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
        MapName = EditorWorld->GetMapName();
    }
    const FString AssetPath = FString::Printf(TEXT("%s-%s"), 
        *UNodeDataAsset::DATA_STORAGE_PREFIX, 
        *MapName);
#if WITH_EDITOR

    DeleteAssetIfExists(*AssetPath);
    // Create package
    UPackage* Package = CreatePackage(*AssetPath);

    // Create the asset inside that package
    UNodeDataAsset* Asset = NewObject<UNodeDataAsset>(
        Package,
        UNodeDataAsset::StaticClass(),
        *FPaths::GetBaseFilename(AssetPath), // "MyLaneGraph"
        RF_Public | RF_Standalone
    );

    Asset->Nodes.Empty();
    // Copy your node data
    for (ULaneNode* Node : Nodes)
    {
        ULaneNode* LaneNodeCopy = DuplicateObject<ULaneNode>(Node, Asset);
        Asset->Nodes.Emplace(LaneNodeCopy->Id, LaneNodeCopy);
    }

    // Notify AssetRegistry
    FAssetRegistryModule::AssetCreated(Asset);

    // Mark dirty
    Asset->MarkPackageDirty();

    // Build the .uasset filename
    FString PackageFileName = FPackageName::LongPackageNameToFilename(
        AssetPath,
        FPackageName::GetAssetPackageExtension()
    );

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
    SaveArgs.Error = GError;
    SaveArgs.bWarnOfLongFilename = true;
    SaveArgs.bForceByteSwapping = false; // Set to true if needed
    SaveArgs.SaveFlags = SAVE_None;
    SaveArgs.bSlowTask = true;
    bool bSaved = UPackage::SavePackage(
        Package,
        Asset,
        *PackageFileName,
        SaveArgs
    );

    // Memory Cleanup because Asset Storage creates stale references
    Asset->ClearFlags(RF_Standalone);  // Remove standalone reference
    Asset->SetFlags(RF_Transient);
    Asset = nullptr;
    Package->MarkAsGarbage();
    CollectGarbage(RF_NoFlags);

    if (bSaved)
    {
        UE_LOG(LogTemp, Log, TEXT("Saved baked asset to %s"), *PackageFileName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save baked asset to %s"), *PackageFileName);
    }
#endif
}

void ULaneGraphEditorSubsystem::BuildGraph()
{
    UE_LOG(LogTemp, Warning, TEXT("LG Editor Subsystem Active, Processing Nodes..."));
    double StartTime = FPlatformTime::Seconds();
    
    //first pass, get all
    TArray<ULaneNode*> RawNodes;
    for (AStreetSplineActor* SSActor : GetStreetSplineActors())
    {
        TArray<ULaneNode*> LaneNodes = SSActor->GenerateLaneNodes();
        for (ULaneNode* LaneNode : LaneNodes)
        {
            RawNodes.Add(LaneNode);
        }
    }

    TArray<ULaneNode*> Nodes;

    //second pass, process neighbors from other splines, and form into TMap
    for (ULaneNode* LN : RawNodes)
    {
        //Get Neighbors
        //WARNING: N^2, Need to optimize for large datasets
        LN->Neighbors.Empty();
        for (ULaneNode* Candidate : RawNodes)
        {
            float Distance = FVector::Distance(Candidate->Position, LN->Position);
            if (Distance <= LN->NeighborDistance && Candidate->Id != LN->Id) 
                LN->addNeighbor(Candidate->Id);
        }
        
        Nodes.Add(LN);
        //Debug
        /*UE_LOG(LogTemp, Warning, TEXT("Node ID: %s ## Node Vector: %s ## Original SS ID: %s ## Point Spacing: %f"),
            *LN->Id.ToString(EGuidFormats::DigitsWithHyphens), 
            *LN->Position.ToString(), 
            *LN->OriginalSplineId.ToString(EGuidFormats::DigitsWithHyphens),
            LN->NeighborDistance);
        for (FGuid PenId : LN->MediumPenalty)
        {
            UE_LOG(LogTemp, Warning, TEXT("Penalty Node ID: %s"), *PenId.ToString(EGuidFormats::DigitsWithHyphens));
        }
        for (FGuid NeighId : LN->Neighbors)
        {
            UE_LOG(LogTemp, Warning, TEXT("Neighbor Node ID: %s"), *NeighId.ToString(EGuidFormats::DigitsWithHyphens));
        }*/
    }


    double EndTime = FPlatformTime::Seconds();
    double ElapsedSeconds = EndTime - StartTime;

    UE_LOG(LogTemp, Warning, TEXT("Operation executed. Compiled %i Nodes in %f ms."), RawNodes.Num(), ElapsedSeconds * 1000);

    SaveNodeData(Nodes);
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