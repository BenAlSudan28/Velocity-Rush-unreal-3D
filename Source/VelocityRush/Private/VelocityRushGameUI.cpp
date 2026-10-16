#include "VelocityRushGameUI.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "VelocityRushGameMode.h"
#include "VelocityRushPlayerController.h"
#include "VelocityRushVehiclePawn.h"

void URushGameUI::SetController(AVelocityRushPlayerController* InController)
{
    Controller = InController;
}

void URushGameUI::SetVehicle(AVelocityRushVehiclePawn* InVehicle)
{
    Vehicle = InVehicle;
}

void URushGameUI::ResetCanvas()
{
    ClearTouchState();
    if (!WidgetTree)
    {
        return;
    }
    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RuntimeCanvas"));
    WidgetTree->RootWidget = RootCanvas;
    SpeedText = nullptr;
    TelemetryText = nullptr;
    FuelText = nullptr;
    NitroText = nullptr;
    CarNameText = nullptr;
    FuelBar = nullptr;
    NitroBar = nullptr;
}

UTextBlock* URushGameUI::AddText(const FString& Text, const FAnchors& Anchors, const FMargin& Offsets, int32 FontSize, FLinearColor Color, EHorizontalAlignment Alignment)
{
    if (!RootCanvas) return nullptr;
    UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Label->SetText(FText::FromString(Text));
    Label->SetColorAndOpacity(FSlateColor(Color));
    Label->SetJustification(Alignment == HAlign_Left ? ETextJustify::Left : (Alignment == HAlign_Right ? ETextJustify::Right : ETextJustify::Center));
    FSlateFontInfo Font = Label->GetFont();
    Font.Size = FontSize;
    Label->SetFont(Font);
    if (UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(Label))
    {
        Slot->SetAnchors(Anchors);
        Slot->SetOffsets(Offsets);
    }
    return Label;
}

UButton* URushGameUI::AddButton(const FString& Text, const FAnchors& Anchors, const FMargin& Offsets, bool bPrimary)
{
    if (!RootCanvas) return nullptr;
    UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    Button->SetBackgroundColor(bPrimary ? FLinearColor(0.015f, 0.55f, 0.67f, 0.96f) : FLinearColor(0.025f, 0.065f, 0.085f, 0.96f));
    UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Label->SetText(FText::FromString(Text));
    Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    Label->SetJustification(ETextJustify::Center);
    FSlateFontInfo Font = Label->GetFont();
    Font.Size = 22;
    Label->SetFont(Font);
    Button->AddChild(Label);
    if (UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(Button))
    {
        Slot->SetAnchors(Anchors);
        Slot->SetOffsets(Offsets);
    }
    return Button;
}

UProgressBar* URushGameUI::AddBar(const FAnchors& Anchors, const FMargin& Offsets, FLinearColor FillColor)
{
    if (!RootCanvas) return nullptr;
    UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
    Bar->SetPercent(1.0f);
    Bar->SetFillColorAndOpacity(FillColor);
    if (UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(Bar))
    {
        Slot->SetAnchors(Anchors);
        Slot->SetOffsets(Offsets);
    }
    return Bar;
}

