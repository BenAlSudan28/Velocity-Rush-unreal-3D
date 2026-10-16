#include "VelocityRushPlayerController.h"
#include "Camera/CameraActor.h"
#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RotationMatrix.h"
#include "VelocityRushGameMode.h"
#include "VelocityRushGameUI.h"
#include "VelocityRushVehiclePawn.h"

void AVelocityRushPlayerController::BeginPlay()
{
    Super::BeginPlay();
    const FVector CameraLocation(0.0f, -9200.0f, 6200.0f);
    MenuCamera = GetWorld()->SpawnActor<ACameraActor>(CameraLocation, FRotationMatrix::MakeFromX(-CameraLocation).Rotator());
    if (MenuCamera)
    {
        SetViewTarget(MenuCamera);
    }
    EnsureUI();
    ShowMainMenu();
}

void AVelocityRushPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (InputComponent)
    {
        InputComponent->BindAction(TEXT("Pause"), IE_Pressed, this, &AVelocityRushPlayerController::TogglePause);
    }
}

void AVelocityRushPlayerController::EnsureUI()
{
    if (!GameUI)
    {
        GameUI = CreateWidget<URushGameUI>(this, URushGameUI::StaticClass());
        if (GameUI)
        {
            GameUI->SetController(this);
            GameUI->AddToViewport(10);
        }
    }
}

void AVelocityRushPlayerController::SetMenuInputMode(bool bMenu)
{
    bShowMouseCursor = bMenu;
    FInputModeGameAndUI Mode;
    if (GameUI)
    {
        Mode.SetWidgetToFocus(GameUI->TakeWidget());
    }
    Mode.SetHideCursorDuringCapture(!bMenu);
    SetInputMode(Mode);
}

void AVelocityRushPlayerController::ShowMainMenu()
{
    EnsureUI();
    if (MenuCamera)
    {
        SetViewTarget(MenuCamera);
    }
    if (GameUI)
    {
        GameUI->SetVehicle(nullptr);
        GameUI->ShowMainMenu();
    }
    SetMenuInputMode(true);
}

void AVelocityRushPlayerController::ShowRaceHUD()
{
    EnsureUI();
    if (GameUI)
    {
        GameUI->SetVehicle(TrackedVehicle);
        GameUI->ShowRaceHUD();
    }
    SetMenuInputMode(false);
}

void AVelocityRushPlayerController::ShowPauseMenu()
{
    EnsureUI();
    if (GameUI)
    {
        GameUI->SetVehicle(TrackedVehicle);
        GameUI->ShowPauseMenu();
    }
    SetMenuInputMode(true);
}

void AVelocityRushPlayerController::ShowResults(bool bWon, int32 Score, float DistanceMeters, int32 Collisions)
{
    EnsureUI();
    if (GameUI)
    {
        GameUI->ShowResults(bWon, Score, DistanceMeters, Collisions);
    }
    SetMenuInputMode(true);
}

void AVelocityRushPlayerController::StartRace(bool bFreeDrive)
{
    bLastRunFreeDrive = bFreeDrive;
    if (AVelocityRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AVelocityRushGameMode>())
    {
        GameMode->StartRace(bFreeDrive, SelectedCarStyle);
    }
}

void AVelocityRushPlayerController::ChangeCar(int32 Step)
{
    SelectedCarStyle = (SelectedCarStyle + Step + 3) % 3;
    ShowMainMenu();
}

void AVelocityRushPlayerController::SetTrackedVehicle(AVelocityRushVehiclePawn* Vehicle)
{
    TrackedVehicle = Vehicle;
    if (IsValid(Vehicle))
    {
        SetViewTargetWithBlend(Vehicle, 0.55f);
    }
    if (GameUI)
    {
        GameUI->SetVehicle(Vehicle);
    }
}

void AVelocityRushPlayerController::TogglePause()
{
    if (AVelocityRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AVelocityRushGameMode>())
    {
        if (GameMode->IsRaceActive())
        {
            GameMode->SetRacePaused(!UGameplayStatics::IsGamePaused(this));
        }
    }
}

void AVelocityRushPlayerController::ResumeRace()
{
    if (AVelocityRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AVelocityRushGameMode>())
    {
        GameMode->SetRacePaused(false);
    }
}

void AVelocityRushPlayerController::RetryRace()
{
    StartRace(bLastRunFreeDrive);
}

void AVelocityRushPlayerController::ReturnToMenu()
{
    if (AVelocityRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AVelocityRushGameMode>())
    {
        GameMode->ReturnToMenu();
    }
    TrackedVehicle = nullptr;
    ShowMainMenu();
}

void AVelocityRushPlayerController::SetTouchThrottle(float Value)
{
    if (TrackedVehicle) TrackedVehicle->SetTouchThrottleInput(Value);
}

void AVelocityRushPlayerController::SetTouchSteering(float Value)
{
    if (TrackedVehicle) TrackedVehicle->SetTouchSteeringInput(Value);
}

void AVelocityRushPlayerController::SetTouchBrake(float Value)
{
    if (TrackedVehicle) TrackedVehicle->SetTouchBrakeInput(Value);
}

void AVelocityRushPlayerController::SetTouchNitro(bool bPressed)
{
    if (TrackedVehicle) TrackedVehicle->SetTouchNitroPressed(bPressed);
}
