#include "VelocityRushPickup.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "VelocityRushVehiclePawn.h"

AVelocityRushPickup::AVelocityRushPickup()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    PickupVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupVolume"));
    PickupVolume->SetupAttachment(SceneRoot);
    PickupVolume->SetBoxExtent(FVector(90.0f, 90.0f, 100.0f));
    PickupVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    PickupVolume->SetGenerateOverlapEvents(true);
    PickupVolume->OnComponentBeginOverlap.AddDynamic(this, &AVelocityRushPickup::HandleOverlap);

    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
    PickupMesh->SetupAttachment(SceneRoot);
    PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereFinder.Succeeded()) PickupMesh->SetStaticMesh(SphereFinder.Object);
    PickupMesh->SetRelativeScale3D(FVector(0.72f));
    SetKind(ERushPickupKind::Fuel);
}

void AVelocityRushPickup::SetKind(ERushPickupKind InKind)
{
    Kind = InKind;
    if (!PickupMesh) return;
    UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (Base)
    {
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, this);
        const FLinearColor Color = Kind == ERushPickupKind::Fuel
            ? FLinearColor(0.95f, 0.55f, 0.08f)
            : FLinearColor(0.05f, 0.70f, 1.0f);
        Material->SetVectorParameterValue(TEXT("Color"), Color);
        PickupMesh->SetMaterial(0, Material);
    }
}

void AVelocityRushPickup::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AVelocityRushVehiclePawn* Vehicle = Cast<AVelocityRushVehiclePawn>(OtherActor);
    if (!IsValid(Vehicle) || Vehicle->IsAIControlled()) return;
    if (Kind == ERushPickupKind::Fuel)
    {
        Vehicle->AddFuel(32.0f);
    }
    else
    {
        Vehicle->AddNitro(45.0f);
    }
    Vehicle->AddBonusScore(150);
    Destroy();
}
