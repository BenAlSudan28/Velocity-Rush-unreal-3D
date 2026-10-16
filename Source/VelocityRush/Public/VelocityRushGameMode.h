#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VelocityRushGameMode.generated.h"

class AVelocityRushTrackActor;
class AVelocityRushVehiclePawn;
class AVelocityRushPickup;

UCLASS()
class VELOCITYRUSH_API AVelocityRushGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AVelocityRushGameMode();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    void StartRace(bool bFreeDrive, int32 SelectedCarStyle);
    void FinishRace(bool bWon);
    void ReturnToMenu();
    void RegisterCollision();
    void SetRacePaused(bool bPaused);

    AVelocityRushTrackActor* GetTrack() const { return Track; }
    AVelocityRushVehiclePawn* GetPlayerVehicle() const { return PlayerVehicle; }
    float GetTimeRemaining() const { return TimeRemaining; }
    int32 GetCollisionCount() const { return CollisionCount; }
    int32 GetFinalScore() const { return FinalScore; }
    bool IsFreeDrive() const { return bFreeDrive; }
    bool IsRaceActive() const { return bRaceActive; }

private:
    void ClearVehicles();
    void SpawnRivals();
    void SpawnPickups();

    UPROPERTY()
    TObjectPtr<AVelocityRushTrackActor> Track;

    UPROPERTY()
    TObjectPtr<AVelocityRushVehiclePawn> PlayerVehicle;

    UPROPERTY()
    TArray<TObjectPtr<AVelocityRushVehiclePawn>> Rivals;

    UPROPERTY()
    TArray<TObjectPtr<AVelocityRushPickup>> Pickups;

    float TimeRemaining = 120.0f;
    int32 CollisionCount = 0;
    int32 FinalScore = 0;
    bool bFreeDrive = false;
    bool bRaceActive = false;
};
