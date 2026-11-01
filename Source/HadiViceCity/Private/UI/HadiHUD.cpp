// HADI: VICE CITY
#include "UI/HadiHUD.h"

#include "Characters/HadiCharacter.h"
#include "Core/HadiCity.h"
#include "Core/HadiRig.h"
#include "Game/HadiGameInstance.h"
#include "Game/HadiGameMode.h"
#include "Game/HadiGameUserSettings.h"
#include "Gameplay/HadiShop.h"
#include "Player/HadiPlayerController.h"
#include "UI/HadiMinimap.h"
#include "Vehicles/HadiVehicle.h"
#include "World/HadiCityGenerator.h"
#include "World/HadiEnvironmentManager.h"

#include "Camera/PlayerCameraManager.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/App.h"

#define LOCTEXT_NAMESPACE "HadiHUD"

namespace HadiHudStyle
{
const FLinearColor White(1.0f, 1.0f, 1.0f, 1.0f);
const FLinearColor Dim(0.62f, 0.64f, 0.7f, 1.0f);
const FLinearColor Disabled(0.38f, 0.39f, 0.42f, 1.0f);
const FLinearColor Pink(1.0f, 0.18f, 0.62f, 1.0f);
const FLinearColor Cyan(0.2f, 0.9f, 1.0f, 1.0f);
const FLinearColor Gold(1.0f, 0.78f, 0.25f, 1.0f);
const FLinearColor Money(0.35f, 0.92f, 0.45f, 1.0f);
const FLinearColor Health(0.32f, 0.85f, 0.38f, 1.0f);
const FLinearColor HealthLow(0.95f, 0.25f, 0.25f, 1.0f);
const FLinearColor Stamina(0.98f, 0.82f, 0.25f, 1.0f);
const FLinearColor Panel(0.0f, 0.0f, 0.02f, 0.62f);
const FLinearColor PanelLight(0.0f, 0.0f, 0.02f, 0.4f);

FText OnOff(bool bValue)
{
	return bValue ? LOCTEXT("On", "On") : LOCTEXT("Off", "Off");
}

FString Km(float Meters)
{
	return FString::Printf(TEXT("%.1f km"), Meters / 1000.0f);
}

FString Duration(float Seconds)
{
	const int32 Total = FMath::FloorToInt(Seconds);
	return FString::Printf(TEXT("%dh %02dm %02ds"), Total / 3600, (Total / 60) % 60, Total % 60);
}

FString MoneyText(int32 Amount)
{
	return FString::Printf(TEXT("$%s"), *FText::AsNumber(Amount).ToString());
}
} // namespace HadiHudStyle

AHadiHUD::AHadiHUD()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHadiHUD::BeginPlay()
{
	Super::BeginPlay();
	const UHadiGameInstance* GameInstance = GetGameInstance<UHadiGameInstance>();
	if (!GameInstance || !GameInstance->bSessionActive)
	{
		OpenMainMenu();
	}
}

UFont* AHadiHUD::MakeFont(int32 Size) const
{
	UFont* Base = GEngine ? GEngine->GetLargeFont() : nullptr;
	if (!Base)
	{
		return nullptr;
	}
	// Runtime (composite) fonts render crisply at any size via LegacyFontSize.
	UFont* Sized = DuplicateObject<UFont>(Base, const_cast<AHadiHUD*>(this));
	Sized->LegacyFontSize = Size;
	return Sized;
}

void AHadiHUD::EnsureResources()
{
	if (bResourcesReady)
	{
		return;
	}
	bResourcesReady = true;
	FontSmall = MakeFont(16);
	FontMedium = MakeFont(22);
	FontLarge = MakeFont(34);
	FontHuge = MakeFont(76);
	if (!FontSmall)
	{
		FontSmall = GEngine->GetSmallFont();
		FontMedium = GEngine->GetMediumFont();
		FontLarge = GEngine->GetLargeFont();
		FontHuge = GEngine->GetLargeFont();
	}
	const AHadiGameMode* GameMode = GetHadiGameMode();
	if (GameMode && GameMode->GetCity())
	{
		Minimap = NewObject<UHadiMinimap>(this);
		Minimap->Initialize(this, &GameMode->GetCity()->GetModel(), FontSmall, 512, 324);
	}
}

AHadiGameMode* AHadiHUD::GetHadiGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AHadiGameMode>() : nullptr;
}

AHadiPlayerController* AHadiHUD::GetHadiController() const
{
	return Cast<AHadiPlayerController>(PlayerOwner);
}

// ------------------------------------------------------------------------------- helpers
AHadiHUD::FTextSize AHadiHUD::Measure(const FString& Text, UFont* Font, float Scale) const
{
	FTextSize Size;
	if (Canvas && Font)
	{
		Canvas->TextSize(Font, Text, Size.X, Size.Y, Scale * UIScale, Scale * UIScale);
	}
	return Size;
}

TArray<FString> AHadiHUD::WrapText(const FString& Text, UFont* Font, float Scale, float MaxWidth) const
{
	TArray<FString> Lines;
	TArray<FString> Words;
	Text.ParseIntoArray(Words, TEXT(" "), true);
	FString Current;
	for (const FString& Word : Words)
	{
		const FString Candidate = Current.IsEmpty() ? Word : Current + TEXT(" ") + Word;
		if (!Current.IsEmpty() && Measure(Candidate, Font, Scale).X > MaxWidth)
		{
			Lines.Add(Current);
			Current = Word;
		}
		else
		{
			Current = Candidate;
		}
	}
	if (!Current.IsEmpty())
	{
		Lines.Add(Current);
	}
	return Lines;
}

void AHadiHUD::DrawLabel(const FString& Text, float X, float Y, const FLinearColor& Color, UFont* Font, float Scale, bool bCenter, bool bShadow)
{
	if (!Canvas || !Font || Text.IsEmpty())
	{
		return;
	}
	FVector2D Position(X, Y);
	if (bCenter)
	{
		Position.X -= Measure(Text, Font, Scale).X * 0.5f;
	}
	FCanvasTextItem Item(Position, FText::FromString(Text), Font, Color);
	Item.Scale = FVector2D(Scale * UIScale, Scale * UIScale);
	if (bShadow)
	{
		Item.EnableShadow(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f * Color.A), FVector2D(2.0f, 2.0f));
	}
	Canvas->DrawItem(Item);
}

void AHadiHUD::DrawPanel(float X, float Y, float W, float H, const FLinearColor& Color)
{
	if (!Canvas)
	{
		return;
	}
	FCanvasTileItem Tile(FVector2D(X, Y), FVector2D(W, H), Color);
	Tile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Tile);
}

// ---------------------------------------------------------------------------------- frame
void AHadiHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}
	EnsureResources();
	UIScale = FMath::Max(0.5f, static_cast<float>(Canvas->ClipY) / 1080.0f);
	const float DeltaSeconds = static_cast<float>(FApp::GetDeltaTime());
	HudTime += DeltaSeconds;

	const bool bMainMenuOpen = IsMenuOpen() && MenuStack[0].bMainMenu;
	if (!bMainMenuOpen && PlayerOwner && PlayerOwner->GetPawn())
	{
		DrawGameplay(DeltaSeconds);
	}
	DrawBigMessage();
	if (IsMenuOpen())
	{
		DrawMenu();
	}
	DrawFade();
	if (BigTimeLeft > 0.0f)
	{
		BigTimeLeft -= DeltaSeconds;
	}
	if (FadeTimeLeft > 0.0f)
	{
		FadeTimeLeft -= DeltaSeconds;
	}
	DrawFps(DeltaSeconds);
}

