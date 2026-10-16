#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VelocityRushTrackActor.generated.h"

class UInstancedStaticMeshComponent;
class UDirectionalLightComponent;
class UProceduralMeshComponent;
class UStaticMeshComponent;
class USkyLightComponent;
class USceneComponent;
class UStaticMesh;
class UMaterialInterface;

UCLASS()
class VELOCITYRUSH_API AVelocityRushTrackActor : public AActor
{
    GENERATED_BODY()

public:
    AVelocityRushTrackActor();
    virtual void BeginPlay() override;

    const TArray<FVector>& GetRoute() const { return Route; }
    FVector GetRouteLocation(int32 Index) const;
    FRotator GetRouteRotation(int32 Index) const;
    int32 GetRouteCount() const { return Route.Num(); }
    float GetRoadWidth() const { return RoadWidth; }

private:
    void BuildRoute();
    void BuildRoad();
    void BuildBarriers();
    void BuildCity();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProceduralMeshComponent> RoadMesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> CityGround;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UDirectionalLightComponent> SunLight;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USkyLightComponent> SkyLight;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UInstancedStaticMeshComponent> BarrierInstances;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UInstancedStaticMeshComponent> BuildingInstances;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BasicMaterial;

    UPROPERTY(VisibleAnywhere, Category="Track")
    TArray<FVector> Route;

    UPROPERTY(EditDefaultsOnly, Category="Track", meta=(ClampMin="600.0"))
    float RoadWidth = 1400.0f;
};
