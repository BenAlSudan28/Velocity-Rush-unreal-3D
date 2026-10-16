#include "VelocityRushVehiclePawn.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "VelocityRushGameMode.h"
#include "VelocityRushPlayerController.h"

AVelocityRushVehiclePawn::AVelocityRushVehiclePawn()
{
    PrimaryActorTick.bCanEverTick = true;
    CollisionBody = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBody"));
    RootComponent = CollisionBody;
    CollisionBody->SetBoxExtent(FVector(205.0f, 92.0f, 48.0f));
    CollisionBody->SetCollisionProfileName(TEXT("Pawn"));
    CollisionBody->SetGenerateOverlapEvents(true);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 920.0f;
    CameraBoom->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f));
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 5.0f;
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->SetFieldOfView(78.0f);

    Chassis = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Chassis"));
    Chassis->SetupAttachment(RootComponent);
    Chassis->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ImportedModel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ImportedModel"));
    ImportedModel->SetupAttachment(RootComponent);
    ImportedModel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ImportedModel->SetVisibility(false, true);
    Cabin = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cabin"));
    Cabin->SetupAttachment(RootComponent);
    FrontSpoiler = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearWing"));
    FrontSpoiler->SetupAttachment(RootComponent);
    NitroFlame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NitroFlame"));
    NitroFlame->SetupAttachment(RootComponent);
    NitroFlame->SetVisibility(false, true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CubeFinder.Succeeded())
    {
        Cabin->SetStaticMesh(CubeFinder.Object);
        FrontSpoiler->SetStaticMesh(CubeFinder.Object);
        NitroFlame->SetStaticMesh(CubeFinder.Object);
    }
    if (CylinderFinder.Succeeded())
    {
        for (int32 I = 0; I < 4; ++I)
        {
            UStaticMeshComponent* Wheel = CreateDefaultSubobject<UStaticMeshComponent>(FName(*FString::Printf(TEXT("Wheel%d"), I)));
            Wheel->SetupAttachment(RootComponent);
            Wheel->SetStaticMesh(CylinderFinder.Object);
            Wheel->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
            Wheel->SetRelativeScale3D(FVector(0.80f, 0.80f, 0.25f));
            Wheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Wheels.Add(Wheel);
        }
    }
    for (UStaticMeshComponent* Component : {Cabin.Get(), FrontSpoiler.Get(), NitroFlame.Get()})
    {
        if (Component)
        {
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void AVelocityRushVehiclePawn::BuildCarVisuals()
{
    struct FBodySection
    {
        float X;
        float HalfWidth;
        float TopHalfWidth;
        float BottomZ;
        float SideTopZ;
        float TopZ;
    };
    const FBodySection Sections[] = {
        {-220.0f, 58.0f, 38.0f, 30.0f, 40.0f, 51.0f},
        {-178.0f, 78.0f, 57.0f, 24.0f, 43.0f, 66.0f},
        {-95.0f, 96.0f, 67.0f, 22.0f, 47.0f, 84.0f},
        {55.0f, 91.0f, 62.0f, 22.0f, 46.0f, 80.0f},
        {165.0f, 70.0f, 49.0f, 25.0f, 41.0f, 64.0f},
        {220.0f, 48.0f, 30.0f, 32.0f, 40.0f, 50.0f}
    };
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> Colors;
    for (const FBodySection& Section : Sections)
    {
        const FVector Ring[] = {
            FVector(Section.X, -Section.HalfWidth, Section.BottomZ),
            FVector(Section.X, -Section.HalfWidth, Section.SideTopZ),
            FVector(Section.X, -Section.TopHalfWidth, Section.TopZ),
            FVector(Section.X, Section.TopHalfWidth, Section.TopZ),
            FVector(Section.X, Section.HalfWidth, Section.SideTopZ),
            FVector(Section.X, Section.HalfWidth, Section.BottomZ)
        };
        for (int32 J = 0; J < 6; ++J)
        {
            Vertices.Add(Ring[J]);
            UVs.Add(FVector2D(static_cast<float>(J) / 6.0f, static_cast<float>(Vertices.Num()) * 0.02f));
            Colors.Add(FLinearColor::White);
        }
    }
    for (int32 I = 0; I < UE_ARRAY_COUNT(Sections) - 1; ++I)
    {
        for (int32 J = 0; J < 6; ++J)
        {
            const int32 A = I * 6 + J;
            const int32 B = I * 6 + ((J + 1) % 6);
            const int32 C = (I + 1) * 6 + ((J + 1) % 6);
            const int32 D = (I + 1) * 6 + J;
            Triangles.Add(A); Triangles.Add(B); Triangles.Add(C);
            Triangles.Add(A); Triangles.Add(C); Triangles.Add(D);
        }
    }
    for (const int32 EndIndex : {0, UE_ARRAY_COUNT(Sections) - 1})
    {
        const int32 Base = EndIndex * 6;
        const FBodySection& Section = Sections[EndIndex];
        const int32 Center = Vertices.Add(FVector(Section.X, 0.0f, (Section.BottomZ + Section.TopZ) * 0.5f));
        UVs.Add(FVector2D(0.5f, 0.5f));
        Colors.Add(FLinearColor::White);
        for (int32 J = 0; J < 6; ++J)
        {
            const int32 A = Base + J;
            const int32 B = Base + ((J + 1) % 6);
            if (EndIndex == 0)
            {
                Triangles.Add(Center); Triangles.Add(B); Triangles.Add(A);
            }
            else
            {
                Triangles.Add(Center); Triangles.Add(A); Triangles.Add(B);
            }
        }
    }
    Chassis->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), UVs, Colors, TArray<FProcMeshTangent>(), false);
    Chassis->SetRelativeLocation(FVector::ZeroVector);
    Cabin->SetRelativeLocation(FVector(-5.0f, 0.0f, 82.0f));
    Cabin->SetRelativeScale3D(FVector(0.91f, 0.70f, 0.34f));
    FrontSpoiler->SetRelativeLocation(FVector(-185.0f, 0.0f, 62.0f));
    FrontSpoiler->SetRelativeScale3D(FVector(0.16f, 0.84f, 0.08f));
    NitroFlame->SetRelativeLocation(FVector(-232.0f, 0.0f, 39.0f));
    NitroFlame->SetRelativeScale3D(FVector(0.42f, 0.15f, 0.15f));
    const FVector WheelLocations[] = {
        FVector(128.0f, -91.0f, -8.0f), FVector(128.0f, 91.0f, -8.0f),
        FVector(-128.0f, -91.0f, -8.0f), FVector(-128.0f, 91.0f, -8.0f)
    };
    for (int32 I = 0; I < Wheels.Num(); ++I)
    {
        Wheels[I]->SetRelativeLocation(WheelLocations[I]);
    }
}