void AHadiHUD::DrawGameplay(float DeltaSeconds)
{
	DrawMinimap();
	DrawStatusBars();
	DrawTopRight();
	DrawObjective();
	DrawNotifications();
	DrawLocation();
	DrawVehicleHud();
	if (!IsMenuOpen())
	{
		DrawPrompt();
	}
	for (int32 Index = Notifications.Num() - 1; Index >= 0; --Index)
	{
		Notifications[Index].TimeLeft -= DeltaSeconds;
		if (Notifications[Index].TimeLeft <= 0.0f)
		{
			Notifications.RemoveAt(Index);
		}
	}
}

// ---------------------------------------------------------------------------- HUD widgets
void AHadiHUD::DrawMinimap()
{
	const float S = UIScale;
	const float W = 380.0f * S;
	const float H = 240.0f * S;
	const float X = 40.0f * S;
	const float Y = Canvas->ClipY - 40.0f * S - 26.0f * S - H;
	DrawPanel(X - 4.0f * S, Y - 4.0f * S, W + 8.0f * S, H + 8.0f * S, FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));

	APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	AHadiGameMode* GameMode = GetHadiGameMode();
	if (!Minimap || !Minimap->IsReady() || !Pawn || !GameMode || !GameMode->GetCity())
	{
		return;
	}
	AHadiCityGenerator* City = GameMode->GetCity();
	FHadiMinimapState State;
	State.Center = FVector2D(Pawn->GetActorLocation().X, Pawn->GetActorLocation().Y);
	State.CameraYaw = PlayerOwner->PlayerCameraManager ? static_cast<float>(PlayerOwner->PlayerCameraManager->GetCameraRotation().Yaw) : 0.0f;
	State.PlayerYaw = static_cast<float>(Pawn->GetActorRotation().Yaw);
	float Zoom = 0.022f;
	if (const AHadiVehicle* Vehicle = Cast<AHadiVehicle>(Pawn))
	{
		Zoom = FMath::Lerp(0.02f, 0.011f, FMath::Clamp(Vehicle->GetSpeedKmh() / 150.0f, 0.0f, 1.0f));
	}
	State.PixelsPerCm = Zoom;

	// GPS route to the current objective (recomputed twice a second).
	FVector Target;
	const bool bTarget = GameMode->GetObjectiveTarget(Target);
	RouteTimer -= static_cast<float>(FApp::GetDeltaTime());
	if (!bTarget)
	{
		CachedRoute.Reset();
	}
	else if (RouteTimer <= 0.0f)
	{
		RouteTimer = 0.5f;
		City->BuildRoute(Pawn->GetActorLocation(), Target, CachedRoute);
	}
	State.Route = CachedRoute;

	// Blips: shops and home are always shown; the objective clamps to the radar edge.
	for (const hadi::Poi& Poi : City->GetModel().pois)
	{
		FHadiMinimapBlip Blip;
		Blip.World = FVector2D(Poi.pos.x, Poi.pos.y);
		switch (Poi.type)
		{
		case hadi::PoiType::Restaurant:
			Blip.Color = FLinearColor(1.0f, 0.55f, 0.1f);
			Blip.Letter = TEXT("R");
			break;
		case hadi::PoiType::ClothingStore:
			Blip.Color = HadiHudStyle::Pink;
			Blip.Letter = TEXT("C");
			break;
		case hadi::PoiType::Safehouse:
			Blip.Color = HadiHudStyle::Cyan;
			Blip.Letter = TEXT("H");
			Blip.bClampToEdge = true;
			break;
		default: continue;
		}
		State.Blips.Add(Blip);
	}
	if (bTarget)
	{
		FHadiMinimapBlip Objective;
		Objective.World = FVector2D(Target.X, Target.Y);
		Objective.Color = HadiHudStyle::Gold;
		Objective.Letter = TEXT("!");
		Objective.bClampToEdge = true;
		State.Blips.Add(Objective);
	}
	Minimap->Render(State);
	DrawTexture(Minimap->GetTexture(), X, Y, W, H, 0.0f, 0.0f, 1.0f, 1.0f, FLinearColor::White, BLEND_Opaque);
}

void AHadiHUD::DrawStatusBars()
{
	const float S = UIScale;
	const float W = 380.0f * S;
	const float X = 40.0f * S;
	const float Y = Canvas->ClipY - 40.0f * S - 14.0f * S;
	const float BarH = 10.0f * S;
	const float Half = W * 0.5f - 3.0f * S;

	float HealthFraction = 1.0f;
	float StaminaFraction = 1.0f;
	if (const AHadiCharacter* Hadi = GetHadiController() ? GetHadiController()->GetHadi() : nullptr)
	{
		HealthFraction = Hadi->GetHealth() / Hadi->GetMaxHealth();
		StaminaFraction = Hadi->GetStamina() / Hadi->GetMaxStamina();
	}
	else if (const AHadiVehicle* Vehicle = GetHadiController() ? GetHadiController()->GetVehicle() : nullptr)
	{
		if (const AHadiCharacter* Driver = Vehicle->GetDriver())
		{
			HealthFraction = Driver->GetHealth() / Driver->GetMaxHealth();
			StaminaFraction = Driver->GetStamina() / Driver->GetMaxStamina();
		}
	}
	const bool bLow = HealthFraction < 0.3f;
	const float Blink = bLow ? 0.6f + 0.4f * FMath::Abs(FMath::Sin(HudTime * 5.0f)) : 1.0f;
	DrawPanel(X, Y, Half, BarH, FLinearColor(0.0f, 0.0f, 0.0f, 0.75f));
	DrawPanel(X, Y, Half * FMath::Clamp(HealthFraction, 0.0f, 1.0f), BarH, (bLow ? HadiHudStyle::HealthLow : HadiHudStyle::Health) * Blink);
	DrawPanel(X + Half + 6.0f * S, Y, Half, BarH, FLinearColor(0.0f, 0.0f, 0.0f, 0.75f));
	DrawPanel(X + Half + 6.0f * S, Y, Half * FMath::Clamp(StaminaFraction, 0.0f, 1.0f), BarH, HadiHudStyle::Stamina);
}

void AHadiHUD::DrawTopRight()
{
	const float S = UIScale;
	const AHadiGameMode* GameMode = GetHadiGameMode();
	if (!GameMode)
	{
		return;
	}
	const float Right = Canvas->ClipX - 40.0f * S;
	if (const AHadiEnvironmentManager* Environment = GameMode->GetEnvironment())
	{
		const FString Clock = Environment->GetClockText();
		DrawLabel(Clock, Right - Measure(Clock, FontMedium).X, 34.0f * S, HadiHudStyle::White, FontMedium);
	}
	const FString Money = HadiHudStyle::MoneyText(GameMode->GetMoney());
	DrawLabel(Money, Right - Measure(Money, FontLarge).X, 64.0f * S, HadiHudStyle::Money, FontLarge);
	const FString Flamingos = FString::Printf(TEXT("Neon Flamingos  %d/%d"), GameMode->GetFlamingoCount(), GameMode->GetFlamingoTotal());
	DrawLabel(Flamingos, Right - Measure(Flamingos, FontSmall).X, 112.0f * S, HadiHudStyle::Pink, FontSmall);
}

