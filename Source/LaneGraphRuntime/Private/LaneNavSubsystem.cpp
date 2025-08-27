#include "LaneNavSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NodeDataAsset.h"

/* ADMIN */

void ULaneNavSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Bind delegates
    FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ULaneNavSubsystem::HandlePostWorldInit);
    FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &ULaneNavSubsystem::HandleWorldTearDown);
}

void ULaneNavSubsystem::Deinitialize()
{
    // Clean up delegates
    FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
    FWorldDelegates::OnWorldBeginTearDown.RemoveAll(this);
    CurrentNavData = nullptr;

    Super::Deinitialize();
}

void ULaneNavSubsystem::HandlePostWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
    // Load Graph Data from Asset By Level
    if (World && World->IsGameWorld())
    {
        FString MapName = FPackageName::GetShortName(World->GetOutermost()->GetName());
        MapName.RemoveFromStart(World->StreamingLevelsPrefix);
        const FString PackageName = FString::Printf(TEXT("%s_%s"), *UNodeDataAsset::OBJECT_PREFIX, *MapName);
        const FString AssetPath = FString::Printf(TEXT("%s%s.%s"),
            *UNodeDataAsset::ASSET_PATH,
            *PackageName,
            *PackageName);

        CurrentNavData = LoadObject<UNodeDataAsset>(nullptr, *AssetPath);
    }
}

void ULaneNavSubsystem::HandleWorldTearDown(UWorld* World)
{
    CurrentNavData = nullptr;
}

void ULaneNavSubsystem::DrawDebugNodes(float Duration)
{
    if (CurrentNavData)
    {
        for (TTuple<FGuid, ULaneNode*> Entry : CurrentNavData->Nodes)
        {
            ULaneNode* Node = Entry.Value;
            FColor color = (Node->Forward) ? FColor::Green : FColor::Red;
            DrawDebugSphere(
                GetWorld(),
                Node->Position,
                15.f,
                12,
                color,
                false,
                Duration,
                0,
                1.f);
        }
    }

}

bool ULaneNavSubsystem::TestNavData()
{
    if (CurrentNavData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d entries!"), CurrentNavData->Nodes.Num());
        for (TTuple<FGuid, ULaneNode*> Node : CurrentNavData->Nodes)
        {
            ULaneNode* LN = Node.Value;
            UE_LOG(LogTemp, Warning, TEXT("Node ID: %s ## Node Vector: %s ## Original SS ID: %s ## Point Spacing: %f"),
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
            }
        }
        return true;
    } else {
        UE_LOG(LogTemp, Warning, TEXT("No Level Data Found!"));
        return false;
    }
}

/* Nav */

float ULaneNavSubsystem::Heuristic(const ULaneNode* A, const ULaneNode* B) const
{
    // Use Euclidean distance as heuristic
    return FVector::Distance(A->Position, B->Position);
}

float ULaneNavSubsystem::CalculatePenalty(const ULaneNode* From, const FGuid To) const
{
    if (const EPenaltyLevel* Level = From->Penalties.Find(To))
    {
        switch (*Level) {
            case EPenaltyLevel::HIGH:
                return HighPenalty;
            case EPenaltyLevel::MEDIUM:
                return MediumPenalty;
            case EPenaltyLevel::LOW:
                return LowPenalty;
        }
    }
    return 0.f;
}

TArray<FGuid> ULaneNavSubsystem::GetPathPoints(FGuid Start, FGuid End)
{
    TArray<FGuid> Path;

    ULaneNode* StartNode = CurrentNavData->Nodes.FindRef(Start);
    ULaneNode* EndNode = CurrentNavData->Nodes.FindRef(End);

    if (!StartNode || !EndNode)
    {
        return Path; // empty, invalid input
    }

    // Maps for A* bookkeeping
    TMap<FGuid, float> GScore;   // cost from start
    TMap<FGuid, float> FScore;   // estimated total cost
    TMap<FGuid, FGuid> CameFrom; // parent map

    struct FOpenNode
    {
        FGuid Id;
        float F;
        bool operator<(const FOpenNode& Other) const { return F < Other.F; } // min-heap
    };

    TArray<FOpenNode> OpenSet;
    OpenSet.HeapPush({ Start, 0.0f });

    GScore.Add(Start, 0.0f);
    FScore.Add(Start, Heuristic(StartNode, EndNode));

    TSet<FGuid> ClosedSet;

    while (OpenSet.Num() > 0)
    {
        // Get lowest F
        FOpenNode CurrentEntry;
        OpenSet.HeapPop(CurrentEntry, EAllowShrinking::Yes);
        FGuid CurrentId = CurrentEntry.Id;

        if (CurrentId == End)
        {
            // END STATE
            // Use CameFrom to reconstruct path by pushing the next camefrom onto [0] in path
            FGuid Step = End;
            while (CameFrom.Contains(Step))
            {
                Path.Insert(Step, 0);
                Step = CameFrom[Step];
            }
            Path.Insert(Start, 0);
            return Path;
        }

        ClosedSet.Add(CurrentId);

        ULaneNode* CurrentNode = CurrentNavData->Nodes.FindRef(CurrentId);
        if (!CurrentNode) continue;

        for (const FGuid& NeighborId : CurrentNode->Neighbors)
        {
            if (ClosedSet.Contains(NeighborId)) continue;

            ULaneNode* Neighbor = CurrentNavData->Nodes.FindRef(NeighborId);
            if (!Neighbor) continue;

            // Neighbor G = [Current G] + [Distance to Neighbor] + [NeighborPenalty]
            const float TentativeG = GScore[CurrentId] 
                + FVector::Dist(CurrentNode->Position, Neighbor->Position)
                + CalculatePenalty(CurrentNode, NeighborId);

            if (!GScore.Contains(NeighborId) || TentativeG < GScore[NeighborId])
            {
                CameFrom.Add(NeighborId, CurrentId);
                GScore.Add(NeighborId, TentativeG);
                FScore.Add(NeighborId, TentativeG + Heuristic(Neighbor, EndNode));

                // Push to open set
                OpenSet.HeapPush({ NeighborId, FScore[NeighborId] });
            }
        }
    }
    return Path;
}

ULaneNode* ULaneNavSubsystem::GetClosestNode(FVector loc)
{
    if (CurrentNavData && CurrentNavData->Nodes.Num() > 0)
    {
        float BestDist = 0.f;
        ULaneNode* BestNode = nullptr;
        for (TTuple<FGuid, ULaneNode*> Node : CurrentNavData->Nodes)
        {
            ULaneNode* LN = Node.Value;
            float Dist = FVector::Distance(LN->Position, loc);
            if (!BestNode || Dist < BestDist)
            {
                BestNode = LN;
                BestDist = Dist;
            }
        }
        return BestNode;
    }
    return nullptr;
}

ULaneNode* ULaneNavSubsystem::GetNode(FGuid Id)
{
    if (CurrentNavData)
    {
        return CurrentNavData->Nodes[Id];
    }
    return nullptr;
}

TArray<FVector> ULaneNavSubsystem::GetPositionsByIds(TArray<FGuid> Ids)
{
    TArray<FVector> Positions;
    for (FGuid Id : Ids)
    {
        ULaneNode* Node = CurrentNavData->Nodes.FindRef(Id);
        if (IsValid(Node)) Positions.Add(Node->Position);

    }
    return Positions;
}