void AVelocityRushVehiclePawn::UpdateCarColor()
{
    static const FLinearColor Paints[] = {
        FLinearColor(0.78f, 0.035f, 0.065f),
        FLinearColor(0.95f, 0.48f, 0.035f),
        FLinearColor(0.035f, 0.48f, 0.72f)
    };
    static const FLinearColor GlassColor(0.025f, 0.12f, 0.18f);
    const FLinearColor Paint = Paints[FMath::Clamp(CarStyle, 0, 2)];
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (Basic)
    {
        UMaterialInstanceDynamic* BodyMat = UMaterialInstanceDynamic::Create(Basic, this);
        BodyMat->SetVectorParameterValue(TEXT("Color"), Paint);
        Chassis->SetMaterial(0, BodyMat);
        Cabin->SetMaterial(0, UMaterialInstanceDynamic::Create(Basic, this));
        if (UMaterialInstanceDynamic* CabinMat = Cast<UMaterialInstanceDynamic>(Cabin->GetMaterial(0)))
        {
            CabinMat->SetVectorParameterValue(TEXT("Color"), GlassColor);
        }
        UMaterialInstanceDynamic* NitroMat = UMaterialInstanceDynamic::Create(Basic, this);
        NitroMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.05f, 0.78f, 1.0f));
        FrontSpoiler->SetMaterial(0, BodyMat);
        NitroFlame->SetMaterial(0, NitroMat);
    }
    static const TCHAR* ModelPaths[] = {
        TEXT("/Game/Vehicles/AerionGT.AerionGT"),
        TEXT("/Game/Vehicles/VortexSVX.VortexSVX"),
        TEXT("/Game/Vehicles/TerraRallyS.TerraRallyS")
    };
    UStaticMesh* Imported = LoadObject<UStaticMesh>(nullptr, ModelPaths[FMath::Clamp(CarStyle, 0, 2)]);
    const bool bHasImportedModel = Imported != nullptr;
    if (bHasImportedModel)
    {
        ImportedModel->SetStaticMesh(Imported);
        const FBox ModelBounds = Imported->GetBoundingBox();
        const FVector ModelSize = ModelBounds.GetSize();
        const float ModelScale = ModelSize.X > KINDA_SMALL_NUMBER ? 420.0f / ModelSize.X : 1.0f;
        const FVector ModelCenter = ModelBounds.GetCenter();
        ImportedModel->SetRelativeScale3D(FVector(ModelScale));
        ImportedModel->SetRelativeLocation(FVector(-ModelCenter.X * ModelScale,
            -ModelCenter.Y * ModelScale, -48.0f - ModelBounds.Min.Z * ModelScale));
    }
    ImportedModel->SetVisibility(bHasImportedModel, true);
    Chassis->SetVisibility(!bHasImportedModel, true);
    Cabin->SetVisibility(!bHasImportedModel, true);
    FrontSpoiler->SetVisibility(!bHasImportedModel, true);
    for (UStaticMeshComponent* Wheel : Wheels)
    {
        if (Wheel) Wheel->SetVisibility(!bHasImportedModel, true);
    }
}