void AHadiHUD::DrawObjective()
{
	const AHadiGameMode* GameMode = GetHadiGameMode();
	if (!GameMode)
	{
		return;
	}
	const float S = UIScale;
	const float X = 40.0f * S;
	const float Y = 34.0f * S;
	const float W = 560.0f * S;
	const TArray<FString> Lines = WrapText(GameMode->GetObjectiveText().ToString(), FontMedium, 1.0f, W - 36.0f * S);
	const float LineH = Measure(TEXT("Ag"), FontMedium).Y;
	const float H = 46.0f * S + LineH * static_cast<float>(Lines.Num()) + 12.0f * S;
	DrawPanel(X, Y, W, H, HadiHudStyle::Panel);
	DrawPanel(X, Y, 5.0f * S, H, GameMode->HasActiveObjective() ? HadiHudStyle::Gold : HadiHudStyle::Cyan);
	DrawLabel(GameMode->GetObjectiveTitle().ToString().ToUpper(), X + 18.0f * S, Y + 10.0f * S, GameMode->HasActiveObjective() ? HadiHudStyle::Gold : HadiHudStyle::Cyan,
		FontSmall);
	float LineY = Y + 40.0f * S;
	for (const FString& Line : Lines)
	{
		DrawLabel(Line, X + 18.0f * S, LineY, HadiHudStyle::White, FontMedium);
		LineY += LineH;
	}
}

void AHadiHUD::DrawNotifications()
{
	const float S = UIScale;
	float Y = 230.0f * S;
	const int32 First = FMath::Max(0, Notifications.Num() - 4);
	for (int32 Index = First; Index < Notifications.Num(); ++Index)
	{
		const FNotification& Note = Notifications[Index];
		const float Alpha = FMath::Clamp(Note.TimeLeft / 0.5f, 0.0f, 1.0f);
		const FString Text = Note.Text.ToString();
		const FTextSize Size = Measure(Text, FontMedium);
		DrawPanel(40.0f * S, Y, Size.X + 36.0f * S, Size.Y + 16.0f * S, FLinearColor(0.0f, 0.0f, 0.02f, 0.62f * Alpha));
		DrawPanel(40.0f * S, Y, 5.0f * S, Size.Y + 16.0f * S, FLinearColor(1.0f, 0.18f, 0.62f, Alpha));
		DrawLabel(Text, 58.0f * S, Y + 8.0f * S, FLinearColor(1.0f, 1.0f, 1.0f, Alpha), FontMedium);
		Y += Size.Y + 24.0f * S;
	}
}

void AHadiHUD::DrawPrompt()
{
	const AHadiPlayerController* Controller = GetHadiController();
	const AHadiCharacter* Hadi = Controller ? Controller->GetHadi() : nullptr;
	if (!Hadi || !Hadi->GetFocusedInteractable())
	{
		return;
	}
	const FString Text = Hadi->GetInteractionPromptText().ToString();
	if (Text.IsEmpty())
	{
		return;
	}
	const float S = UIScale;
	const FTextSize Size = Measure(Text, FontMedium);
	const float KeyW = 40.0f * S;
	const float TotalW = KeyW + 14.0f * S + Size.X;
	const float X = (Canvas->ClipX - TotalW) * 0.5f;
	const float Y = Canvas->ClipY * 0.72f;
	DrawPanel(X - 16.0f * S, Y - 10.0f * S, TotalW + 32.0f * S, Size.Y + 20.0f * S, HadiHudStyle::Panel);
	DrawPanel(X, Y - 4.0f * S, KeyW, Size.Y + 8.0f * S, FLinearColor(1.0f, 1.0f, 1.0f, 0.92f));
	DrawLabel(TEXT("E"), X + KeyW * 0.5f, Y, FLinearColor::Black, FontMedium, 1.0f, true, false);
	DrawLabel(Text, X + KeyW + 14.0f * S, Y, HadiHudStyle::White, FontMedium);
}

void AHadiHUD::DrawLocation()
{
	const AHadiGameMode* GameMode = GetHadiGameMode();
	const APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	if (!GameMode || !GameMode->GetCity() || !Pawn)
	{
		return;
	}
	const float S = UIScale;
	const FString Name = GameMode->GetCity()->GetLocationName(Pawn->GetActorLocation());
	const float Right = Canvas->ClipX - 40.0f * S;
	DrawLabel(Name, Right - Measure(Name, FontMedium).X, Canvas->ClipY - 70.0f * S, HadiHudStyle::White, FontMedium);
}

