#include "VelocityRushTrackActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AVelocityRushTrackActor::AVelocityRushTrackActor()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    RoadMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RoadMesh"));
    RoadMesh->SetupAttachment(SceneRoot);
    RoadMesh->SetCollisionProfileName(TEXT("BlockAll"));
    RoadMesh->bUseAsyncCooking = true;

    CityGround = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CityGround"));
    CityGround->SetupAttachment(SceneRoot);
    CityGround->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
    CityGround->SetRelativeScale3D(FVector(160.0f, 160.0f, 0.18f));
    CityGround->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CityGround->SetCastShadow(false);

    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetIntensity(12.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.88f, 0.72f));
    SunLight->SetRelativeRotation(FRotator(-42.0f, -28.0f, 0.0f));

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetIntensity(0.8f);

    BarrierInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Barriers"));
    BarrierInstances->SetupAttachment(SceneRoot);
    BarrierInstances->SetCollisionProfileName(TEXT("BlockAll"));

    BuildingInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CityBuildings"));
    BuildingInstances->SetupAttachment(SceneRoot);
    BuildingInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeFinder.Succeeded())
    {
        CubeMesh = CubeFinder.Object;
        CityGround->SetStaticMesh(CubeMesh);
        BarrierInstances->SetStaticMesh(CubeMesh);
        BuildingInstances->SetStaticMesh(CubeMesh);
    }
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (MaterialFinder.Succeeded())
    {
        BasicMaterial = MaterialFinder.Object;
        UMaterialInstanceDynamic* Asphalt = UMaterialInstanceDynamic::Create(BasicMaterial, this);
        Asphalt->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.045f, 0.055f, 0.07f));
        RoadMesh->SetMaterial(0, Asphalt);
        UMaterialInstanceDynamic* BarrierPaint = UMaterialInstanceDynamic::Create(BasicMaterial, this);
        BarrierPaint->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.16f, 0.21f, 0.25f));
        BarrierInstances->SetMaterial(0, BarrierPaint);
        UMaterialInstanceDynamic* CityPaint = UMaterialInstanceDynamic::Create(BasicMaterial, this);
        CityPaint->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.12f, 0.18f, 0.24f));
        BuildingInstances->SetMaterial(0, CityPaint);
        UMaterialInstanceDynamic* GroundPaint = UMaterialInstanceDynamic::Create(BasicMaterial, this);
        GroundPaint->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.055f, 0.12f, 0.14f));
        CityGround->SetMaterial(0, GroundPaint);
    }
}

void AVelocityRushTrackActor::BeginPlay()
{
    Super::BeginPlay();
    BuildRoute();
    BuildRoad();
    BuildBarriers();
    BuildCity();
}

