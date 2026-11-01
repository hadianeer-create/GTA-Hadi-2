// HADI: VICE CITY
// Creates every Enhanced Input action / mapping in code (keyboard, mouse and gamepad), routes
// input to Hadi, the current vehicle or the menus, and runs the main-menu fly-over camera.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

#include "HadiPlayerController.generated.h"

class ACameraActor;
class AHadiCharacter;
class AHadiHUD;
class AHadiShop;
class AHadiVehicle;
class UInputAction;
class UInputMappingContext;
struct FKey;

UCLASS()
class HADIVICECITY_API AHadiPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHadiPlayerController();

	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	// Called by the game mode once Hadi has been spawned and possessed.
	void EnterGameplay();

	void OpenPauseMenu();
	void ClosePauseMenu();
	void OpenShopMenu(AHadiShop* Shop);
	void OpenSafehouseMenu();
	void SetMenuInputMode(bool bMenuOpen);
	void QuitToDesktop();

	AHadiCharacter* GetHadi() const;
	AHadiVehicle* GetVehicle() const;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	void CreateInputAssets();
	UInputAction* MakeAction(const TCHAR* Name, EInputActionValueType ValueType, bool bWhenPaused);
	void Map(UInputAction* Action, const FKey& Key, bool bNegate = false, bool bSwizzle = false, bool bDeadZone = false);
	AHadiHUD* GetHadiHUD() const;
	bool IsMenuOpen() const;
	void UpdateMenuCamera(float DeltaTime);

	void OnMove(const FInputActionValue& Value);
	void OnMoveStop(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnLookGamepad(const FInputActionValue& Value);
	void OnJumpStart(const FInputActionValue& Value);
	void OnJumpStop(const FInputActionValue& Value);
	void OnSprintStart(const FInputActionValue& Value);
	void OnSprintStop(const FInputActionValue& Value);
	void OnWalk(const FInputActionValue& Value);
	void OnInteract(const FInputActionValue& Value);
	void OnThrottle(const FInputActionValue& Value);
	void OnThrottleStop(const FInputActionValue& Value);
	void OnBrake(const FInputActionValue& Value);
	void OnBrakeStop(const FInputActionValue& Value);
	void OnCamera(const FInputActionValue& Value);
	void OnZoom(const FInputActionValue& Value);
	void OnPause(const FInputActionValue& Value);
	void OnMenuUp(const FInputActionValue& Value);
	void OnMenuDown(const FInputActionValue& Value);
	void OnMenuLeft(const FInputActionValue& Value);
	void OnMenuRight(const FInputActionValue& Value);
	void OnConfirm(const FInputActionValue& Value);
	void OnBack(const FInputActionValue& Value);
	void OnClick(const FInputActionValue& Value);

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> Context;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> LookGamepadAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> WalkAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ThrottleAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> BrakeAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> CameraAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MenuUpAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MenuDownAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MenuLeftAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MenuRightAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ConfirmAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> BackAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ClickAction;

	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> MenuCamera;

	float MenuCameraTime = 0.0f;
	bool bMappingAdded = false;
};
