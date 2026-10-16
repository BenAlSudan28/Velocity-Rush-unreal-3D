#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VelocityRushPlayerController.generated.h"

class AVelocityRushVehiclePawn;
class ACameraActor;
class URushGameUI;

UCLASS()
class VELOCITYRUSH_API AVelocityRushPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    void ShowMainMenu();
    void ShowRaceHUD();
    void ShowPauseMenu();
    void ShowResults(bool bWon, int32 Score, float DistanceMeters, int32 Collisions);
    void StartRace(bool bFreeDrive);
    void ChangeCar(int32 Step);
    void SetTrackedVehicle(AVelocityRushVehiclePawn* Vehicle);
    AVelocityRushVehiclePawn* GetTrackedVehicle() const { return TrackedVehicle; }
    int32 GetSelectedCarStyle() const { return SelectedCarStyle; }
    void TogglePause();
    void ResumeRace();
    void RetryRace();
    void ReturnToMenu();

    void SetTouchThrottle(float Value);
    void SetTouchSteering(float Value);
    void SetTouchBrake(float Value);
    void SetTouchNitro(bool bPressed);

private:
    void EnsureUI();
    void SetMenuInputMode(bool bMenu);

    UPROPERTY()
    TObjectPtr<URushGameUI> GameUI;

    UPROPERTY()
    TObjectPtr<ACameraActor> MenuCamera;

    UPROPERTY()
    TObjectPtr<AVelocityRushVehiclePawn> TrackedVehicle;

    int32 SelectedCarStyle = 0;
    bool bLastRunFreeDrive = false;
};
