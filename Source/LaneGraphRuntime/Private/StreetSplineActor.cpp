// Fill out your copyright notice in the Description page of Project Settings.

#include "StreetSplineActor.h"
#include "DrawDebugHelpers.h"
#include <Components/DrawSphereComponent.h>

// Sets default values
AStreetSplineActor::AStreetSplineActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    Id = FGuid::NewGuid();

    // Create spline and set as root
    StreetSpline = CreateDefaultSubobject<USplineComponent>(TEXT("StreetSpline"));
    RootComponent = StreetSpline;

#if WITH_EDITORONLY_DATA
    DebugRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DebugRoot"));
    DebugRoot->SetupAttachment(RootComponent);
    DebugRoot->SetHiddenInGame(true);                   // never visible in PIE/game
    DebugRoot->SetIsVisualizationComponent(true);       // UE treats it as "helper", hidden in Outliner
    DebugRoot->bIsEditorOnly = true;                    // destroyed in cooked builds

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        DebugSphereMesh = SphereMesh.Object;
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
    if (ConeMesh.Succeeded())
    {
        DebugConeMesh = ConeMesh.Object;
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (BaseMat.Succeeded())
    {
        DebugBaseMaterial = BaseMat.Object;
    }

    MIDGreen = UMaterialInstanceDynamic::Create(DebugBaseMaterial, this);
    MIDGreen->SetVectorParameterValue("Color", FColor::Green);

    MIDRed = UMaterialInstanceDynamic::Create(DebugBaseMaterial, this);
    MIDRed->SetVectorParameterValue("Color", FColor::Red);

#endif

}

// Called when the game starts or when spawned
void AStreetSplineActor::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AStreetSplineActor::Tick(float DeltaTime)
{

    Super::Tick(DeltaTime);

}

void AStreetSplineActor::DebugText(FString message)
{
    GEngine->AddOnScreenDebugMessage(
        -1,           // Key: -1 for a unique message that doesn't overwrite others
        5.0f,         // Duration: How long the message stays on screen (in seconds)
        FColor::Red,  // Color: The color of the text
        message // The message to display
    );
}



void AStreetSplineActor::DrawLanePoint(FVector Loc, FVector Tan, bool Right)
{
#if WITH_EDITOR
    if (!DebugRoot || !DebugSphereMesh || !DebugConeMesh || !MIDGreen || !MIDRed) return;

    UMaterialInstanceDynamic* Mat = Right ? MIDGreen : MIDRed;
    FRotator Correction = Right ? FRotator(-90.f, 0.f, 0.f) : FRotator(90.f, 0.f, 0.f);
    FVector ConeLoc = Loc + (Right ? (Tan * 10) : (Tan * -10));


    // path point sphere
    UStaticMeshComponent* Sphere = NewObject<UStaticMeshComponent>(this);
    Sphere->SetupAttachment(DebugRoot);
    Sphere->RegisterComponent();

    Sphere->SetStaticMesh(DebugSphereMesh);
    Sphere->SetMaterial(0, Mat);

    Sphere->SetWorldLocation(Loc);
    Sphere->SetWorldScale3D(FVector(0.2f, 0.2f, 0.2f));
    Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Sphere->SetHiddenInGame(true);                   // never visible in PIE/game
    Sphere->SetIsVisualizationComponent(true);       // UE treats it as "helper", hidden in Outliner
    Sphere->bIsEditorOnly = true;                    // destroyed in cooked builds
    Sphere->SetMobility(EComponentMobility::Movable);

    //directional cone
    UStaticMeshComponent* Cone = NewObject<UStaticMeshComponent>(this);
    Cone->SetupAttachment(DebugRoot);
    Cone->RegisterComponent();

    Cone->SetStaticMesh(DebugConeMesh);
    Cone->SetMaterial(0, Mat);


    Cone->SetWorldLocation(ConeLoc);
    Cone->SetWorldRotation((Tan.Rotation().Quaternion() * Correction.Quaternion()));
    Cone->SetWorldScale3D(FVector(0.1f, 0.2f, 0.2f));
    Cone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Cone->SetHiddenInGame(true);                   // never visible in PIE/game
    Cone->SetIsVisualizationComponent(true);       // UE treats it as "helper", hidden in Outliner
    Cone->bIsEditorOnly = true;                    // destroyed in cooked builds
    Cone->SetMobility(EComponentMobility::Movable);
#endif
}

float AStreetSplineActor::GetLaneSpacing(int32 LaneNumber) {
    return MedianWidth + ((LaneNumber + 1) * LaneWidth) - LaneWidth / 2;
}