void AVelocityRushVehiclePawn::BeginPlay()
{
    Super::BeginPlay();
    BuildCarVisuals();
    UpdateCarColor();
}

void AVelocityRushVehiclePawn::SetCarStyle(int32 StyleIndex)
{
    CarStyle = ((StyleIndex % 3) + 3) % 3;
    const float WidthScale = CarStyle == 1 ? 1.05f : (CarStyle == 2 ? 0.96f : 1.0f);
    const float HeightScale = CarStyle == 1 ? 0.91f : (CarStyle == 2 ? 1.02f : 1.0f);
    Chassis->SetRelativeScale3D(FVector(1.0f, WidthScale, HeightScale));
    UpdateCarColor();
}

void AVelocityRushVehiclePawn::SetDrivingEnabled(bool bEnabled)
{
    bDrivingEnabled = bEnabled;
    if (!bEnabled)
    {
        ThrottleInput = 0.0f;
        SteeringInput = 0.0f;
        BrakeInput = 0.0f;
        TouchThrottleInput = 0.0f;
        TouchSteeringInput = 0.0f;
        TouchBrakeInput = 0.0f;
        bNitroPressed = false;
        bTouchNitroPressed = false;
        if (NitroFlame) NitroFlame->SetVisibility(false, true);
    }
}

void AVelocityRushVehiclePawn::SetAIPath(const TArray<FVector>& InPath, int32 StartIndex)
{
    AIPath = InPath;
    AIWaypoint = AIPath.Num() > 0 ? ((StartIndex % AIPath.Num()) + AIPath.Num()) % AIPath.Num() : 0;
    bAIControlled = true;
    bDrivingEnabled = true;
}

void AVelocityRushVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAxis(TEXT("Throttle"), this, &AVelocityRushVehiclePawn::SetThrottleInput);
    PlayerInputComponent->BindAxis(TEXT("Steering"), this, &AVelocityRushVehiclePawn::SetSteeringInput);
    PlayerInputComponent->BindAxis(TEXT("Brake"), this, &AVelocityRushVehiclePawn::SetBrakeInput);
    PlayerInputComponent->BindAction(TEXT("Nitro"), IE_Pressed, this, &AVelocityRushVehiclePawn::PressNitro);
    PlayerInputComponent->BindAction(TEXT("Nitro"), IE_Released, this, &AVelocityRushVehiclePawn::ReleaseNitro);
}

void AVelocityRushVehiclePawn::UpdateAI(float DeltaSeconds)
{
    if (AIPath.Num() < 2)
    {
        return;
    }
    const FVector Target = AIPath[AIWaypoint];
    const FVector LocalTarget = GetActorTransform().InverseTransformPosition(Target);
    if (FVector::Dist2D(GetActorLocation(), Target) < 520.0f)
    {
        AIWaypoint = (AIWaypoint + 1) % AIPath.Num();
    }
    SteeringInput = FMath::Clamp(LocalTarget.Y / 520.0f, -1.0f, 1.0f);
    ThrottleInput = 1.0f;
    BrakeInput = FMath::Abs(SteeringInput) > 0.90f ? 0.22f : 0.0f;
    bNitroPressed = SpeedKmh < 165.0f && NitroPercent > 60.0f;
}

void AVelocityRushVehiclePawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bDrivingEnabled || DeltaSeconds <= 0.0f)
    {
        return;
    }
    CollisionCooldown = FMath::Max(0.0f, CollisionCooldown - DeltaSeconds);
    if (bAIControlled)
    {
        UpdateAI(DeltaSeconds);
    }
    float Throttle = bAIControlled ? ThrottleInput : FMath::Clamp(ThrottleInput + TouchThrottleInput, -0.45f, 1.0f);
    const float Steering = bAIControlled ? SteeringInput : FMath::Clamp(SteeringInput + TouchSteeringInput, -1.0f, 1.0f);
    const float Brake = bAIControlled ? BrakeInput : FMath::Max(BrakeInput, TouchBrakeInput);
    const bool bBoostRequested = bNitroPressed || bTouchNitroPressed;
    const bool bBoosting = bBoostRequested && Throttle > 0.0f && NitroPercent > 0.0f;
    if (FuelPercent <= 0.0f)
    {
        Throttle = FMath::Min(Throttle, 0.0f);
    }
    const float Acceleration = CarStyle == 1 ? 45.0f : (CarStyle == 2 ? 34.0f : 38.0f);
    const float TopSpeed = CarStyle == 1 ? 340.0f : (CarStyle == 2 ? 295.0f : 315.0f);
    SpeedKmh += Throttle * Acceleration * (bBoosting ? 1.85f : 1.0f) * DeltaSeconds;
    SpeedKmh -= Brake * 150.0f * DeltaSeconds;
    SpeedKmh -= (Throttle <= 0.0f ? 7.0f : 2.0f) * DeltaSeconds;
    SpeedKmh = FMath::Clamp(SpeedKmh, 0.0f, TopSpeed * (bBoosting ? 1.12f : 1.0f));

    if (Throttle > 0.0f)
    {
        FuelPercent = FMath::Max(0.0f, FuelPercent - DeltaSeconds * (0.48f + (bBoosting ? 0.18f : 0.0f)));
    }
    if (bBoosting)
    {
        NitroPercent = FMath::Max(0.0f, NitroPercent - DeltaSeconds * 30.0f);
    }
    else
    {
        NitroPercent = FMath::Min(100.0f, NitroPercent + DeltaSeconds * 5.0f);
    }
    NitroFlame->SetVisibility(bBoosting, true);

    const float Travel = SpeedKmh / 3.6f * DeltaSeconds;
    FHitResult Hit;
    AddActorLocalOffset(FVector(Travel, 0.0f, 0.0f), true, &Hit);
    if (Hit.IsValidBlockingHit() && CollisionCooldown <= 0.0f)
    {
        ApplyCollisionPenalty();
        CollisionCooldown = 0.85f;
    }
    AddActorLocalRotation(FRotator(0.0f, Steering * FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 300.0f), FVector2D(0.0f, 86.0f), SpeedKmh) * DeltaSeconds, 0.0f));
    DistanceMeters += Travel / 100.0f;
    Score = FMath::Max(0, FMath::RoundToInt(DistanceMeters * 8.0f + SpeedKmh * 2.0f) + BonusScore - CollisionPenalty);

    if (FuelPercent <= 0.0f && SpeedKmh < 1.0f && !bAIControlled)
    {
        if (AVelocityRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AVelocityRushGameMode>())
        {
            GameMode->FinishRace(false);
        }
    }
}

void AVelocityRushVehiclePawn::AddFuel(float Amount)
{
    FuelPercent = FMath::Clamp(FuelPercent + Amount, 0.0f, 100.0f);
}

void AVelocityRushVehiclePawn::AddNitro(float Amount)
{
    NitroPercent = FMath::Clamp(NitroPercent + Amount, 0.0f, 100.0f);
}

void AVelocityRushVehiclePawn::AddBonusScore(int32 Amount)
{
    BonusScore = FMath::Max(0, BonusScore + Amount);
}

void AVelocityRushVehiclePawn::ApplyCollisionPenalty()
{
    SpeedKmh *= 0.38f;
    CollisionPenalty += 250;
    Score = FMath::Max(0, Score - 250);
    if (!bAIControlled)
    {
        if (AVelocityRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AVelocityRushGameMode>())
        {
            GameMode->RegisterCollision();
        }
    }
}

void AVelocityRushVehiclePawn::PressNitro() { bNitroPressed = true; }
void AVelocityRushVehiclePawn::ReleaseNitro() { bNitroPressed = false; }