void URushGameUI::ShowMainMenu()
{
    ResetCanvas();
    AddText(TEXT("VELOCITY RUSH 3D"), FAnchors(0.22f, 0.075f, 0.78f, 0.15f), FMargin(0), 42, FLinearColor(0.92f, 0.98f, 1.0f));
    AddText(TEXT("سباق المدينة • اختر سيارتك"), FAnchors(0.24f, 0.15f, 0.76f, 0.205f), FMargin(0), 22, FLinearColor(0.10f, 0.78f, 0.88f));
    static const TCHAR* CarNames[] = { TEXT("AERION GT-R • إيطالية رياضية"), TEXT("VORTEX SV-X • سوبركار"), TEXT("TERRA RALLY S • يابانية رياضية") };
    const int32 Selected = Controller.IsValid() ? Controller->GetSelectedCarStyle() : 0;
    CarNameText = AddText(CarNames[FMath::Clamp(Selected, 0, 2)], FAnchors(0.26f, 0.25f, 0.74f, 0.31f), FMargin(0), 22, FLinearColor(0.94f, 0.72f, 0.30f));
    if (UButton* Previous = AddButton(TEXT("◀ السابقة"), FAnchors(0.30f, 0.32f, 0.46f, 0.38f), FMargin(0), false))
        Previous->OnClicked.AddDynamic(this, &URushGameUI::HandlePreviousCar);
    if (UButton* Next = AddButton(TEXT("التالية ▶"), FAnchors(0.54f, 0.32f, 0.70f, 0.38f), FMargin(0), false))
        Next->OnClicked.AddDynamic(this, &URushGameUI::HandleNextCar);
    if (UButton* Race = AddButton(TEXT("ابدأ سباق السرعة"), FAnchors(0.35f, 0.44f, 0.65f, 0.53f), FMargin(0), true))
        Race->OnClicked.AddDynamic(this, &URushGameUI::HandleRace);
    if (UButton* Free = AddButton(TEXT("قيادة حرة"), FAnchors(0.35f, 0.56f, 0.65f, 0.65f), FMargin(0), false))
        Free->OnClicked.AddDynamic(this, &URushGameUI::HandleFreeDrive);
    AddText(TEXT("WASD / الأسهم • Space فرامل • Shift نيترو"), FAnchors(0.25f, 0.79f, 0.75f, 0.84f), FMargin(0), 16, FLinearColor(0.68f, 0.80f, 0.86f));
    AddText(TEXT("تصاميم أصلية خيالية • لا تتطلب اتصالًا بالإنترنت"), FAnchors(0.25f, 0.86f, 0.75f, 0.90f), FMargin(0), 14, FLinearColor(0.43f, 0.57f, 0.66f));
}

void URushGameUI::ShowRaceHUD()
{
    ResetCanvas();
    SpeedText = AddText(TEXT("000 كم/س"), FAnchors(0.025f, 0.025f, 0.26f, 0.09f), FMargin(10), 30, FLinearColor(0.94f, 0.98f, 1.0f), HAlign_Left);
    TelemetryText = AddText(TEXT("المسافة 0 م   •   النقاط 0"), FAnchors(0.025f, 0.09f, 0.42f, 0.14f), FMargin(10), 17, FLinearColor(0.64f, 0.81f, 0.87f), HAlign_Left);
    FuelText = AddText(TEXT("الوقود 100%"), FAnchors(0.74f, 0.035f, 0.96f, 0.075f), FMargin(4), 16, FLinearColor(0.55f, 0.95f, 0.62f), HAlign_Right);
    FuelBar = AddBar(FAnchors(0.74f, 0.08f, 0.96f, 0.105f), FMargin(2), FLinearColor(0.18f, 0.82f, 0.35f));
    NitroText = AddText(TEXT("نيترو 100%"), FAnchors(0.74f, 0.115f, 0.96f, 0.155f), FMargin(4), 16, FLinearColor(0.26f, 0.86f, 0.98f), HAlign_Right);
    NitroBar = AddBar(FAnchors(0.74f, 0.16f, 0.96f, 0.185f), FMargin(2), FLinearColor(0.05f, 0.75f, 0.97f));
    if (UButton* Pause = AddButton(TEXT("إيقاف Ⅱ"), FAnchors(0.87f, 0.21f, 0.97f, 0.27f), FMargin(0), false))
        Pause->OnClicked.AddDynamic(this, &URushGameUI::HandlePause);

    if (UButton* Left = AddButton(TEXT("◀"), FAnchors(0.035f, 0.72f, 0.145f, 0.88f), FMargin(0), false))
    {
        Left->OnPressed.AddDynamic(this, &URushGameUI::HandleLeftPressed);
        Left->OnReleased.AddDynamic(this, &URushGameUI::HandleLeftReleased);
    }
    if (UButton* Right = AddButton(TEXT("▶"), FAnchors(0.16f, 0.72f, 0.27f, 0.88f), FMargin(0), false))
    {
        Right->OnPressed.AddDynamic(this, &URushGameUI::HandleRightPressed);
        Right->OnReleased.AddDynamic(this, &URushGameUI::HandleRightReleased);
    }
    if (UButton* Brake = AddButton(TEXT("فرامل"), FAnchors(0.68f, 0.75f, 0.79f, 0.90f), FMargin(0), false))
    {
        Brake->OnPressed.AddDynamic(this, &URushGameUI::HandleBrakePressed);
        Brake->OnReleased.AddDynamic(this, &URushGameUI::HandleBrakeReleased);
    }
    if (UButton* Throttle = AddButton(TEXT("تسارع"), FAnchors(0.82f, 0.72f, 0.96f, 0.91f), FMargin(0), true))
    {
        Throttle->OnPressed.AddDynamic(this, &URushGameUI::HandleThrottlePressed);
        Throttle->OnReleased.AddDynamic(this, &URushGameUI::HandleThrottleReleased);
    }
    if (UButton* Nitro = AddButton(TEXT("نيترو"), FAnchors(0.80f, 0.57f, 0.95f, 0.65f), FMargin(0), true))
    {
        Nitro->OnPressed.AddDynamic(this, &URushGameUI::HandleNitroPressed);
        Nitro->OnReleased.AddDynamic(this, &URushGameUI::HandleNitroReleased);
    }
}

