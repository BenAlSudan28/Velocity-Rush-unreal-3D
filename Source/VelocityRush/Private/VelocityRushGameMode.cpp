#include "VelocityRushGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "VelocityRushPickup.h"
#include "VelocityRushPlayerController.h"
#include "VelocityRushTrackActor.h"
#include "VelocityRushVehiclePawn.h"

AVelocityRushGameMode::AVelocityRushGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PlayerControllerClass = AVelocityRushPlayerController::StaticClass();
    DefaultPawnClass = nullptr;
}

void AVelocityRushGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (GetWorld())
    {
        Track = GetWorld()->SpawnActor<AVelocityRushTrackActor>();
    }
}

void AVelocityRushGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bRaceActive && !bFreeDrive && !UGameplayStatics::IsGamePaused(this))
    {
        TimeRemaining = FMath::Max(0.0f, TimeRemaining - DeltaSeconds);
        if (TimeRemaining <= 0.0f)
        {
            FinishRace(true);
        }
    }
}

void AVelocityRushGameMode::ClearVehicles()
{
    if (IsValid(PlayerVehicle))
    {
        PlayerVehicle->Destroy();
    }
    for (AVelocityRushVehiclePawn* Rival : Rivals)
    {
        if (IsValid(Rival))
        {
            Rival->Destroy();
        }
    }
    for (AVelocityRushPickup* Pickup : Pickups)
    {
        if (IsValid(Pickup))
        {
            Pickup->Destroy();
        }
    }
    PlayerVehicle = nullptr;
    Rivals.Reset();
    Pickups.Reset();
}

void AVelocityRushGameMode::StartRace(bool bStartFreeDrive, int32 SelectedCarStyle)
{
    if (!GetWorld() || !IsValid(Track) || Track->GetRouteCount() < 4)
    {
        return;
    }
    UGameplayStatics::SetGamePaused(this, false);
    ClearVehicles();
    bFreeDrive = bStartFreeDrive;
    bRaceActive = true;
    CollisionCount = 0;
    FinalScore = 0;
    TimeRemaining = bFreeDrive ? 0.0f : 120.0f;

    const FVector Start = Track->GetRouteLocation(0) + FVector(0.0f, 0.0f, 54.0f);
    const FRotator Facing = Track->GetRouteRotation(0);
    PlayerVehicle = GetWorld()->SpawnActor<AVelocityRushVehiclePawn>(Start, Facing);
    if (!IsValid(PlayerVehicle))
    {
        bRaceActive = false;
        return;
    }
    PlayerVehicle->SetCarStyle(SelectedCarStyle);
    PlayerVehicle->SetDrivingEnabled(true);

    if (!bFreeDrive)
    {
        SpawnRivals();
    }
    SpawnPickups();
    if (AVelocityRushPlayerController* Controller = Cast<AVelocityRushPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        Controller->Possess(PlayerVehicle);
        Controller->SetTrackedVehicle(PlayerVehicle);
        Controller->ShowRaceHUD();
    }
}

void AVelocityRushGameMode::SpawnRivals()
{
    const TArray<FVector>& Route = Track->GetRoute();
    if (Route.Num() < 8)
    {
        return;
    }
    for (int32 I = 0; I < 4; ++I)
    {
        const int32 RouteIndex = 2 + I * 2;
        const FRotator Facing = Track->GetRouteRotation(RouteIndex);
        const FVector Side = FVector::CrossProduct(FVector::UpVector, Facing.Vector()).GetSafeNormal();
        const float LaneOffset = (I % 2 == 0) ? -300.0f : 300.0f;
        const FVector Location = Track->GetRouteLocation(RouteIndex) + Side * LaneOffset + FVector(0.0f, 0.0f, 54.0f);
        AVelocityRushVehiclePawn* Rival = GetWorld()->SpawnActor<AVelocityRushVehiclePawn>(Location, Facing);
        if (IsValid(Rival))
        {
            Rival->SetCarStyle((I + 1) % 3);
            Rival->SetAIPath(Route, (RouteIndex + 1) % Route.Num());
            Rivals.Add(Rival);
        }
    }
}

void AVelocityRushGameMode::SpawnPickups()
{
    if (!GetWorld() || !IsValid(Track))
    {
        return;
    }
    const int32 RouteCount = Track->GetRouteCount();
    for (int32 I = 10; I < RouteCount; I += 12)
    {
        const FVector Location = Track->GetRouteLocation(I) + FVector(0.0f, 0.0f, 118.0f);
        AVelocityRushPickup* Pickup = GetWorld()->SpawnActor<AVelocityRushPickup>(Location, FRotator::ZeroRotator);
        if (IsValid(Pickup))
        {
            Pickup->SetKind(((I / 12) % 2 == 0) ? ERushPickupKind::Fuel : ERushPickupKind::Nitro);
            Pickups.Add(Pickup);
        }
    }
}

void AVelocityRushGameMode::FinishRace(bool bWon)
{
    if (!bRaceActive)
    {
        return;
    }
    bRaceActive = false;
    if (IsValid(PlayerVehicle))
    {
        PlayerVehicle->SetDrivingEnabled(false);
        FinalScore = PlayerVehicle->GetScore();
    }
    UGameplayStatics::SetGamePaused(this, true);
    if (AVelocityRushPlayerController* Controller = Cast<AVelocityRushPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        Controller->ShowResults(bWon, FinalScore, PlayerVehicle ? PlayerVehicle->GetDistanceMeters() : 0.0f, CollisionCount);
    }
}

void AVelocityRushGameMode::RegisterCollision()
{
    ++CollisionCount;
}

void AVelocityRushGameMode::ReturnToMenu()
{
    UGameplayStatics::SetGamePaused(this, false);
    bRaceActive = false;
    ClearVehicles();
}

void AVelocityRushGameMode::SetRacePaused(bool bPaused)
{
    UGameplayStatics::SetGamePaused(this, bPaused);
    if (IsValid(PlayerVehicle))
    {
        PlayerVehicle->SetDrivingEnabled(!bPaused);
    }
    if (AVelocityRushPlayerController* Controller = Cast<AVelocityRushPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        if (bPaused)
        {
            Controller->ShowPauseMenu();
        }
        else
        {
            Controller->ShowRaceHUD();
        }
    }
}
