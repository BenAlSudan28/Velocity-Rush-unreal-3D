#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VelocityRushVehiclePawn.generated.h"

class UBoxComponent;
class UCameraComponent;
class UProceduralMeshComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UInputComponent;

UCLASS()
class VELOCITYRUSH_API AVelocityRushVehiclePawn : public APawn
{
    GENERATED_BODY()

public:
    AVelocityRushVehiclePawn();
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    void SetCarStyle(int32 StyleIndex);
    void SetAIPath(const TArray<FVector>& InPath, int32 StartIndex);
    void SetAIControlled(bool bEnabled) { bAIControlled = bEnabled; }
    void SetDrivingEnabled(bool bEnabled);
    void SetThrottleInput(float Value) { ThrottleInput = FMath::Clamp(Value, -0.45f, 1.0f); }
    void SetSteeringInput(float Value) { SteeringInput = FMath::Clamp(Value, -1.0f, 1.0f); }
    void SetBrakeInput(float Value) { BrakeInput = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetNitroPressed(bool bPressed) { bNitroPressed = bPressed; }
    void SetTouchThrottleInput(float Value) { TouchThrottleInput = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetTouchSteeringInput(float Value) { TouchSteeringInput = FMath::Clamp(Value, -1.0f, 1.0f); }
    void SetTouchBrakeInput(float Value) { TouchBrakeInput = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetTouchNitroPressed(bool bPressed) { bTouchNitroPressed = bPressed; }
    void AddFuel(float Amount);
    void AddNitro(float Amount);
    void AddBonusScore(int32 Amount);
    void ApplyCollisionPenalty();

    float GetSpeedKmh() const { return SpeedKmh; }
    float GetFuelPercent() const { return FuelPercent; }
    float GetNitroPercent() const { return NitroPercent; }
    float GetDistanceMeters() const { return DistanceMeters; }
    int32 GetScore() const { return Score; }
    int32 GetCarStyle() const { return CarStyle; }
    bool IsAIControlled() const { return bAIControlled; }
    bool IsOutOfFuel() const { return FuelPercent <= 0.0f; }

protected:
    virtual void BeginPlay() override;

private:
    void PressNitro();
    void ReleaseNitro();
    void BuildCarVisuals();
    void UpdateCarColor();
    void UpdateAI(float DeltaSeconds);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBoxComponent> CollisionBody;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProceduralMeshComponent> Chassis;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> ImportedModel;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Cabin;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> FrontSpoiler;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> NitroFlame;

    UPROPERTY(VisibleAnywhere)
    TArray<TObjectPtr<UStaticMeshComponent>> Wheels;

    UPROPERTY()
    TArray<FVector> AIPath;

    UPROPERTY(EditAnywhere, Category="Vehicle")
    int32 CarStyle = 0;

    UPROPERTY(VisibleAnywhere, Category="Telemetry")
    float SpeedKmh = 0.0f;

    UPROPERTY(VisibleAnywhere, Category="Telemetry")
    float FuelPercent = 100.0f;

    UPROPERTY(VisibleAnywhere, Category="Telemetry")
    float NitroPercent = 100.0f;

    UPROPERTY(VisibleAnywhere, Category="Telemetry")
    float DistanceMeters = 0.0f;

    UPROPERTY(VisibleAnywhere, Category="Telemetry")
    int32 Score = 0;

    int32 BonusScore = 0;
    int32 CollisionPenalty = 0;

    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;
    float BrakeInput = 0.0f;
    float TouchThrottleInput = 0.0f;
    float TouchSteeringInput = 0.0f;
    float TouchBrakeInput = 0.0f;
    int32 AIWaypoint = 0;
    float CollisionCooldown = 0.0f;
    bool bAIControlled = false;
    bool bDrivingEnabled = false;
    bool bNitroPressed = false;
    bool bTouchNitroPressed = false;
};