void URushGameUI::ShowPauseMenu()
{
    ResetCanvas();
    AddText(TEXT("توقّف مؤقت"), FAnchors(0.30f, 0.28f, 0.70f, 0.38f), FMargin(0), 36, FLinearColor::White);
    if (UButton* Resume = AddButton(TEXT("استئناف"), FAnchors(0.36f, 0.43f, 0.64f, 0.51f), FMargin(0), true))
        Resume->OnClicked.AddDynamic(this, &URushGameUI::HandleResume);
    if (UButton* Retry = AddButton(TEXT("إعادة السباق"), FAnchors(0.36f, 0.54f, 0.64f, 0.62f), FMargin(0), false))
        Retry->OnClicked.AddDynamic(this, &URushGameUI::HandleRetry);
    if (UButton* Menu = AddButton(TEXT("القائمة الرئيسية"), FAnchors(0.36f, 0.65f, 0.64f, 0.73f), FMargin(0), false))
        Menu->OnClicked.AddDynamic(this, &URushGameUI::HandleMenu);
}

void URushGameUI::ShowResults(bool bWon, int32 Score, float DistanceMeters, int32 Collisions)
{
    ResetCanvas();
    AddText(bWon ? TEXT("انتهى السباق") : TEXT("نفد الوقود"), FAnchors(0.27f, 0.20f, 0.73f, 0.30f), FMargin(0), 36, FLinearColor(0.94f, 0.98f, 1.0f));
    AddText(FString::Printf(TEXT("النقاط %d   •   المسافة %d م"), Score, FMath::RoundToInt(DistanceMeters)), FAnchors(0.25f, 0.34f, 0.75f, 0.42f), FMargin(0), 22, FLinearColor(0.13f, 0.82f, 0.91f));
    AddText(FString::Printf(TEXT("الاصطدامات %d"), Collisions), FAnchors(0.25f, 0.43f, 0.75f, 0.49f), FMargin(0), 18, FLinearColor(0.76f, 0.82f, 0.87f));
    if (UButton* Retry = AddButton(TEXT("العب مجددًا"), FAnchors(0.36f, 0.56f, 0.64f, 0.64f), FMargin(0), true))
        Retry->OnClicked.AddDynamic(this, &URushGameUI::HandleRetry);
    if (UButton* Menu = AddButton(TEXT("القائمة الرئيسية"), FAnchors(0.36f, 0.67f, 0.64f, 0.75f), FMargin(0), false))
        Menu->OnClicked.AddDynamic(this, &URushGameUI::HandleMenu);
}

void URushGameUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (Vehicle.IsValid() && SpeedText)
    {
        SpeedText->SetText(FText::FromString(FString::Printf(TEXT("%03d كم/س"), FMath::RoundToInt(Vehicle->GetSpeedKmh()))));
        if (FuelText) FuelText->SetText(FText::FromString(FString::Printf(TEXT("الوقود %d%%"), FMath::RoundToInt(Vehicle->GetFuelPercent()))));
        if (NitroText) NitroText->SetText(FText::FromString(FString::Printf(TEXT("نيترو %d%%"), FMath::RoundToInt(Vehicle->GetNitroPercent()))));
        if (FuelBar) FuelBar->SetPercent(Vehicle->GetFuelPercent() / 100.0f);
        if (NitroBar) NitroBar->SetPercent(Vehicle->GetNitroPercent() / 100.0f);
    }
    if (TelemetryText && GetWorld())
    {
        FString Stats = Vehicle.IsValid()
            ? FString::Printf(TEXT("المسافة %d م   •   النقاط %d"), FMath::RoundToInt(Vehicle->GetDistanceMeters()), Vehicle->GetScore())
            : FString();
        if (const AVelocityRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AVelocityRushGameMode>())
        {
            if (!GameMode->IsFreeDrive() && GameMode->IsRaceActive())
            {
                Stats += FString::Printf(TEXT("   •   الوقت %d ث"), FMath::CeilToInt(GameMode->GetTimeRemaining()));
            }
        }
        TelemetryText->SetText(FText::FromString(Stats));
    }
}

void URushGameUI::HandleRace() { if (Controller.IsValid()) Controller->StartRace(false); }
void URushGameUI::HandleFreeDrive() { if (Controller.IsValid()) Controller->StartRace(true); }
void URushGameUI::HandleNextCar() { if (Controller.IsValid()) Controller->ChangeCar(1); }
void URushGameUI::HandlePreviousCar() { if (Controller.IsValid()) Controller->ChangeCar(-1); }
void URushGameUI::HandlePause() { if (Controller.IsValid()) Controller->TogglePause(); }
void URushGameUI::HandleResume() { if (Controller.IsValid()) Controller->ResumeRace(); }
void URushGameUI::HandleRetry() { if (Controller.IsValid()) Controller->RetryRace(); }
void URushGameUI::HandleMenu() { if (Controller.IsValid()) Controller->ReturnToMenu(); }
void URushGameUI::UpdateTouchState()
{
    if (!Controller.IsValid()) return;
    const float Steering = bTouchLeft == bTouchRight ? 0.0f : (bTouchLeft ? -1.0f : 1.0f);
    Controller->SetTouchSteering(Steering);
    Controller->SetTouchThrottle(bTouchThrottle ? 1.0f : 0.0f);
    Controller->SetTouchBrake(bTouchBrake ? 1.0f : 0.0f);
    Controller->SetTouchNitro(bTouchNitro);
}

void URushGameUI::ClearTouchState()
{
    bTouchLeft = false;
    bTouchRight = false;
    bTouchThrottle = false;
    bTouchBrake = false;
    bTouchNitro = false;
    UpdateTouchState();
}

void URushGameUI::HandleLeftPressed() { bTouchLeft = true; UpdateTouchState(); }
void URushGameUI::HandleRightPressed() { bTouchRight = true; UpdateTouchState(); }
void URushGameUI::HandleLeftReleased() { bTouchLeft = false; UpdateTouchState(); }
void URushGameUI::HandleRightReleased() { bTouchRight = false; UpdateTouchState(); }
void URushGameUI::HandleThrottlePressed() { bTouchThrottle = true; UpdateTouchState(); }
void URushGameUI::HandleThrottleReleased() { bTouchThrottle = false; UpdateTouchState(); }
void URushGameUI::HandleBrakePressed() { bTouchBrake = true; UpdateTouchState(); }
void URushGameUI::HandleBrakeReleased() { bTouchBrake = false; UpdateTouchState(); }
void URushGameUI::HandleNitroPressed() { bTouchNitro = true; UpdateTouchState(); }
void URushGameUI::HandleNitroReleased() { bTouchNitro = false; UpdateTouchState(); }
