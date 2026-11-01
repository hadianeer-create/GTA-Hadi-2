// HADI: VICE CITY
#include "Player/HadiPlayerController.h"

#include "HadiViceCity.h"
#include "Characters/HadiCharacter.h"
#include "Game/HadiGameUserSettings.h"
#include "UI/HadiHUD.h"
#include "Vehicles/HadiVehicle.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/KismetSystemLibrary.h"

AHadiPlayerController::AHadiPlayerController()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
}

// ------------------------------------------------------------------------------ input assets
UInputAction* AHadiPlayerController::MakeAction(const TCHAR* Name, EInputActionValueType ValueType, bool bWhenPaused)
{
	UInputAction* Action = NewObject<UInputAction>(this, Name);
	Action->ValueType = ValueType;
	Action->bTriggerWhenPaused = bWhenPaused;
	return Action;
}

void AHadiPlayerController::Map(UInputAction* Action, const FKey& Key, bool bNegate, bool bSwizzle, bool bDeadZone)
{
	FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);
	if (bSwizzle)
	{
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(Context);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		Mapping.Modifiers.Add(Swizzle);
	}
	if (bNegate)
	{
		Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(Context));
	}
	if (bDeadZone)
	{
		Mapping.Modifiers.Add(NewObject<UInputModifierDeadZone>(Context));
	}
}

void AHadiPlayerController::CreateInputAssets()
{
	if (Context)
	{
		return;
	}
	Context = NewObject<UInputMappingContext>(this, TEXT("IMC_Hadi"));

	MoveAction = MakeAction(TEXT("IA_Move"), EInputActionValueType::Axis2D, false);
	LookAction = MakeAction(TEXT("IA_Look"), EInputActionValueType::Axis2D, false);
	LookGamepadAction = MakeAction(TEXT("IA_LookGamepad"), EInputActionValueType::Axis2D, false);
	JumpAction = MakeAction(TEXT("IA_Jump"), EInputActionValueType::Boolean, false);
	SprintAction = MakeAction(TEXT("IA_Sprint"), EInputActionValueType::Boolean, false);
	WalkAction = MakeAction(TEXT("IA_Walk"), EInputActionValueType::Boolean, false);
	InteractAction = MakeAction(TEXT("IA_Interact"), EInputActionValueType::Boolean, false);
	ThrottleAction = MakeAction(TEXT("IA_Throttle"), EInputActionValueType::Axis1D, false);
	BrakeAction = MakeAction(TEXT("IA_Brake"), EInputActionValueType::Axis1D, false);
	CameraAction = MakeAction(TEXT("IA_Camera"), EInputActionValueType::Boolean, false);
	ZoomAction = MakeAction(TEXT("IA_Zoom"), EInputActionValueType::Axis1D, false);
	PauseAction = MakeAction(TEXT("IA_Pause"), EInputActionValueType::Boolean, true);
	MenuUpAction = MakeAction(TEXT("IA_MenuUp"), EInputActionValueType::Boolean, true);
	MenuDownAction = MakeAction(TEXT("IA_MenuDown"), EInputActionValueType::Boolean, true);
	MenuLeftAction = MakeAction(TEXT("IA_MenuLeft"), EInputActionValueType::Boolean, true);
	MenuRightAction = MakeAction(TEXT("IA_MenuRight"), EInputActionValueType::Boolean, true);
	ConfirmAction = MakeAction(TEXT("IA_Confirm"), EInputActionValueType::Boolean, true);
	BackAction = MakeAction(TEXT("IA_Back"), EInputActionValueType::Boolean, true);
	ClickAction = MakeAction(TEXT("IA_Click"), EInputActionValueType::Boolean, true);

	// On foot: WASD / left stick. In a car the same action steers (X).
	Map(MoveAction, EKeys::W, false, true);
	Map(MoveAction, EKeys::S, true, true);
	Map(MoveAction, EKeys::A, true);
	Map(MoveAction, EKeys::D);
	Map(MoveAction, EKeys::Gamepad_Left2D, false, false, true);

	// Camera: mouse uses the engine's mouse sensitivity scaling, the right stick is rate based.
	Context->MapKey(LookAction, EKeys::Mouse2D).Modifiers.Add([this]() {
		UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(Context);
		Negate->bX = false;
		Negate->bY = true;
		Negate->bZ = false;
		return Negate;
	}());
	Map(LookGamepadAction, EKeys::Gamepad_Right2D, false, false, true);

	Map(JumpAction, EKeys::SpaceBar);
	Map(JumpAction, EKeys::Gamepad_FaceButton_Bottom);
	Map(SprintAction, EKeys::LeftShift);
	Map(SprintAction, EKeys::Gamepad_FaceButton_Left);
	Map(WalkAction, EKeys::LeftControl);
	Map(WalkAction, EKeys::Gamepad_LeftThumbstick);
	Map(InteractAction, EKeys::E);
	Map(InteractAction, EKeys::F);
	Map(InteractAction, EKeys::Gamepad_FaceButton_Top);

	Map(ThrottleAction, EKeys::W);
	Map(ThrottleAction, EKeys::Up);
	Map(ThrottleAction, EKeys::Gamepad_RightTriggerAxis);
	Map(BrakeAction, EKeys::S);
	Map(BrakeAction, EKeys::Down);
	Map(BrakeAction, EKeys::Gamepad_LeftTriggerAxis);
	Map(CameraAction, EKeys::V);
	Map(CameraAction, EKeys::Gamepad_Special_Left);
	Map(ZoomAction, EKeys::MouseWheelAxis);

	Map(PauseAction, EKeys::Escape);
	Map(PauseAction, EKeys::P);
	Map(PauseAction, EKeys::Gamepad_Special_Right);
	Map(MenuUpAction, EKeys::Up);
	Map(MenuUpAction, EKeys::W);
	Map(MenuUpAction, EKeys::Gamepad_DPad_Up);
	Map(MenuDownAction, EKeys::Down);
	Map(MenuDownAction, EKeys::S);
	Map(MenuDownAction, EKeys::Gamepad_DPad_Down);
	Map(MenuLeftAction, EKeys::Left);
	Map(MenuLeftAction, EKeys::A);
	Map(MenuLeftAction, EKeys::Gamepad_DPad_Left);
	Map(MenuRightAction, EKeys::Right);
	Map(MenuRightAction, EKeys::D);
	Map(MenuRightAction, EKeys::Gamepad_DPad_Right);
	Map(ConfirmAction, EKeys::Enter);
	Map(ConfirmAction, EKeys::SpaceBar);
	Map(ConfirmAction, EKeys::Gamepad_FaceButton_Bottom);
	Map(BackAction, EKeys::BackSpace);
	Map(BackAction, EKeys::Gamepad_FaceButton_Right);
	Map(ClickAction, EKeys::LeftMouseButton);
}

void AHadiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	CreateInputAssets();
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (!Input)
	{
		UE_LOG(LogHadi, Error, TEXT("Enhanced Input is not the default input component. Check Config/DefaultInput.ini."));
		return;
	}
	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHadiPlayerController::OnMove);
	Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &AHadiPlayerController::OnMoveStop);
	Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHadiPlayerController::OnLook);
	Input->BindAction(LookGamepadAction, ETriggerEvent::Triggered, this, &AHadiPlayerController::OnLookGamepad);
	Input->BindAction(JumpAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnJumpStart);
	Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHadiPlayerController::OnJumpStop);
	Input->BindAction(SprintAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnSprintStart);
	Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHadiPlayerController::OnSprintStop);
	Input->BindAction(WalkAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnWalk);
	Input->BindAction(InteractAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnInteract);
	Input->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AHadiPlayerController::OnThrottle);
	Input->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AHadiPlayerController::OnThrottleStop);
	Input->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AHadiPlayerController::OnBrake);
	Input->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AHadiPlayerController::OnBrakeStop);
	Input->BindAction(CameraAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnCamera);
	Input->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AHadiPlayerController::OnZoom);
	Input->BindAction(PauseAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnPause);
	Input->BindAction(MenuUpAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnMenuUp);
	Input->BindAction(MenuDownAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnMenuDown);
	Input->BindAction(MenuLeftAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnMenuLeft);
	Input->BindAction(MenuRightAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnMenuRight);
	Input->BindAction(ConfirmAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnConfirm);
	Input->BindAction(BackAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnBack);
	Input->BindAction(ClickAction, ETriggerEvent::Started, this, &AHadiPlayerController::OnClick);
}

void AHadiPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController())
	{
		return;
	}
	CreateInputAssets();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(Context, 0);
		bMappingAdded = true;
	}

	// Main menu: cinematic fly-over of Coral Key until the player starts / continues a game.
	if (!GetPawn())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		MenuCamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FTransform::Identity, Params);
		if (MenuCamera)
		{
			MenuCamera->GetCameraComponent()->SetFieldOfView(55.0f);
			MenuCamera->GetCameraComponent()->bConstrainAspectRatio = false;
			UpdateMenuCamera(0.0f);
			SetViewTarget(MenuCamera);
		}
		SetMenuInputMode(true);
	}
}

void AHadiPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = -70.0f;
		PlayerCameraManager->ViewPitchMax = 45.0f;
	}
}

void AHadiPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (!GetPawn() && MenuCamera)
	{
		UpdateMenuCamera(DeltaTime);
	}
}

void AHadiPlayerController::UpdateMenuCamera(float DeltaTime)
{
	MenuCameraTime += DeltaTime;
	// Slow orbit around the island, looking at the deco beachfront.
	const float Angle = FMath::DegreesToRadians(200.0f + MenuCameraTime * 2.2f);
	const FVector Focus(-3000.0f, 6000.0f, 1500.0f);
	const FVector Position = Focus + FVector(FMath::Cos(Angle) * 90000.0f, FMath::Sin(Angle) * 70000.0f, 26000.0f);
	MenuCamera->SetActorLocationAndRotation(Position, (Focus - Position).Rotation());
}

void AHadiPlayerController::EnterGameplay()
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->CloseAllMenus();
	}
	SetMenuInputMode(false);
	if (MenuCamera)
	{
		MenuCamera->Destroy();
		MenuCamera = nullptr;
	}
}

void AHadiPlayerController::SetMenuInputMode(bool bMenuOpen)
{
	bShowMouseCursor = bMenuOpen;
	if (bMenuOpen)
	{
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
}

void AHadiPlayerController::OpenPauseMenu()
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		SetPause(true);
		HUD->OpenPauseMenu();
		SetMenuInputMode(true);
	}
}

void AHadiPlayerController::ClosePauseMenu()
{
	SetPause(false);
	SetMenuInputMode(false);
}

void AHadiPlayerController::OpenShopMenu(AHadiShop* Shop)
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->OpenShopMenu(Shop);
		SetMenuInputMode(true);
	}
}

void AHadiPlayerController::OpenSafehouseMenu()
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->OpenSafehouseMenu();
		SetMenuInputMode(true);
	}
}

void AHadiPlayerController::QuitToDesktop()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

AHadiCharacter* AHadiPlayerController::GetHadi() const
{
	return Cast<AHadiCharacter>(GetPawn());
}

AHadiVehicle* AHadiPlayerController::GetVehicle() const
{
	return Cast<AHadiVehicle>(GetPawn());
}

AHadiHUD* AHadiPlayerController::GetHadiHUD() const
{
	return GetHUD<AHadiHUD>();
}

bool AHadiPlayerController::IsMenuOpen() const
{
	const AHadiHUD* HUD = GetHadiHUD();
	return HUD && HUD->IsMenuOpen();
}

// --------------------------------------------------------------------------- input handlers
void AHadiPlayerController::OnMove(const FInputActionValue& Value)
{
	if (IsMenuOpen())
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	if (AHadiCharacter* Hadi = GetHadi())
	{
		Hadi->MoveInput(Axis);
	}
	else if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->SetSteer(static_cast<float>(Axis.X));
	}
}

void AHadiPlayerController::OnMoveStop(const FInputActionValue& /*Value*/)
{
	if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->SetSteer(0.0f);
	}
}

void AHadiPlayerController::OnLook(const FInputActionValue& Value)
{
	if (IsMenuOpen() || !GetPawn())
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	const UHadiGameUserSettings* Settings = UHadiGameUserSettings::GetHadiSettings();
	const float Sensitivity = Settings ? Settings->MouseSensitivity : 1.0f;
	const float Invert = (Settings && Settings->bInvertLook) ? -1.0f : 1.0f;
	AddYawInput(static_cast<float>(Axis.X) * Sensitivity);
	AddPitchInput(static_cast<float>(Axis.Y) * Sensitivity * Invert);
	if (AHadiVehicle* Vehicle = GetVehicle())
	{
		if (Axis.SizeSquared() > 0.01)
		{
			Vehicle->NotifyLookInput();
		}
	}
}

void AHadiPlayerController::OnLookGamepad(const FInputActionValue& Value)
{
	if (IsMenuOpen() || !GetPawn())
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	const UHadiGameUserSettings* Settings = UHadiGameUserSettings::GetHadiSettings();
	const float Sensitivity = Settings ? Settings->MouseSensitivity : 1.0f;
	const float Invert = (Settings && Settings->bInvertLook) ? -1.0f : 1.0f;
	// Rate-based: ~160 degrees/second at full deflection (engine applies a 2.5x legacy scale).
	const float Rate = 64.0f * GetWorld()->GetDeltaSeconds() * Sensitivity;
	AddYawInput(static_cast<float>(Axis.X) * Rate);
	AddPitchInput(-static_cast<float>(Axis.Y) * Rate * Invert);
	if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->NotifyLookInput();
	}
}