TArray<ULaneNode*> AStreetSplineActor::GenerateLaneNodes()
{
    TArray<ULaneNode*> Nodes;

    if (!StreetSpline) return Nodes;

#if WITH_EDITOR
    //Cleanup Debug Shapes
    TArray<USceneComponent*> DebugChildren;
    DebugRoot->GetChildrenComponents(false, DebugChildren);
    for (USceneComponent* DebugShape : DebugChildren)
    {
        DebugShape->DestroyComponent();
    }
#endif

    const float SplineLength = StreetSpline->GetSplineLength();
    const int32 NumSteps = FMath::FloorToInt(SplineLength / PointDensitySpacing);
    float NeighborDistance = sqrtf(LaneWidth * LaneWidth + PointDensitySpacing * PointDensitySpacing);

    TArray<ULaneNode*> PrevRightLane;
    TArray<ULaneNode*> PrevLeftLane;
    for (int32 i = 0; i <= NumSteps; i++)
    {
        float Distance = i * PointDensitySpacing;
        FVector Location = StreetSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
        FVector RightVec = StreetSpline->GetRightVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
        FVector N_Tan = StreetSpline->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World).GetSafeNormal();
        
        TArray<ULaneNode*> RightLane;
        for (int32 l = 0; l < RightLaneCount; l++)
        {
            FVector LanePointLocation = Location + (RightVec * GetLaneSpacing(l));
            if (ShowLanePathDebug) DrawLanePoint(LanePointLocation, N_Tan, true);

            ULaneNode* NewNode = NewObject<ULaneNode>(this);
            NewNode->Init(LanePointLocation, Id, NeighborDistance);
            NewNode->Forward = true;
            //R Backwards Penalty
            for (ULaneNode* Prev : PrevRightLane)
            {
                NewNode->addPenalty(Prev->Id, EPenaltyLevel::MEDIUM);
            }

            RightLane.Add(NewNode);
        }

        TArray<ULaneNode*> LeftLane;
        for (int32 l = 0; l < LeftLaneCount; l++)
        {
            FVector LanePointLocation = Location + (RightVec * GetLaneSpacing(l) * -1);
            if (ShowLanePathDebug) DrawLanePoint(LanePointLocation, N_Tan, false);

            ULaneNode* NewNode = NewObject<ULaneNode>(this);
            NewNode->Init(LanePointLocation, Id, NeighborDistance);
            NewNode->Forward = false;

            //L Backwards Penalty
            for (ULaneNode* Prev : PrevLeftLane)
            {
                Prev->addPenalty(NewNode->Id, EPenaltyLevel::HIGH);
            }

            LeftLane.Add(NewNode);
        }

        TArray<ULaneNode*> Row;
        Row.Append(RightLane);
        Row.Append(LeftLane);

        // Row Neighbor (Low) Penalty
        for (ULaneNode* RowMember : Row)
        {
            for (ULaneNode* OtherMember : Row)
            {
                if (RowMember != OtherMember)
                {
                    RowMember->addPenalty(OtherMember->Id, EPenaltyLevel::LOW);
                }
            }
        }


        Nodes.Append(Row);

        PrevRightLane.Empty();
        PrevRightLane.Append(RightLane);
        PrevLeftLane.Empty();
        PrevLeftLane.Append(LeftLane);
    }
    return Nodes;
    
}

// Declare dirty event
FOnStreetSplineDirtyEvent AStreetSplineActor::OnStreetSplineDirtyEvent;

// Editor OnConstruct (Something was changed about Actor in editor)
#if WITH_EDITOR
void AStreetSplineActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    TArray<ULaneNode*> Nodes = GenerateLaneNodes();

    OnStreetSplineDirtyEvent.Broadcast();    

    /* Intense node debugging
    for (ULaneNode* Node : Nodes)
    {
        UE_LOG(LogTemp, Warning, TEXT("Node ID: %s ## Node Vector: %s"), *Node->Id.ToString(EGuidFormats::DigitsWithHyphens), *Node->Position.ToString());
        for (FGuid PenId : Node->MediumPenalty)
        {
            UE_LOG(LogTemp, Warning, TEXT("Penalty Node ID: %s"), *PenId.ToString(EGuidFormats::DigitsWithHyphens));
        }
    }
    int32 NumNodes = Nodes.Num();
    UE_LOG(LogTemp, Warning, TEXT("Total Nodes: %i"), NumNodes);
    */
}
#endif

