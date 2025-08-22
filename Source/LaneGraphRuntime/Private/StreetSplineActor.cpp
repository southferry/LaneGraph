// Fill out your copyright notice in the Description page of Project Settings.

#include "StreetSplineActor.h"
#include "DrawDebugHelpers.h"
#include <Components/DrawSphereComponent.h>

// Sets default values
AStreetSplineActor::AStreetSplineActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

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
    if (SphereMesh.Succeeded())
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
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,           // Key: -1 for a unique message that doesn't overwrite others
            5.0f,         // Duration: How long the message stays on screen (in seconds)
            FColor::Red,  // Color: The color of the text
            TEXT("Hello, I am a StreetSpline Actor") // The message to display
        );
    }
    Super::BeginPlay();

}

// Called every frame
void AStreetSplineActor::Tick(float DeltaTime)
{

    Super::Tick(DeltaTime);

}
/*
USphereComponent* Sphere = NewObject<USphereComponent>(this);
        Sphere->AttachToComponent(DebugRoot, FAttachmentTransformRules::KeepWorldTransform);
        Sphere->RegisterComponent();
        Sphere->InitSphereRadius(25.f);
        Sphere->SetWorldLocation(Node);
        Sphere->SetHiddenInGame(true);


*/

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
    Sphere->SetWorldScale3D(FVector(0.07f, 0.07f, 0.01f));
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
    Cone->SetWorldScale3D(FVector(0.01f, 0.07f, 0.07f));
    Cone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Cone->SetHiddenInGame(true);                   // never visible in PIE/game
    Cone->SetIsVisualizationComponent(true);       // UE treats it as "helper", hidden in Outliner
    Cone->bIsEditorOnly = true;                    // destroyed in cooked builds
    Cone->SetMobility(EComponentMobility::Movable);
}

float AStreetSplineActor::GetLaneSpacing(int32 LaneNumber) {
    return MedianWidth + ((LaneNumber + 1) * LaneWidth) - LaneWidth / 2;
}

void AStreetSplineActor::BakeAllStreets()
{
    DebugText("Baking Street Network...");
}

#if WITH_EDITOR
void AStreetSplineActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (!StreetSpline) return;

    //Cleanup Debug Shapes
    TArray<USceneComponent*> DebugChildren;
    DebugRoot->GetChildrenComponents(false, DebugChildren);
    for (USceneComponent* DebugShape : DebugChildren)
    {
        DebugShape->DestroyComponent();
    }

    const float SplineLength = StreetSpline->GetSplineLength();
    const int32 NumSteps = FMath::FloorToInt(SplineLength / PointDensitySpacing);

    FlushPersistentDebugLines(GetWorld());

    for (int32 i = 0; i <= NumSteps; i++)
    {
        float Distance = i * PointDensitySpacing;
        FVector Location = StreetSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
        FVector RightVec = StreetSpline->GetRightVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
        FVector N_Tan = StreetSpline->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World).GetSafeNormal();

        for (int32 l = 0; l < RightLaneCount; l++)
        {
            FVector LanePointLocation = Location + (RightVec * GetLaneSpacing(l));
            DrawLanePoint(LanePointLocation, N_Tan, true);
        }

        for (int32 l = 0; l < LeftLaneCount; l++)
        {
            FVector LanePointLocation = Location + (RightVec * GetLaneSpacing(l) * -1);
            DrawLanePoint(LanePointLocation, N_Tan, false);
        }

    }
}
#endif