void AHadiPlayerController::OnJumpStart(const FInputActionValue& /*Value*/)
{
	if (IsMenuOpen())
	{
		return;
	}
	if (AHadiCharacter* Hadi = GetHadi())
	{
		Hadi->JumpPressed();
	}
	else if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->SetHandbrake(true);
	}
}

void AHadiPlayerController::OnJumpStop(const FInputActionValue& /*Value*/)
{
	if (AHadiCharacter* Hadi = GetHadi())
	{
		Hadi->JumpReleased();
	}
	else if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->SetHandbrake(false);
	}
}

void AHadiPlayerController::OnSprintStart(const FInputActionValue& /*Value*/)
{
	if (AHadiCharacter* Hadi = GetHadi())
	{
		Hadi->SetSprintHeld(true);
	}
}

void AHadiPlayerController::OnSprintStop(const FInputActionValue& /*Value*/)
{
	if (AHadiCharacter* Hadi = GetHadi())
	{
		Hadi->SetSprintHeld(false);
	}
}

void AHadiPlayerController::OnWalk(const FInputActionValue& /*Value*/)
{
	if (AHadiCharacter* Hadi = GetHadi())
	{
		Hadi->ToggleWalk();
	}
}

void AHadiPlayerController::OnInteract(const FInputActionValue& /*Value*/)
{
	if (IsMenuOpen())
	{
		return;
	}
	if (AHadiCharacter* Hadi = GetHadi())
	{
		Hadi->InteractPressed();
	}
	else if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->RequestExit();
	}
}

void AHadiPlayerController::OnThrottle(const FInputActionValue& Value)
{
	if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->SetThrottle(IsMenuOpen() ? 0.0f : Value.Get<float>());
	}
}

void AHadiPlayerController::OnThrottleStop(const FInputActionValue& /*Value*/)
{
	if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->SetThrottle(0.0f);
	}
}

void AHadiPlayerController::OnBrake(const FInputActionValue& Value)
{
	if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->SetBrake(IsMenuOpen() ? 0.0f : Value.Get<float>());
	}
}

void AHadiPlayerController::OnBrakeStop(const FInputActionValue& /*Value*/)
{
	if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->SetBrake(0.0f);
	}
}

void AHadiPlayerController::OnCamera(const FInputActionValue& /*Value*/)
{
	if (IsMenuOpen())
	{
		return;
	}
	if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->ToggleCamera();
	}
}

void AHadiPlayerController::OnZoom(const FInputActionValue& Value)
{
	if (IsMenuOpen())
	{
		return;
	}
	const float Delta = Value.Get<float>();
	if (AHadiCharacter* Hadi = GetHadi())
	{
		Hadi->ZoomCamera(Delta);
	}
	else if (AHadiVehicle* Vehicle = GetVehicle())
	{
		Vehicle->ZoomCamera(Delta);
	}
}

void AHadiPlayerController::OnPause(const FInputActionValue& /*Value*/)
{
	AHadiHUD* HUD = GetHadiHUD();
	if (!HUD)
	{
		return;
	}
	if (HUD->IsMenuOpen())
	{
		HUD->MenuBack();
	}
	else if (GetPawn())
	{
		OpenPauseMenu();
	}
}

void AHadiPlayerController::OnMenuUp(const FInputActionValue& /*Value*/)
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->MenuNavigate(-1);
	}
}

void AHadiPlayerController::OnMenuDown(const FInputActionValue& /*Value*/)
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->MenuNavigate(1);
	}
}

void AHadiPlayerController::OnMenuLeft(const FInputActionValue& /*Value*/)
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->MenuAdjust(-1);
	}
}

void AHadiPlayerController::OnMenuRight(const FInputActionValue& /*Value*/)
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->MenuAdjust(1);
	}
}

void AHadiPlayerController::OnConfirm(const FInputActionValue& /*Value*/)
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->MenuConfirm();
	}
}

void AHadiPlayerController::OnBack(const FInputActionValue& /*Value*/)
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->MenuBack();
	}
}

void AHadiPlayerController::OnClick(const FInputActionValue& /*Value*/)
{
	float X = 0.0f;
	float Y = 0.0f;
	AHadiHUD* HUD = GetHadiHUD();
	if (HUD && GetMousePosition(X, Y))
	{
		HUD->MenuClick(FVector2D(X, Y));
	}
}
