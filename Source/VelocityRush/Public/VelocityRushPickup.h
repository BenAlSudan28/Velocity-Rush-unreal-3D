#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VelocityRushPickup.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class USceneComponent;

UENUM()
enum class ERushPickupKind : uint8
{
    Fuel,
    Nitro
};

UCLASS()
class VELOCITYRUSH_API AVelocityRushPickup : public AActor
{
    GENERATED_BODY()

public:
    AVelocityRushPickup();
    void SetKind(ERushPickupKind InKind);

private:
    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBoxComponent> PickupVolume;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> PickupMesh;

    UPROPERTY()
    ERushPickupKind Kind = ERushPickupKind::Fuel;
};
