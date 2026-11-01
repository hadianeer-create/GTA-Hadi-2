// HADI: VICE CITY
// Complete canvas-drawn interface: main menu, pause / settings / stats / controls pages, shop,
// wardrobe and safehouse menus, minimap, health & stamina, money, clock, objective panel,
// interaction prompt, notifications, mission banners, speedometer and FPS counter.
// Works with keyboard, gamepad and mouse; needs no UMG assets.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "HadiHUD.generated.h"

class AHadiGameMode;
class AHadiPlayerController;
class AHadiShop;
class UFont;
class UHadiMinimap;

UCLASS()
class HADIVICECITY_API AHadiHUD : public AHUD
{
	GENERATED_BODY()

public:
	AHadiHUD();

	virtual void DrawHUD() override;

	// ---- Menus ----
	bool IsMenuOpen() const { return MenuStack.Num() > 0; }
	void OpenMainMenu();
	void OpenPauseMenu();
	void OpenShopMenu(AHadiShop* Shop);
	void OpenSafehouseMenu();
	void CloseAllMenus();
	void MenuNavigate(int32 Direction);
	void MenuAdjust(int32 Direction);
	void MenuConfirm();
	void MenuBack();
	void MenuClick(const FVector2D& ScreenPosition);

	// ---- Messages ----
	void PushNotification(const FText& Message);
	void ShowBigMessage(const FText& Title, const FText& Subtitle, float Duration);
	void FadeOutIn(float Duration);

protected:
	virtual void BeginPlay() override;

private:
	struct FMenuItem
	{
		FText Label;
		TFunction<FText()> Value;
		TFunction<void()> OnActivate;
		TFunction<void(int32)> OnAdjust;
		TFunction<bool()> IsEnabled;
		bool bSelectable = true;
	};

	struct FMenuPage
	{
		FText Title;
		TFunction<FText()> Subtitle;
		TArray<FMenuItem> Items;
		int32 Selected = 0;
		bool bMainMenu = false;
		bool bPausesGame = false;
		TFunction<void()> OnClose;
	};

	struct FTextSize
	{
		float X = 0.0f;
		float Y = 0.0f;
	};

	struct FNotification
	{
		FText Text;
		float TimeLeft = 0.0f;
	};

	// Page builders.
	FMenuPage BuildMainMenu();
	FMenuPage BuildPauseMenu();
	FMenuPage BuildSettingsMenu();
	FMenuPage BuildStatsPage();
	FMenuPage BuildControlsPage();
	FMenuPage BuildConfirmNewGamePage();
	FMenuPage BuildShopPage(AHadiShop* Shop);
	FMenuPage BuildWardrobePage(bool bStore);
	FMenuPage BuildSafehousePage();
	static FMenuItem MakeItem(const FText& Label, TFunction<void()> OnActivate);
	static FMenuItem MakeInfo(const FText& Label, const FText& Value);
	void PushPage(FMenuPage&& Page);
	void PopPage();
	bool IsItemEnabled(const FMenuItem& Item) const;
	void FixSelection(FMenuPage& Page, int32 Direction);

	// Drawing.
	void EnsureResources();
	void DrawGameplay(float DeltaSeconds);
	void DrawMinimap();
	void DrawStatusBars();
	void DrawTopRight();
	void DrawObjective();
	void DrawPrompt();
	void DrawNotifications();
	void DrawVehicleHud();
	void DrawLocation();
	void DrawBigMessage();
	void DrawMenu();
	void DrawFade();
	void DrawFps(float DeltaSeconds);
	void DrawLabel(const FString& Text, float X, float Y, const FLinearColor& Color, UFont* Font, float Scale = 1.0f, bool bCenter = false, bool bShadow = true);
	FTextSize Measure(const FString& Text, UFont* Font, float Scale = 1.0f) const;
	TArray<FString> WrapText(const FString& Text, UFont* Font, float Scale, float MaxWidth) const;
	void DrawPanel(float X, float Y, float W, float H, const FLinearColor& Color);
	UFont* MakeFont(int32 Size) const;
	AHadiGameMode* GetHadiGameMode() const;
	AHadiPlayerController* GetHadiController() const;

	UPROPERTY(Transient)
	TObjectPtr<UHadiMinimap> Minimap;

	UPROPERTY(Transient)
	TObjectPtr<UFont> FontSmall;

	UPROPERTY(Transient)
	TObjectPtr<UFont> FontMedium;

	UPROPERTY(Transient)
	TObjectPtr<UFont> FontLarge;

	UPROPERTY(Transient)
	TObjectPtr<UFont> FontHuge;

	TArray<FMenuPage> MenuStack;
	TArray<FBox2D> ItemRects;
	TArray<FNotification> Notifications;
	TWeakObjectPtr<AHadiShop> ActiveShop;

	FText BigTitle;
	FText BigSubtitle;
	float BigTimeLeft = 0.0f;
	float BigDuration = 1.0f;
	float FadeTimeLeft = 0.0f;
	float FadeDuration = 1.0f;
	float SmoothedFps = 60.0f;
	float UIScale = 1.0f;
	float HudTime = 0.0f;
	FVector2D LastMouse = FVector2D(-1.0, -1.0);
	TArray<FVector2D> CachedRoute;
	float RouteTimer = 0.0f;
	bool bResourcesReady = false;
};