void AVelocityRushTrackActor::BuildRoute()
{
    Route.Reset();
    const TArray<FVector> ControlPoints = {
        FVector(-4200.0f, -1800.0f, 0.0f), FVector(-2400.0f, -3900.0f, 0.0f),
        FVector(900.0f, -4300.0f, 0.0f), FVector(3900.0f, -2500.0f, 0.0f),
        FVector(4300.0f, 900.0f, 0.0f), FVector(2300.0f, 3900.0f, 0.0f),
        FVector(-1100.0f, 4200.0f, 0.0f), FVector(-4000.0f, 2100.0f, 0.0f)
    };
    constexpr int32 SamplesPerSegment = 20;
    const int32 Count = ControlPoints.Num();
    for (int32 I = 0; I < Count; ++I)
    {
        const FVector& P0 = ControlPoints[(I - 1 + Count) % Count];
        const FVector& P1 = ControlPoints[I];
        const FVector& P2 = ControlPoints[(I + 1) % Count];
        const FVector& P3 = ControlPoints[(I + 2) % Count];
        for (int32 Sample = 0; Sample < SamplesPerSegment; ++Sample)
        {
            const float T = static_cast<float>(Sample) / SamplesPerSegment;
            const float T2 = T * T;
            const float T3 = T2 * T;
            const FVector Point = 0.5f * ((2.0f * P1) + (-P0 + P2) * T +
                (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * T2 +
                (-P0 + 3.0f * P1 - 3.0f * P2 + P3) * T3);
            Route.Add(Point);
        }
    }
}

void AVelocityRushTrackActor::BuildRoad()
{
    if (Route.Num() < 3 || !RoadMesh)
    {
        return;
    }
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> Colors;
    Vertices.Reserve(Route.Num() * 2);
    Triangles.Reserve(Route.Num() * 6);
    for (int32 I = 0; I < Route.Num(); ++I)
    {
        const FVector Tangent = (Route[(I + 1) % Route.Num()] - Route[(I - 1 + Route.Num()) % Route.Num()]).GetSafeNormal();
        const FVector Side = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
        Vertices.Add(Route[I] + Side * RoadWidth * 0.5f + FVector(0, 0, 6));
        Vertices.Add(Route[I] - Side * RoadWidth * 0.5f + FVector(0, 0, 6));
        const float V = static_cast<float>(I) * 0.08f;
        UVs.Add(FVector2D(0.0f, V));
        UVs.Add(FVector2D(1.0f, V));
        Normals.Add(FVector::UpVector);
        Normals.Add(FVector::UpVector);
        Colors.Add(FLinearColor(0.075f, 0.085f, 0.10f, 1.0f));
        Colors.Add(FLinearColor(0.075f, 0.085f, 0.10f, 1.0f));
        const int32 A = I * 2;
        const int32 B = A + 1;
        const int32 C = ((I + 1) % Route.Num()) * 2 + 1;
        const int32 D = ((I + 1) % Route.Num()) * 2;
        Triangles.Add(A);
        Triangles.Add(B);
        Triangles.Add(C);
        Triangles.Add(A);
        Triangles.Add(C);
        Triangles.Add(D);
    }
    RoadMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, Colors, TArray<FProcMeshTangent>(), true);
    if (BasicMaterial)
    {
        UMaterialInstanceDynamic* Asphalt = UMaterialInstanceDynamic::Create(BasicMaterial, this);
        Asphalt->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.045f, 0.055f, 0.07f));
        RoadMesh->SetMaterial(0, Asphalt);
    }
}

void AVelocityRushTrackActor::BuildBarriers()
{
    if (!CubeMesh || Route.Num() == 0)
    {
        return;
    }
    for (int32 I = 0; I < Route.Num(); I += 3)
    {
        const FVector Tangent = (Route[(I + 1) % Route.Num()] - Route[(I - 1 + Route.Num()) % Route.Num()]).GetSafeNormal();
        const FVector Side = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
        const FRotator Facing = Tangent.Rotation();
        for (const float Sign : { -1.0f, 1.0f })
        {
            const FVector Location = Route[I] + Side * (RoadWidth * 0.5f + 110.0f) * Sign + FVector(0, 0, 52.0f);
            BarrierInstances->AddInstance(FTransform(Facing, Location, FVector(0.85f, 1.9f, 0.52f)));
        }
    }
}

void AVelocityRushTrackActor::BuildCity()
{
    if (!CubeMesh || Route.Num() == 0)
    {
        return;
    }
    FRandomStream Random(43125);
    for (int32 I = 0; I < Route.Num(); I += 5)
    {
        const FVector Tangent = (Route[(I + 1) % Route.Num()] - Route[(I - 1 + Route.Num()) % Route.Num()]).GetSafeNormal();
        const FVector Side = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
        for (const float Sign : { -1.0f, 1.0f })
        {
            for (int32 Row = 0; Row < 3; ++Row)
            {
                const float Height = Random.FRandRange(520.0f, 1900.0f);
                const float Width = Random.FRandRange(360.0f, 760.0f);
                const float Depth = Random.FRandRange(360.0f, 720.0f);
                const float Offset = RoadWidth * 0.5f + 1100.0f + Row * 1000.0f;
                const FVector Location = Route[I] + Side * Offset * Sign + FVector(0, 0, Height * 0.5f);
                const FRotator Rotation(0.0f, Random.FRandRange(-12.0f, 12.0f), 0.0f);
                BuildingInstances->AddInstance(FTransform(Rotation, Location, FVector(Width / 100.0f, Depth / 100.0f, Height / 100.0f)));
            }
        }
    }
}

FVector AVelocityRushTrackActor::GetRouteLocation(int32 Index) const
{
    return Route.IsValidIndex(Index) ? Route[Index] : FVector::ZeroVector;
}

FRotator AVelocityRushTrackActor::GetRouteRotation(int32 Index) const
{
    if (Route.Num() < 2)
    {
        return FRotator::ZeroRotator;
    }
    const FVector Tangent = (Route[(Index + 1) % Route.Num()] - Route[(Index - 1 + Route.Num()) % Route.Num()]).GetSafeNormal();
    return Tangent.Rotation();
}
