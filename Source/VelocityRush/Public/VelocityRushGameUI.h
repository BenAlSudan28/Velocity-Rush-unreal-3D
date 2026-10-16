#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VelocityRushGameUI.generated.h"

class AVelocityRushPlayerController;
class AVelocityRushVehiclePawn;
class UButton;
class UCanvasPanel;
class UProgressBar;
class UTextBlock;

UCLASS()
class VELOCITYRUSH_API URushGameUI : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetController(AVelocityRushPlayerController* InController);
    void SetVehicle(AVelocityRushVehiclePawn* InVehicle);
    void ShowMainMenu();
    void ShowRaceHUD();
    void ShowPauseMenu();
    void ShowResults(bool bWon, int32 Score, float DistanceMeters, int32 Collisions);

protected:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    void ResetCanvas();
    UTextBlock* AddText(const FString& Text, const FAnchors& Anchors, const FMargin& Offsets, int32 FontSize, FLinearColor Color, EHorizontalAlignment Alignment = HAlign_Center);
    UButton* AddButton(const FString& Text, const FAnchors& Anchors, const FMargin& Offsets, bool bPrimary);
    UProgressBar* AddBar(const FAnchors& Anchors, const FMargin& Offsets, FLinearColor FillColor);

    UFUNCTION() void HandleRace();
    UFUNCTION() void HandleFreeDrive();
    UFUNCTION() void HandleNextCar();
    UFUNCTION() void HandlePreviousCar();
    UFUNCTION() void HandlePause();
    UFUNCTION() void HandleResume();
    UFUNCTION() void HandleRetry();
    UFUNCTION() void HandleMenu();
    UFUNCTION() void HandleLeftPressed();
    UFUNCTION() void HandleRightPressed();
    UFUNCTION() void HandleLeftReleased();
    UFUNCTION() void HandleRightReleased();
    UFUNCTION() void HandleThrottlePressed();
    UFUNCTION() void HandleThrottleReleased();
    UFUNCTION() void HandleBrakePressed();
    UFUNCTION() void HandleBrakeReleased();
    UFUNCTION() void HandleNitroPressed();
    UFUNCTION() void HandleNitroReleased();

    void UpdateTouchState();
    void ClearTouchState();

    UPROPERTY() TObjectPtr<UCanvasPanel> RootCanvas;
    UPROPERTY() TObjectPtr<UTextBlock> SpeedText;
    UPROPERTY() TObjectPtr<UTextBlock> TelemetryText;
    UPROPERTY() TObjectPtr<UTextBlock> FuelText;
    UPROPERTY() TObjectPtr<UTextBlock> NitroText;
    UPROPERTY() TObjectPtr<UTextBlock> CarNameText;
    UPROPERTY() TObjectPtr<UProgressBar> FuelBar;
    UPROPERTY() TObjectPtr<UProgressBar> NitroBar;
    UPROPERTY() TWeakObjectPtr<AVelocityRushPlayerController> Controller;
    UPROPERTY() TWeakObjectPtr<AVelocityRushVehiclePawn> Vehicle;
    bool bTouchLeft = false;
    bool bTouchRight = false;
    bool bTouchThrottle = false;
    bool bTouchBrake = false;
    bool bTouchNitro = false;
};