void AHadiHUD::DrawVehicleHud()
{
	const AHadiPlayerController* Controller = GetHadiController();
	const AHadiVehicle* Vehicle = Controller ? Controller->GetVehicle() : nullptr;
	if (!Vehicle)
	{
		return;
	}
	const float S = UIScale;
	const float Right = Canvas->ClipX - 40.0f * S;
	const float Bottom = Canvas->ClipY - 100.0f * S;
	const FString Speed = FString::Printf(TEXT("%d"), FMath::RoundToInt(Vehicle->GetSpeedKmh()));
	const FTextSize SpeedSize = Measure(Speed, FontHuge, 0.9f);
	DrawLabel(Speed, Right - 70.0f * S - SpeedSize.X, Bottom - SpeedSize.Y, HadiHudStyle::White, FontHuge, 0.9f);
	DrawLabel(TEXT("KM/H"), Right - 64.0f * S, Bottom - 44.0f * S, HadiHudStyle::Dim, FontSmall);
	const int32 Gear = Vehicle->GetGear();
	DrawLabel(Gear < 0 ? FString(TEXT("R")) : FString::FromInt(Gear), Right - 64.0f * S, Bottom - SpeedSize.Y + 4.0f * S, HadiHudStyle::Cyan, FontLarge);
	// RPM bar.
	const float BarW = 260.0f * S;
	const float Rpm = Vehicle->GetRpmFraction();
	DrawPanel(Right - BarW, Bottom + 2.0f * S, BarW, 6.0f * S, FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
	DrawPanel(Right - BarW, Bottom + 2.0f * S, BarW * Rpm, 6.0f * S, Rpm > 0.88f ? HadiHudStyle::HealthLow : HadiHudStyle::Cyan);
	const FString Name = Vehicle->DisplayName.ToString();
	DrawLabel(Name, Right - Measure(Name, FontSmall).X, Bottom - SpeedSize.Y - 26.0f * S, HadiHudStyle::Dim, FontSmall);
	const FString Hint = TEXT("[E] Exit   [V] Camera   [Space] Handbrake");
	DrawLabel(Hint, Canvas->ClipX * 0.5f, Canvas->ClipY - 46.0f * S, HadiHudStyle::Dim, FontSmall, 1.0f, true);
}

void AHadiHUD::DrawBigMessage()
{
	if (BigTimeLeft <= 0.0f || BigTitle.IsEmpty())
	{
		return;
	}
	const float S = UIScale;
	const float Elapsed = BigDuration - BigTimeLeft;
	const float Alpha = FMath::Clamp(FMath::Min(Elapsed / 0.4f, BigTimeLeft / 0.7f), 0.0f, 1.0f);
	const float CenterX = Canvas->ClipX * 0.5f;
	const float Y = Canvas->ClipY * 0.30f;
	const TArray<FString> Lines = WrapText(BigSubtitle.ToString(), FontMedium, 1.0f, 1100.0f * S);
	const float LineH = Measure(TEXT("Ag"), FontMedium).Y;
	const float BandH = 150.0f * S + LineH * static_cast<float>(Lines.Num());
	DrawPanel(0.0f, Y - 30.0f * S, Canvas->ClipX, BandH, FLinearColor(0.0f, 0.0f, 0.0f, 0.55f * Alpha));
	DrawLabel(BigTitle.ToString(), CenterX, Y, FLinearColor(HadiHudStyle::Gold.R, HadiHudStyle::Gold.G, HadiHudStyle::Gold.B, Alpha), FontHuge, 0.9f, true);
	float LineY = Y + 100.0f * S;
	for (const FString& Line : Lines)
	{
		DrawLabel(Line, CenterX, LineY, FLinearColor(1.0f, 1.0f, 1.0f, Alpha), FontMedium, 1.0f, true);
		LineY += LineH;
	}
}

void AHadiHUD::DrawFade()
{
	if (FadeTimeLeft <= 0.0f)
	{
		return;
	}
	const float T = 1.0f - FadeTimeLeft / FadeDuration;
	const float Alpha = FMath::Sin(T * UE_PI);
	DrawPanel(0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY, FLinearColor(0.0f, 0.0f, 0.0f, Alpha));
}

void AHadiHUD::DrawFps(float DeltaSeconds)
{
	const UHadiGameUserSettings* Settings = UHadiGameUserSettings::GetHadiSettings();
	if (!Settings || !Settings->bShowFPS || DeltaSeconds <= 0.0f)
	{
		return;
	}
	SmoothedFps = FMath::Lerp(SmoothedFps, 1.0f / DeltaSeconds, 0.05f);
	DrawLabel(FString::Printf(TEXT("%d FPS  %.1f ms"), FMath::RoundToInt(SmoothedFps), 1000.0f / FMath::Max(1.0f, SmoothedFps)), 8.0f * UIScale, 4.0f * UIScale,
		HadiHudStyle::Money, FontSmall);
}

// ------------------------------------------------------------------------------- messages
void AHadiHUD::PushNotification(const FText& Message)
{
	FNotification Note;
	Note.Text = Message;
	Note.TimeLeft = 4.5f;
	Notifications.Add(Note);
}

void AHadiHUD::ShowBigMessage(const FText& Title, const FText& Subtitle, float Duration)
{
	BigTitle = Title;
	BigSubtitle = Subtitle;
	BigDuration = FMath::Max(1.0f, Duration);
	BigTimeLeft = BigDuration;
}

void AHadiHUD::FadeOutIn(float Duration)
{
	FadeDuration = FMath::Max(0.2f, Duration);
	FadeTimeLeft = FadeDuration;
}

// ---------------------------------------------------------------------------------- menus
AHadiHUD::FMenuItem AHadiHUD::MakeItem(const FText& Label, TFunction<void()> OnActivate)
{
	FMenuItem Item;
	Item.Label = Label;
	Item.OnActivate = MoveTemp(OnActivate);
	return Item;
}

AHadiHUD::FMenuItem AHadiHUD::MakeInfo(const FText& Label, const FText& Value)
{
	FMenuItem Item;
	Item.Label = Label;
	Item.Value = [Value]() { return Value; };
	Item.bSelectable = false;
	return Item;
}

bool AHadiHUD::IsItemEnabled(const FMenuItem& Item) const
{
	return Item.bSelectable && (!Item.IsEnabled || Item.IsEnabled());
}

void AHadiHUD::FixSelection(FMenuPage& Page, int32 Direction)
{
	const int32 Count = Page.Items.Num();
	if (Count == 0)
	{
		return;
	}
	const int32 Step = Direction >= 0 ? 1 : -1;
	for (int32 Attempt = 0; Attempt < Count; ++Attempt)
	{
		Page.Selected = (Page.Selected % Count + Count) % Count;
		if (IsItemEnabled(Page.Items[Page.Selected]))
		{
			return;
		}
		Page.Selected += Step;
	}
}

void AHadiHUD::PushPage(FMenuPage&& Page)
{
	FixSelection(Page, 1);
	MenuStack.Add(MoveTemp(Page));
	ItemRects.Reset();
}

void AHadiHUD::PopPage()
{
	if (MenuStack.Num() == 0)
	{
		return;
	}
	TFunction<void()> OnClose = MenuStack.Last().OnClose;
	MenuStack.Pop();
	ItemRects.Reset();
	if (OnClose)
	{
		OnClose();
	}
	if (MenuStack.Num() == 0)
	{
		ActiveShop = nullptr;
		if (AHadiPlayerController* Controller = GetHadiController())
		{
			Controller->ClosePauseMenu();
		}
	}
}

void AHadiHUD::CloseAllMenus()
{
	MenuStack.Reset();
	ItemRects.Reset();
	ActiveShop = nullptr;
}

void AHadiHUD::OpenMainMenu()
{
	CloseAllMenus();
	PushPage(BuildMainMenu());
}

void AHadiHUD::OpenPauseMenu()
{
	if (!IsMenuOpen())
	{
		PushPage(BuildPauseMenu());
	}
}

void AHadiHUD::OpenShopMenu(AHadiShop* Shop)
{
	if (!IsMenuOpen() && Shop)
	{
		ActiveShop = Shop;
		PushPage(Shop->GetShopType() == EHadiShopType::Restaurant ? BuildShopPage(Shop) : BuildWardrobePage(true));
	}
}

void AHadiHUD::OpenSafehouseMenu()
{
	if (!IsMenuOpen())
	{
		PushPage(BuildSafehousePage());
	}
}

void AHadiHUD::MenuNavigate(int32 Direction)
{
	if (!IsMenuOpen())
	{
		return;
	}
	FMenuPage& Page = MenuStack.Last();
	Page.Selected += Direction;
	FixSelection(Page, Direction);
}

void AHadiHUD::MenuAdjust(int32 Direction)
{
	if (!IsMenuOpen())
	{
		return;
	}
	FMenuPage& Page = MenuStack.Last();
	if (Page.Items.IsValidIndex(Page.Selected))
	{
		const FMenuItem& Item = Page.Items[Page.Selected];
		if (Item.OnAdjust && IsItemEnabled(Item))
		{
			Item.OnAdjust(Direction);
		}
	}
}

void AHadiHUD::MenuConfirm()
{
	if (!IsMenuOpen())
	{
		return;
	}
	FMenuPage& Page = MenuStack.Last();
	if (!Page.Items.IsValidIndex(Page.Selected))
	{
		return;
	}
	// Copy: the callback may push/pop pages and invalidate references.
	const FMenuItem Item = Page.Items[Page.Selected];
	if (!IsItemEnabled(Item))
	{
		return;
	}
	if (Item.OnActivate)
	{
		Item.OnActivate();
	}
	else if (Item.OnAdjust)
	{
		Item.OnAdjust(1);
	}
}

void AHadiHUD::MenuBack()
{
	if (!IsMenuOpen() || MenuStack.Last().bMainMenu)
	{
		return;
	}
	PopPage();
}

void AHadiHUD::MenuClick(const FVector2D& ScreenPosition)
{
	if (!IsMenuOpen())
	{
		return;
	}
	for (int32 Index = 0; Index < ItemRects.Num(); ++Index)
	{
		if (ItemRects[Index].IsInside(ScreenPosition) && MenuStack.Last().Items.IsValidIndex(Index) && IsItemEnabled(MenuStack.Last().Items[Index]))
		{
			MenuStack.Last().Selected = Index;
			MenuConfirm();
			return;
		}
	}
}

// --------------------------------------------------------------------------- page builders
AHadiHUD::FMenuPage AHadiHUD::BuildMainMenu()
{
	FMenuPage Page;
	Page.bMainMenu = true;
	Page.Title = LOCTEXT("GameTitle", "HADI");
	TWeakObjectPtr<AHadiHUD> Self(this);

	FMenuItem Continue = MakeItem(LOCTEXT("Continue", "Continue"), [Self]() {
		if (Self.IsValid() && Self->GetHadiGameMode() && Self->GetHadiGameMode()->ContinueGame(Self->PlayerOwner))
		{
			return;
		}
		if (Self.IsValid())
		{
			Self->PushNotification(LOCTEXT("LoadFailed", "Could not load the save game."));
		}
	});
	Continue.IsEnabled = [Self]() { return Self.IsValid() && Self->GetHadiGameMode() && Self->GetHadiGameMode()->HasSaveGame(); };
	Continue.Value = [Self]() {
		const AHadiGameMode* GameMode = Self.IsValid() ? Self->GetHadiGameMode() : nullptr;
		return GameMode && GameMode->HasSaveGame() ? FText::FromString(GameMode->GetSaveDescription()) : LOCTEXT("NoSave", "No save yet");
	};
	Page.Items.Add(Continue);

	Page.Items.Add(MakeItem(LOCTEXT("NewGame", "New Game"), [Self]() {
		if (!Self.IsValid() || !Self->GetHadiGameMode())
		{
			return;
		}
		if (Self->GetHadiGameMode()->HasSaveGame())
		{
			Self->PushPage(Self->BuildConfirmNewGamePage());
		}
		else
		{
			Self->GetHadiGameMode()->StartNewGame(Self->PlayerOwner);
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("Settings", "Settings"), [Self]() {
		if (Self.IsValid())
		{
			Self->PushPage(Self->BuildSettingsMenu());
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("Controls", "Controls"), [Self]() {
		if (Self.IsValid())
		{
			Self->PushPage(Self->BuildControlsPage());
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("Quit", "Quit to Desktop"), [Self]() {
		if (Self.IsValid() && Self->GetHadiController())
		{
			Self->GetHadiController()->QuitToDesktop();
		}
	}));
	return Page;
}

AHadiHUD::FMenuPage AHadiHUD::BuildConfirmNewGamePage()
{
	FMenuPage Page;
	Page.Title = LOCTEXT("NewGameTitle", "Start a new game?");
	Page.Subtitle = []() { return LOCTEXT("NewGameWarn", "Your current save will be replaced the next time the game saves."); };
	TWeakObjectPtr<AHadiHUD> Self(this);
	Page.Items.Add(MakeItem(LOCTEXT("StartNew", "Start New Game"), [Self]() {
		if (Self.IsValid() && Self->GetHadiGameMode())
		{
			Self->GetHadiGameMode()->StartNewGame(Self->PlayerOwner);
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("Cancel", "Cancel"), [Self]() {
		if (Self.IsValid())
		{
			Self->PopPage();
		}
	}));
	Page.Selected = 1;
	return Page;
}

AHadiHUD::FMenuPage AHadiHUD::BuildPauseMenu()
{
	FMenuPage Page;
	Page.Title = LOCTEXT("Paused", "Paused");
	Page.bPausesGame = true;
	TWeakObjectPtr<AHadiHUD> Self(this);
	Page.Subtitle = [Self]() {
		const AHadiGameMode* GameMode = Self.IsValid() ? Self->GetHadiGameMode() : nullptr;
		return GameMode ? GameMode->GetObjectiveText() : FText::GetEmpty();
	};
	Page.Items.Add(MakeItem(LOCTEXT("Resume", "Resume"), [Self]() {
		if (Self.IsValid())
		{
			Self->PopPage();
		}
	}));
	FMenuItem Save = MakeItem(LOCTEXT("SaveGame", "Save Game"), [Self]() {
		if (Self.IsValid() && Self->GetHadiGameMode())
		{
			Self->GetHadiGameMode()->SaveGame();
		}
	});
	Save.IsEnabled = [Self]() {
		const AHadiPlayerController* Controller = Self.IsValid() ? Self->GetHadiController() : nullptr;
		const AHadiCharacter* Hadi = Controller ? Controller->GetHadi() : nullptr;
		return Controller && (!Hadi || (!Hadi->IsSwimming() && !Hadi->IsDead()));
	};
	Page.Items.Add(Save);
	Page.Items.Add(MakeItem(LOCTEXT("Stats", "Statistics"), [Self]() {
		if (Self.IsValid())
		{
			Self->PushPage(Self->BuildStatsPage());
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("Settings", "Settings"), [Self]() {
		if (Self.IsValid())
		{
			Self->PushPage(Self->BuildSettingsMenu());
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("Controls", "Controls"), [Self]() {
		if (Self.IsValid())
		{
			Self->PushPage(Self->BuildControlsPage());
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("MainMenu", "Quit to Main Menu"), [Self]() {
		if (Self.IsValid() && Self->GetHadiGameMode())
		{
			if (APlayerController* Controller = Self->PlayerOwner)
			{
				Controller->SetPause(false);
			}
			Self->GetHadiGameMode()->ReturnToMainMenu();
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("Quit", "Quit to Desktop"), [Self]() {
		if (Self.IsValid() && Self->GetHadiController())
		{
			Self->GetHadiController()->QuitToDesktop();
		}
	}));
	return Page;
}

AHadiHUD::FMenuPage AHadiHUD::BuildSettingsMenu()
{
	FMenuPage Page;
	Page.Title = LOCTEXT("SettingsTitle", "Settings");
	Page.Subtitle = []() { return LOCTEXT("SettingsSub", "Changes apply immediately and are saved when you leave this page."); };
	Page.OnClose = []() {
		if (UHadiGameUserSettings* Settings = UHadiGameUserSettings::GetHadiSettings())
		{
			Settings->SaveSettings();
		}
	};
	auto Settings = []() { return UHadiGameUserSettings::GetHadiSettings(); };

	// Graphics quality preset.
	FMenuItem Quality;
	Quality.Label = LOCTEXT("Quality", "Graphics Quality");
	Quality.Value = [Settings]() {
		static const FText Names[5] = {LOCTEXT("Low", "Low"), LOCTEXT("Medium", "Medium"), LOCTEXT("High", "High"), LOCTEXT("Epic", "Epic"), LOCTEXT("Cinematic", "Cinematic")};
		const int32 Level = Settings() ? Settings()->GetOverallScalabilityLevel() : -1;
		return Level >= 0 && Level < 5 ? Names[Level] : LOCTEXT("Custom", "Custom");
	};
	Quality.OnAdjust = [Settings](int32 Direction) {
		if (UHadiGameUserSettings* S = Settings())
		{
			const int32 Current = FMath::Max(0, S->GetOverallScalabilityLevel());
			S->SetOverallScalabilityLevel(FMath::Clamp(Current + Direction, 0, 4));
			S->ApplyNonResolutionSettings();
		}
	};
	Page.Items.Add(Quality);

	Page.Items.Add(MakeItem(LOCTEXT("AutoDetect", "Auto-Detect Quality"), [Settings]() {
		if (UHadiGameUserSettings* S = Settings())
		{
			S->RunHardwareBenchmark();
			S->ApplyHardwareBenchmarkResults();
		}
	}));

	FMenuItem Window;
	Window.Label = LOCTEXT("WindowMode", "Window Mode");
	Window.Value = [Settings]() {
		const EWindowMode::Type Mode = Settings() ? Settings()->GetFullscreenMode() : EWindowMode::Windowed;
		return Mode == EWindowMode::Fullscreen ? LOCTEXT("Fullscreen", "Fullscreen")
											   : (Mode == EWindowMode::WindowedFullscreen ? LOCTEXT("Borderless", "Borderless") : LOCTEXT("Windowed", "Windowed"));
	};
	Window.OnAdjust = [Settings](int32 Direction) {
		if (UHadiGameUserSettings* S = Settings())
		{
			const int32 Current = static_cast<int32>(S->GetFullscreenMode());
			S->SetFullscreenMode(EWindowMode::ConvertIntToWindowMode((Current + Direction + 3) % 3));
			S->ApplyResolutionSettings(false);
		}
	};
	Page.Items.Add(Window);

	FMenuItem Resolution;
	Resolution.Label = LOCTEXT("Resolution", "Resolution");
	Resolution.Value = [Settings]() {
		const FIntPoint R = Settings() ? Settings()->GetScreenResolution() : FIntPoint(0, 0);
		return FText::FromString(FString::Printf(TEXT("%d x %d"), R.X, R.Y));
	};
	Resolution.OnAdjust = [Settings](int32 Direction) {
		UHadiGameUserSettings* S = Settings();
		TArray<FIntPoint> Options;
		if (!S || !UKismetSystemLibrary::GetSupportedFullscreenResolutions(Options) || Options.Num() == 0)
		{
			return;
		}
		const int32 Current = Options.IndexOfByKey(S->GetScreenResolution());
		const int32 Next = Current == INDEX_NONE ? Options.Num() - 1 : (Current + Direction + Options.Num()) % Options.Num();
		S->SetScreenResolution(Options[Next]);
		S->ApplyResolutionSettings(false);
	};
	Page.Items.Add(Resolution);

	FMenuItem VSync;
	VSync.Label = LOCTEXT("VSync", "V-Sync");
	VSync.Value = [Settings]() { return HadiHudStyle::OnOff(Settings() && Settings()->IsVSyncEnabled()); };
	VSync.OnAdjust = [Settings](int32) {
		if (UHadiGameUserSettings* S = Settings())
		{
			S->SetVSyncEnabled(!S->IsVSyncEnabled());
			S->ApplyNonResolutionSettings();
		}
	};
	Page.Items.Add(VSync);

	FMenuItem FrameLimit;
	FrameLimit.Label = LOCTEXT("FrameLimit", "Frame Rate Limit");
	FrameLimit.Value = [Settings]() {
		const float Limit = Settings() ? Settings()->GetFrameRateLimit() : 0.0f;
		return Limit <= 0.0f ? LOCTEXT("Unlimited", "Unlimited") : FText::AsNumber(FMath::RoundToInt(Limit));
	};
	FrameLimit.OnAdjust = [Settings](int32 Direction) {
		static const float Limits[5] = {30.0f, 60.0f, 120.0f, 144.0f, 0.0f};
		if (UHadiGameUserSettings* S = Settings())
		{
			int32 Current = 4;
			for (int32 Index = 0; Index < 5; ++Index)
			{
				if (FMath::IsNearlyEqual(Limits[Index], S->GetFrameRateLimit()))
				{
					Current = Index;
				}
			}
			S->SetFrameRateLimit(Limits[(Current + Direction + 5) % 5]);
			S->ApplyNonResolutionSettings();
		}
	};
	Page.Items.Add(FrameLimit);

	auto Toggle = [Settings](const FText& Label, bool UHadiGameUserSettings::*Member) {
		FMenuItem Item;
		Item.Label = Label;
		Item.Value = [Settings, Member]() { return HadiHudStyle::OnOff(Settings() && Settings()->*Member); };
		Item.OnAdjust = [Settings, Member](int32) {
			if (UHadiGameUserSettings* S = Settings())
			{
				S->*Member = !(S->*Member);
				S->ApplyHadiSettings();
			}
		};
		return Item;
	};
	Page.Items.Add(Toggle(LOCTEXT("MotionBlur", "Motion Blur"), &UHadiGameUserSettings::bMotionBlur));
	Page.Items.Add(Toggle(LOCTEXT("DOF", "Depth of Field"), &UHadiGameUserSettings::bDepthOfField));
	Page.Items.Add(Toggle(LOCTEXT("RayTracing", "Ray Tracing (Lumen HWRT)"), &UHadiGameUserSettings::bHardwareRayTracing));

	FMenuItem Fov;
	Fov.Label = LOCTEXT("FOV", "Field of View");
	Fov.Value = [Settings]() { return FText::AsNumber(FMath::RoundToInt(Settings() ? Settings()->FieldOfView : 80.0f)); };
	Fov.OnAdjust = [Settings](int32 Direction) {
		if (UHadiGameUserSettings* S = Settings())
		{
			S->FieldOfView = FMath::Clamp(S->FieldOfView + 5.0f * static_cast<float>(Direction), 60.0f, 110.0f);
		}
	};
	Page.Items.Add(Fov);

	FMenuItem Sensitivity;
	Sensitivity.Label = LOCTEXT("Sensitivity", "Look Sensitivity");
	Sensitivity.Value = [Settings]() { return FText::FromString(FString::Printf(TEXT("%.1f"), Settings() ? Settings()->MouseSensitivity : 1.0f)); };
	Sensitivity.OnAdjust = [Settings](int32 Direction) {
		if (UHadiGameUserSettings* S = Settings())
		{
			S->MouseSensitivity = FMath::Clamp(S->MouseSensitivity + 0.1f * static_cast<float>(Direction), 0.2f, 3.0f);
		}
	};
	Page.Items.Add(Sensitivity);
	Page.Items.Add(Toggle(LOCTEXT("Invert", "Invert Look"), &UHadiGameUserSettings::bInvertLook));
	Page.Items.Add(Toggle(LOCTEXT("ShowFPS", "Show FPS"), &UHadiGameUserSettings::bShowFPS));

	TWeakObjectPtr<AHadiHUD> Self(this);
	Page.Items.Add(MakeItem(LOCTEXT("Back", "Back"), [Self]() {
		if (Self.IsValid())
		{
			Self->PopPage();
		}
	}));
	return Page;
}

AHadiHUD::FMenuPage AHadiHUD::BuildStatsPage()
{
	FMenuPage Page;
	Page.Title = LOCTEXT("StatsTitle", "Statistics");
	const UHadiGameInstance* GameInstance = GetGameInstance<UHadiGameInstance>();
	const AHadiGameMode* GameMode = GetHadiGameMode();
	if (GameInstance)
	{
		const FHadiStats& Stats = GameInstance->Profile.Stats;
		auto Num = [](int32 Value) { return FText::AsNumber(Value); };
		Page.Items.Add(MakeInfo(LOCTEXT("PlayTime", "Time played"), FText::FromString(HadiHudStyle::Duration(Stats.PlayTimeSeconds))));
		Page.Items.Add(MakeInfo(LOCTEXT("OnFoot", "Distance on foot"), FText::FromString(HadiHudStyle::Km(Stats.DistanceOnFootMeters))));
		Page.Items.Add(MakeInfo(LOCTEXT("Driven", "Distance driven"), FText::FromString(HadiHudStyle::Km(Stats.DistanceDrivenMeters))));
		Page.Items.Add(MakeInfo(LOCTEXT("Swum", "Distance swum"), FText::FromString(HadiHudStyle::Km(Stats.DistanceSwumMeters))));
		Page.Items.Add(MakeInfo(LOCTEXT("TopSpeed", "Top speed"), FText::FromString(FString::Printf(TEXT("%d km/h"), FMath::RoundToInt(Stats.TopSpeedKmh)))));
		Page.Items.Add(MakeInfo(LOCTEXT("Earned", "Money earned"), FText::FromString(HadiHudStyle::MoneyText(Stats.MoneyEarned))));
		Page.Items.Add(MakeInfo(LOCTEXT("Spent", "Money spent"), FText::FromString(HadiHudStyle::MoneyText(Stats.MoneySpent))));
		Page.Items.Add(MakeInfo(LOCTEXT("Meals", "Meals eaten"), Num(Stats.MealsEaten)));
		Page.Items.Add(MakeInfo(LOCTEXT("Outfits", "Outfits bought"), Num(Stats.OutfitsBought)));
		Page.Items.Add(MakeInfo(LOCTEXT("Ledges", "Ledges climbed"), Num(Stats.LedgesClimbed)));
		Page.Items.Add(MakeInfo(LOCTEXT("Vehicles", "Times behind the wheel"), Num(Stats.VehiclesEntered)));
		Page.Items.Add(MakeInfo(LOCTEXT("Blackouts", "Times blacked out"), Num(Stats.TimesBlackedOut)));
		if (GameMode)
		{
			Page.Items.Add(MakeInfo(LOCTEXT("Flamingos", "Neon Flamingos"),
				FText::FromString(FString::Printf(TEXT("%d / %d"), GameMode->GetFlamingoCount(), GameMode->GetFlamingoTotal()))));
		}
	}
	TWeakObjectPtr<AHadiHUD> Self(this);
	Page.Items.Add(MakeItem(LOCTEXT("Back", "Back"), [Self]() {
		if (Self.IsValid())
		{
			Self->PopPage();
		}
	}));
	return Page;
}

AHadiHUD::FMenuPage AHadiHUD::BuildControlsPage()
{
	FMenuPage Page;
	Page.Title = LOCTEXT("ControlsTitle", "Controls");
	Page.Subtitle = []() { return LOCTEXT("ControlsSub", "Keyboard & mouse  /  Gamepad"); };
	auto Row = [&Page](const FText& Action, const TCHAR* Keys) { Page.Items.Add(MakeInfo(Action, FText::FromString(Keys))); };
	Row(LOCTEXT("CMove", "Move / steer"), TEXT("W A S D  /  Left stick"));
	Row(LOCTEXT("CLook", "Camera"), TEXT("Mouse  /  Right stick"));
	Row(LOCTEXT("CSprint", "Sprint (hold)"), TEXT("Left Shift  /  X"));
	Row(LOCTEXT("CWalk", "Walk toggle"), TEXT("Left Ctrl  /  L3"));
	Row(LOCTEXT("CJump", "Jump / climb / handbrake"), TEXT("Space  /  A"));
	Row(LOCTEXT("CInteract", "Interact / enter / exit"), TEXT("E or F  /  Y"));
	Row(LOCTEXT("CDrive", "Accelerate / brake & reverse"), TEXT("W, S  /  RT, LT"));
	Row(LOCTEXT("CCam", "Vehicle camera"), TEXT("V  /  View"));
	Row(LOCTEXT("CZoom", "Camera distance"), TEXT("Mouse wheel"));
	Row(LOCTEXT("CPause", "Pause"), TEXT("Esc or P  /  Menu"));
	Row(LOCTEXT("CBack", "Menu back"), TEXT("Backspace / Esc  /  B"));
	TWeakObjectPtr<AHadiHUD> Self(this);
	Page.Items.Add(MakeItem(LOCTEXT("Back", "Back"), [Self]() {
		if (Self.IsValid())
		{
			Self->PopPage();
		}
	}));
	return Page;
}

AHadiHUD::FMenuPage AHadiHUD::BuildShopPage(AHadiShop* Shop)
{
	FMenuPage Page;
	Page.Title = Shop->GetDisplayName();
	TWeakObjectPtr<AHadiHUD> Self(this);
	TWeakObjectPtr<AHadiShop> WeakShop(Shop);
	Page.Subtitle = [Self]() {
		const AHadiGameMode* GameMode = Self.IsValid() ? Self->GetHadiGameMode() : nullptr;
		return FText::Format(LOCTEXT("Cash", "Cash: {0}   -   food restores health"), FText::FromString(HadiHudStyle::MoneyText(GameMode ? GameMode->GetMoney() : 0)));
	};
	const TArray<FHadiFoodItem>& Menu = Shop->GetMenu();
	for (int32 Index = 0; Index < Menu.Num(); ++Index)
	{
		FMenuItem Item = MakeItem(Menu[Index].Name, [Self, WeakShop, Index]() {
			AHadiPlayerController* Controller = Self.IsValid() ? Self->GetHadiController() : nullptr;
			if (!WeakShop.IsValid() || !Controller)
			{
				return;
			}
			FText Message;
			WeakShop->BuyFood(Controller->GetHadi(), Index, Message);
			Self->PushNotification(Message);
		});
		const FHadiFoodItem Food = Menu[Index];
		Item.Value = [Food]() {
			return FText::FromString(FString::Printf(TEXT("%s   +%d HP"), *HadiHudStyle::MoneyText(Food.Price), FMath::RoundToInt(Food.Heal)));
		};
		Page.Items.Add(Item);
	}
	Page.Items.Add(MakeItem(LOCTEXT("Leave", "Leave"), [Self]() {
		if (Self.IsValid())
		{
			Self->PopPage();
		}
	}));
	return Page;
}

AHadiHUD::FMenuPage AHadiHUD::BuildWardrobePage(bool bStore)
{
	FMenuPage Page;
	Page.Title = bStore ? LOCTEXT("StoreTitle", "Tide & Thread") : LOCTEXT("WardrobeTitle", "Wardrobe");
	TWeakObjectPtr<AHadiHUD> Self(this);
	Page.Subtitle = [Self]() {
		const AHadiGameMode* GameMode = Self.IsValid() ? Self->GetHadiGameMode() : nullptr;
		return FText::Format(LOCTEXT("WardrobeCash", "Cash: {0}"), FText::FromString(HadiHudStyle::MoneyText(GameMode ? GameMode->GetMoney() : 0)));
	};
	const AHadiGameMode* GameMode = GetHadiGameMode();
	for (int32 Index = 0; Index < hadi::OutfitCount(); ++Index)
	{
		if (!bStore && GameMode && !GameMode->OwnsOutfit(Index))
		{
			continue; // the home wardrobe only shows clothes Hadi owns
		}
		const hadi::Outfit& Outfit = hadi::GetOutfit(Index);
		FMenuItem Item = MakeItem(FText::FromString(ANSI_TO_TCHAR(Outfit.name)), [Self, Index]() {
			AHadiPlayerController* Controller = Self.IsValid() ? Self->GetHadiController() : nullptr;
			AHadiGameMode* Mode = Self.IsValid() ? Self->GetHadiGameMode() : nullptr;
			if (!Controller || !Mode)
			{
				return;
			}
			FText Message;
			Mode->BuyOrWearOutfit(Controller->GetHadi(), Index, Message);
			Self->PushNotification(Message);
		});
		const int32 Price = Outfit.price;
		Item.Value = [Self, Index, Price]() {
			const AHadiGameMode* Mode = Self.IsValid() ? Self->GetHadiGameMode() : nullptr;
			const AHadiPlayerController* Controller = Self.IsValid() ? Self->GetHadiController() : nullptr;
			const AHadiCharacter* Hadi = Controller ? Controller->GetHadi() : nullptr;
			if (Hadi && Hadi->GetOutfit() == Index)
			{
				return LOCTEXT("Wearing", "Wearing");
			}
			if (Mode && Mode->OwnsOutfit(Index))
			{
				return LOCTEXT("Owned", "Owned");
			}
			return FText::FromString(HadiHudStyle::MoneyText(Price));
		};
		Page.Items.Add(Item);
	}
	Page.Items.Add(MakeItem(LOCTEXT("Leave", "Leave"), [Self]() {
		if (Self.IsValid())
		{
			Self->PopPage();
		}
	}));
	return Page;
}

AHadiHUD::FMenuPage AHadiHUD::BuildSafehousePage()
{
	FMenuPage Page;
	Page.Title = LOCTEXT("SafehouseTitle", "Hadi's Apartment");
	Page.Subtitle = []() { return LOCTEXT("SafehouseSub", "The Seagrape Apartments, Coral Bay"); };
	TWeakObjectPtr<AHadiHUD> Self(this);
	Page.Items.Add(MakeItem(LOCTEXT("SafeSave", "Save Game"), [Self]() {
		if (Self.IsValid() && Self->GetHadiGameMode())
		{
			Self->GetHadiGameMode()->SaveGame();
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("Sleep", "Sleep 6 hours (restores health, saves)"), [Self]() {
		if (Self.IsValid() && Self->GetHadiGameMode())
		{
			Self->GetHadiGameMode()->SleepAtSafehouse();
			Self->FadeOutIn(1.5f);
			Self->PopPage();
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("Wardrobe", "Wardrobe"), [Self]() {
		if (Self.IsValid())
		{
			Self->PushPage(Self->BuildWardrobePage(false));
		}
	}));
	Page.Items.Add(MakeItem(LOCTEXT("Leave", "Leave"), [Self]() {
		if (Self.IsValid())
		{
			Self->PopPage();
		}
	}));
	return Page;
}

// ---------------------------------------------------------------------------- menu drawing
void AHadiHUD::DrawMenu()
{
	FMenuPage& Page = MenuStack.Last();
	const float S = UIScale;
	ItemRects.SetNum(Page.Items.Num());

	// Mouse hover selects items.
	float MouseX = 0.0f;
	float MouseY = 0.0f;
	const bool bHasMouse = PlayerOwner && PlayerOwner->GetMousePosition(MouseX, MouseY);
	const FVector2D Mouse(MouseX, MouseY);
	const bool bMouseMoved = bHasMouse && !Mouse.Equals(LastMouse, 1.0);
	LastMouse = Mouse;

	if (Page.bMainMenu)
	{
		DrawPanel(0.0f, 0.0f, 780.0f * S, Canvas->ClipY, FLinearColor(0.0f, 0.0f, 0.03f, 0.6f));
		DrawPanel(780.0f * S, 0.0f, 4.0f * S, Canvas->ClipY, FLinearColor(1.0f, 0.18f, 0.62f, 0.8f));
		DrawLabel(TEXT("HADI"), 110.0f * S, 110.0f * S, HadiHudStyle::White, FontHuge, 1.6f);
		const float Glow = 0.75f + 0.25f * FMath::Sin(HudTime * 2.0f);
		DrawLabel(TEXT("VICE CITY"), 118.0f * S, 290.0f * S, FLinearColor(1.0f, 0.18f * Glow, 0.62f * Glow, 1.0f), FontHuge, 0.75f);
		float Y = 450.0f * S;
		for (int32 Index = 0; Index < Page.Items.Num(); ++Index)
		{
			const FMenuItem& Item = Page.Items[Index];
			const bool bEnabled = IsItemEnabled(Item);
			const bool bSelected = Index == Page.Selected;
			const float RowH = 70.0f * S;
			ItemRects[Index] = FBox2D(FVector2D(100.0f * S, Y - 6.0f * S), FVector2D(740.0f * S, Y + RowH - 10.0f * S));
			if (bMouseMoved && bEnabled && ItemRects[Index].IsInside(Mouse))
			{
				Page.Selected = Index;
			}
			if (bSelected)
			{
				DrawPanel(100.0f * S, Y - 6.0f * S, 640.0f * S, RowH - 4.0f * S, FLinearColor(1.0f, 0.18f, 0.62f, 0.22f));
				DrawPanel(100.0f * S, Y - 6.0f * S, 6.0f * S, RowH - 4.0f * S, HadiHudStyle::Pink);
			}
			const FLinearColor Color = !bEnabled ? HadiHudStyle::Disabled : (bSelected ? HadiHudStyle::Cyan : HadiHudStyle::White);
			DrawLabel(Item.Label.ToString().ToUpper(), 126.0f * S, Y, Color, FontLarge);
			if (Item.Value)
			{
				DrawLabel(Item.Value().ToString(), 128.0f * S, Y + 40.0f * S, HadiHudStyle::Dim, FontSmall);
			}
			Y += RowH;
		}
		DrawLabel(TEXT("Original IP  -  Coral Key district preview"), 110.0f * S, Canvas->ClipY - 60.0f * S, HadiHudStyle::Dim, FontSmall);
		return;
	}

	// Centred panel pages.
	const float RowH = 46.0f * S;
	const float W = 820.0f * S;
	FString SubtitleText = Page.Subtitle ? Page.Subtitle().ToString() : FString();
	const TArray<FString> SubLines = SubtitleText.IsEmpty() ? TArray<FString>() : WrapText(SubtitleText, FontSmall, 1.0f, W - 80.0f * S);
	const float SubLineH = Measure(TEXT("Ag"), FontSmall).Y;
	const float HeaderH = 90.0f * S + SubLineH * static_cast<float>(SubLines.Num());
	const float H = HeaderH + RowH * static_cast<float>(Page.Items.Num()) + 70.0f * S;
	const float X = (Canvas->ClipX - W) * 0.5f;
	const float Y0 = FMath::Max(20.0f * S, (Canvas->ClipY - H) * 0.5f);
	if (Page.bPausesGame || MenuStack.Num() > 1)
	{
		DrawPanel(0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY, FLinearColor(0.0f, 0.0f, 0.0f, 0.45f));
	}
	DrawPanel(X, Y0, W, H, FLinearColor(0.02f, 0.02f, 0.05f, 0.88f));
	DrawPanel(X, Y0, W, 5.0f * S, HadiHudStyle::Pink);
	DrawLabel(Page.Title.ToString().ToUpper(), X + 40.0f * S, Y0 + 26.0f * S, HadiHudStyle::White, FontLarge);
	float SubY = Y0 + 74.0f * S;
	for (const FString& Line : SubLines)
	{
		DrawLabel(Line, X + 40.0f * S, SubY, HadiHudStyle::Dim, FontSmall);
		SubY += SubLineH;
	}
	float Y = Y0 + HeaderH;
	for (int32 Index = 0; Index < Page.Items.Num(); ++Index)
	{
		const FMenuItem& Item = Page.Items[Index];
		const bool bEnabled = IsItemEnabled(Item);
		const bool bSelected = Index == Page.Selected;
		ItemRects[Index] = FBox2D(FVector2D(X + 20.0f * S, Y), FVector2D(X + W - 20.0f * S, Y + RowH - 4.0f * S));
		if (bMouseMoved && bEnabled && ItemRects[Index].IsInside(Mouse))
		{
			Page.Selected = Index;
		}
		if (bSelected && bEnabled)
		{
			DrawPanel(X + 20.0f * S, Y, W - 40.0f * S, RowH - 4.0f * S, FLinearColor(1.0f, 0.18f, 0.62f, 0.25f));
			DrawPanel(X + 20.0f * S, Y, 5.0f * S, RowH - 4.0f * S, HadiHudStyle::Pink);
		}
		const FLinearColor Color = !Item.bSelectable ? HadiHudStyle::Dim : (!bEnabled ? HadiHudStyle::Disabled : (bSelected ? HadiHudStyle::Cyan : HadiHudStyle::White));
		DrawLabel(Item.Label.ToString(), X + 40.0f * S, Y + 8.0f * S, Color, FontMedium);
		if (Item.Value)
		{
			FString Value = Item.Value().ToString();
			if (Item.OnAdjust && bSelected)
			{
				Value = FString::Printf(TEXT("<  %s  >"), *Value);
			}
			DrawLabel(Value, X + W - 40.0f * S - Measure(Value, FontMedium).X, Y + 8.0f * S, Item.bSelectable ? HadiHudStyle::White : HadiHudStyle::Gold, FontMedium);
		}
		Y += RowH;
	}
	DrawLabel(TEXT("W/S or arrows: select    A/D: change    Enter: confirm    Esc/Backspace: back"), Canvas->ClipX * 0.5f, Y0 + H - 44.0f * S, HadiHudStyle::Dim,
		FontSmall, 1.0f, true);
}

#undef LOCTEXT_NAMESPACE
