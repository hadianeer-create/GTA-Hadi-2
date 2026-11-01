// HADI: VICE CITY
#include "Characters/HadiCharacter.h"

#include "Characters/HadiRigComponent.h"
#include "Game/HadiGameInstance.h"
#include "Game/HadiGameMode.h"
#include "Game/HadiGameUserSettings.h"
#include "Gameplay/HadiInteractable.h"
#include "Vehicles/HadiVehicle.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

AHadiCharacter::AHadiCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::Disabled;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 90.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.0f, 620.0f, 0.0f);
	Move->JumpZVelocity = 470.0f;
	Move->AirControl = 0.3f;
	Move->MaxWalkSpeed = JogSpeed;
	Move->MinAnalogWalkSpeed = 20.0f;
	Move->BrakingDecelerationWalking = 1800.0f;
	Move->BrakingDecelerationFlying = 900.0f;
	Move->GroundFriction = 8.0f;
	Move->MaxStepHeight = 45.0f;
	Move->MaxFlySpeed = SwimSpeed;
	Move->SetWalkableFloorAngle(46.0f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = DesiredArmLength;
	CameraBoom->SocketOffset = FVector(0.0f, 45.0f, 65.0f);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 14.0f;
	CameraBoom->ProbeSize = 14.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->SetFieldOfView(80.0f);

	// Procedural body: rig root sits at the bottom of the capsule (Hadi's feet).
	Rig = CreateDefaultSubobject<UHadiRigComponent>(TEXT("Rig"));
	Rig->SetupAttachment(RootComponent);
	Rig->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
}

void AHadiCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	Stamina = MaxStamina;
	LastYaw = static_cast<float>(GetActorRotation().Yaw);
	if (const AHadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AHadiGameMode>())
	{
		WaterLevel = GameMode->GetSeaLevel();
	}
}

void AHadiCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CurrentVehicle && !bDead)
	{
		if (bMantling)
		{
			UpdateMantle(DeltaSeconds);
		}
		else
		{
			UpdateSwimming(DeltaSeconds);
		}
		UpdateStamina(DeltaSeconds);
		UpdateMovementSpeeds();
		UpdateInteraction(DeltaSeconds);
		UpdateStats(DeltaSeconds);
	}
	UpdateHealth(DeltaSeconds);
	UpdateCamera(DeltaSeconds);
	UpdateAnimation(DeltaSeconds);
}

// ------------------------------------------------------------------------------------- input
void AHadiCharacter::MoveInput(const FVector2D& Axis)
{
	if (!Controller || bDead || bMantling || CurrentVehicle)
	{
		return;
	}
	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Forward, static_cast<float>(Axis.Y));
	AddMovementInput(Right, static_cast<float>(Axis.X));
}

void AHadiCharacter::JumpPressed()
{
	if (bDead || bMantling || CurrentVehicle)
	{
		return;
	}
	// Climbing takes priority: jumping at a wall vaults / climbs onto it (also out of the water).
	if (TryMantle())
	{
		return;
	}
	if (!bSwimming)
	{
		Jump();
	}
}

void AHadiCharacter::JumpReleased()
{
	StopJumping();
}

void AHadiCharacter::SetSprintHeld(bool bHeld)
{
	bSprintHeld = bHeld;
}

void AHadiCharacter::ToggleWalk()
{
	bWalkMode = !bWalkMode;
}

void AHadiCharacter::InteractPressed()
{
	if (bDead || bMantling || CurrentVehicle)
	{
		return;
	}
	if (IHadiInteractable* Interactable = Cast<IHadiInteractable>(FocusedInteractable.Get()))
	{
		Interactable->Interact(this);
	}
}

void AHadiCharacter::ZoomCamera(float Delta)
{
	DesiredArmLength = FMath::Clamp(DesiredArmLength - Delta * 40.0f, 180.0f, 650.0f);
}

FText AHadiCharacter::GetInteractionPromptText() const
{
	if (const IHadiInteractable* Interactable = Cast<IHadiInteractable>(FocusedInteractable.Get()))
	{
		return Interactable->GetInteractionPrompt(this);
	}
	return FText::GetEmpty();
}

// ------------------------------------------------------------------------------- movement
void AHadiCharacter::UpdateStamina(float DeltaSeconds)
{
	const float Speed = static_cast<float>(GetVelocity().Size2D());
	const bool bCanSprintHere = GetCharacterMovement()->IsMovingOnGround() || bSwimming;
	bSprinting = bSprintHeld && !bStaminaExhausted && bCanSprintHere && Speed > 50.0f;
	if (bSprinting)
	{
		Stamina = FMath::Max(0.0f, Stamina - StaminaDrainPerSecond * DeltaSeconds);
		if (Stamina <= 0.0f)
		{
			bStaminaExhausted = true;
			bSprinting = false;
		}
	}
	else
	{
		Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenPerSecond * DeltaSeconds);
		if (bStaminaExhausted && Stamina > 25.0f)
		{
			bStaminaExhausted = false;
		}
	}
}

void AHadiCharacter::UpdateMovementSpeeds()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->MaxWalkSpeed = bSprinting ? SprintSpeed : (bWalkMode ? WalkSpeed : JogSpeed);
	Move->MaxFlySpeed = bSprinting ? SwimSprintSpeed : SwimSpeed;
}

void AHadiCharacter::UpdateSwimming(float /*DeltaSeconds*/)
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	const FVector Location = GetActorLocation();
	if (!bSwimming)
	{
		// Chest-deep water (capsule centre 35 cm below the surface) -> swim.
		if ((Move->IsMovingOnGround() || Move->IsFalling()) && Location.Z < WaterLevel - 35.0f)
		{
			StartSwimming();
		}
		return;
	}

	// Stand up again once the sea floor is shallow enough (hysteresis vs. the entry depth).
	FHitResult Hit;
	const FCollisionQueryParams Params(FName(TEXT("HadiSwimFloor")), false, this);
	const bool bFloor = GetWorld()->LineTraceSingleByChannel(Hit, Location, Location - FVector(0.0f, 0.0f, 260.0f), ECC_Visibility, Params);
	if (bFloor && Hit.ImpactPoint.Z > WaterLevel - 100.0f)
	{
		StopSwimming();
		return;
	}
	// Buoyancy: float with the head and shoulders above the surface.
	const float TargetZ = WaterLevel - 15.0f;
	Move->Velocity.Z = FMath::Clamp(static_cast<float>((TargetZ - Location.Z) * 4.0), -250.0f, 250.0f);
}

void AHadiCharacter::StartSwimming()
{
	bSwimming = true;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->SetMovementMode(MOVE_Flying);
	Move->Velocity *= 0.4f;
}

void AHadiCharacter::StopSwimming()
{
	bSwimming = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

bool AHadiCharacter::TryMantle()
{
	UWorld* World = GetWorld();
	const float Radius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Location = GetActorLocation();
	const FVector Feet = Location - FVector(0.0f, 0.0f, HalfHeight);
	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.0f;
	if (!Forward.Normalize())
	{
		return false;
	}
	const FCollisionQueryParams Params(FName(TEXT("HadiMantle")), false, this);

	// 1) A wall within arm's reach at knee, waist or chest height.
	FHitResult WallHit;
	bool bWall = false;
	const float ProbeHeights[3] = {60.0f, 110.0f, 160.0f};
	for (const float ProbeHeight : ProbeHeights)
	{
		const FVector Start = Feet + FVector(0.0f, 0.0f, ProbeHeight);
		if (World->LineTraceSingleByChannel(WallHit, Start, Start + Forward * (Radius + 75.0f), ECC_Visibility, Params))
		{
			bWall = true;
			break;
		}
	}
	if (!bWall || WallHit.ImpactNormal.Z > 0.6)
	{
		return false; // ramps and slopes are walked, not climbed
	}

	// 2) A walkable ledge on top of it.
	const FVector Over = WallHit.ImpactPoint + Forward * 30.0f;
	const FVector TopStart(Over.X, Over.Y, Feet.Z + MantleMaxHeight + 20.0f);
	const FVector TopEnd(Over.X, Over.Y, Feet.Z + MantleMinHeight - 10.0f);
	FHitResult TopHit;
	if (!World->LineTraceSingleByChannel(TopHit, TopStart, TopEnd, ECC_Visibility, Params) || TopHit.bStartPenetrating || TopHit.ImpactNormal.Z < 0.7)
	{
		return false;
	}
	const float LedgeHeight = static_cast<float>(TopHit.ImpactPoint.Z - Feet.Z);
	if (LedgeHeight < MantleMinHeight || LedgeHeight > MantleMaxHeight)
	{
		return false;
	}

	// 3) Room to stand up there.
	const FVector Destination = TopHit.ImpactPoint + FVector(0.0f, 0.0f, HalfHeight + 3.0f);
	if (World->OverlapBlockingTestByChannel(Destination, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(Radius, HalfHeight), Params))
	{
		return false;
	}

	bMantling = true;
	bSwimming = false;
	MantleStart = Location;
	MantleTarget = Destination;
	MantleTime = 0.0f;
	MantleDuration = FMath::Lerp(0.45f, 0.95f, FMath::Clamp((LedgeHeight - MantleMinHeight) / (MantleMaxHeight - MantleMinHeight), 0.0f, 1.0f));
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->StopMovementImmediately();
	Move->SetMovementMode(MOVE_Flying);
	if (UHadiGameInstance* GameInstance = GetGameInstance<UHadiGameInstance>())
	{
		GameInstance->Profile.Stats.LedgesClimbed++;
	}
	ShowExpression(hadi::Expression::Focused, MantleDuration);
	return true;
}

void AHadiCharacter::UpdateMantle(float DeltaSeconds)
{
	MantleTime += DeltaSeconds;
	const float Alpha = FMath::Clamp(MantleTime / MantleDuration, 0.0f, 1.0f);
	// Pull up first, then roll forward over the edge.
	const float Rise = FMath::SmoothStep(0.0f, 0.6f, Alpha);
	const float Over = FMath::SmoothStep(0.45f, 1.0f, Alpha);
	FVector Position;
	Position.X = FMath::Lerp(MantleStart.X, MantleTarget.X, Over);
	Position.Y = FMath::Lerp(MantleStart.Y, MantleTarget.Y, Over);
	Position.Z = FMath::Lerp(MantleStart.Z, MantleTarget.Z, Rise);
	SetActorLocation(Position, false, nullptr, ETeleportType::None);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	if (Alpha >= 1.0f)
	{
		bMantling = false;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void AHadiCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	const float ImpactSpeed = static_cast<float>(-GetVelocity().Z);
	if (ImpactSpeed > FallDamageSpeed)
	{
		ApplyHadiDamage((ImpactSpeed - FallDamageSpeed) * FallDamagePerSpeed);
	}
	else if (ImpactSpeed > 750.0f)
	{
		ShowExpression(hadi::Expression::Surprised, 0.8f);
	}
}

// ------------------------------------------------------------------------------ interaction
void AHadiCharacter::UpdateInteraction(float DeltaSeconds)
{
	InteractionTimer -= DeltaSeconds;
	if (InteractionTimer > 0.0f)
	{
		return;
	}
	InteractionTimer = 0.1f;
	if (bMantling)
	{
		FocusedInteractable = nullptr;
		return;
	}
	if (const UHadiInteractionSubsystem* Interactions = GetWorld()->GetSubsystem<UHadiInteractionSubsystem>())
	{
		FocusedInteractable = Interactions->FindBest(this);
	}
}

// ---------------------------------------------------------------------------------- health
void AHadiCharacter::ApplyHadiDamage(float Amount)
{
	if (bDead || Amount <= 0.0f)
	{
		return;
	}
	Health = FMath::Max(0.0f, Health - Amount);
	TimeSinceDamage = 0.0f;
	ShowExpression(hadi::Expression::Angry, 2.5f);
	if (Health <= 0.0f)
	{
		Die();
	}
}

void AHadiCharacter::Heal(float Amount)
{
	if (!bDead)
	{
		Health = FMath::Min(MaxHealth, Health + Amount);
	}
}

void AHadiCharacter::SetHealthDirect(float NewHealth)
{
	Health = FMath::Clamp(NewHealth, 1.0f, MaxHealth);
}

void AHadiCharacter::UpdateHealth(float DeltaSeconds)
{
	TimeSinceDamage += DeltaSeconds;
	if (!bDead && TimeSinceDamage > 5.0f && Health < RegenCap)
	{
		Health = FMath::Min(RegenCap, Health + RegenPerSecond * DeltaSeconds);
	}
	if (ExpressionTimer > 0.0f)
	{
		ExpressionTimer -= DeltaSeconds;
	}
}

void AHadiCharacter::Die()
{
	bDead = true;
	bSwimming = false;
	bMantling = false;
	bSprinting = false;
	GetCharacterMovement()->DisableMovement();
	if (AHadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AHadiGameMode>())
	{
		GameMode->OnPlayerDied(this);
	}
}

void AHadiCharacter::ReviveAt(const FVector& Location, float Yaw, float NewHealth)
{
	bDead = false;
	Health = FMath::Clamp(NewHealth, 1.0f, MaxHealth);
	Stamina = MaxStamina;
	TimeSinceDamage = 100.0f;
	SetActorLocationAndRotation(Location, FRotator(0.0f, Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	if (Controller)
	{
		Controller->SetControlRotation(FRotator(-10.0f, Yaw, 0.0f));
	}
}

// ------------------------------------------------------------------------------- wardrobe
void AHadiCharacter::SetOutfit(int32 OutfitIndex)
{
	Rig->ApplyOutfit(OutfitIndex);
}

int32 AHadiCharacter::GetOutfit() const
{
	return Rig->GetOutfitIndex();
}

void AHadiCharacter::ShowExpression(hadi::Expression InExpression, float Duration)
{
	Expression = InExpression;
	ExpressionTimer = Duration;
}

hadi::Expression AHadiCharacter::ChooseExpression() const
{
	if (ExpressionTimer > 0.0f)
	{
		return Expression;
	}
	if (bDead)
	{
		return hadi::Expression::Neutral;
	}
	if (CurrentVehicle && CurrentVehicle->GetSpeedKmh() > 110.0f)
	{
		return hadi::Expression::Happy;
	}
	if (bSprinting)
	{
		return hadi::Expression::Focused;
	}
	if (Health < 30.0f)
	{
		return hadi::Expression::Sad;
	}
	return hadi::Expression::Neutral;
}

// -------------------------------------------------------------------------------- vehicles
void AHadiCharacter::OnEnteredVehicle(AHadiVehicle* Vehicle, const FVector& SeatOffset)
{
	CurrentVehicle = Vehicle;
	bSprintHeld = false;
	bSprinting = false;
	bSwimming = false;
	FocusedInteractable = nullptr;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->StopMovementImmediately();
	Move->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachToComponent(Vehicle->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(SeatOffset);
	SetActorRelativeRotation(FRotator::ZeroRotator);
}

void AHadiCharacter::OnExitedVehicle(const FVector& ExitLocation, float ExitYaw)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CurrentVehicle = nullptr;
	SetActorLocationAndRotation(ExitLocation, FRotator(0.0f, ExitYaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	SetRigHidden(false);
	LastYaw = ExitYaw;
}

void AHadiCharacter::SetRigHidden(bool bInHidden)
{
	Rig->SetRigHidden(bInHidden);
}

// ------------------------------------------------------------------------- camera & anim
void AHadiCharacter::UpdateCamera(float DeltaSeconds)
{
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, bSwimming ? DesiredArmLength + 60.0f : DesiredArmLength, DeltaSeconds, 6.0f);
	const FVector TargetOffset = bSwimming ? FVector(0.0f, 40.0f, 30.0f) : FVector(0.0f, 45.0f, 65.0f);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetOffset, DeltaSeconds, 4.0f);

	float BaseFov = 80.0f;
	if (const UHadiGameUserSettings* Settings = UHadiGameUserSettings::GetHadiSettings())
	{
		BaseFov = Settings->FieldOfView;
	}
	const float TargetFov = BaseFov + (bSprinting ? 7.0f : 0.0f);
	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetFov, DeltaSeconds, 5.0f));
}

void AHadiCharacter::UpdateAnimation(float DeltaSeconds)
{
	if (DeltaSeconds <= 0.0f)
	{
		return;
	}
	const FVector Velocity = GetVelocity();
	const float Yaw = static_cast<float>(GetActorRotation().Yaw);

	hadi::AnimInput Input;
	Input.speed = CurrentVehicle ? 0.0f : static_cast<float>(Velocity.Size2D());
	Input.verticalSpeed = static_cast<float>(Velocity.Z);
	Input.turnRate = hadi::WrapDegrees(Yaw - LastYaw) / DeltaSeconds;
	Input.grounded = GetCharacterMovement()->IsMovingOnGround() || bMantling || CurrentVehicle != nullptr;
	Input.swimming = bSwimming;
	Input.driving = CurrentVehicle != nullptr;
	Input.steer = CurrentVehicle ? CurrentVehicle->GetSteer() : 0.0f;
	Input.climbing = bMantling;
	Input.climbAlpha = bMantling ? FMath::Clamp(MantleTime / MantleDuration, 0.0f, 1.0f) : 0.0f;
	Input.dead = bDead;
	Input.expression = ChooseExpression();
	if (Controller && !CurrentVehicle)
	{
		const FRotator ControlRotation = Controller->GetControlRotation();
		Input.lookYaw = hadi::WrapDegrees(static_cast<float>(ControlRotation.Yaw) - Yaw);
		Input.lookPitch = hadi::WrapDegrees(static_cast<float>(ControlRotation.Pitch));
	}
	LastYaw = Yaw;
	Rig->UpdateAnimation(Input, DeltaSeconds);
}

void AHadiCharacter::UpdateStats(float DeltaSeconds)
{
	UHadiGameInstance* GameInstance = GetGameInstance<UHadiGameInstance>();
	if (!GameInstance)
	{
		return;
	}
	const float Meters = static_cast<float>(GetVelocity().Size2D()) * DeltaSeconds / 100.0f;
	if (bSwimming)
	{
		GameInstance->Profile.Stats.DistanceSwumMeters += Meters;
	}
	else if (GetCharacterMovement()->IsMovingOnGround())
	{
		GameInstance->Profile.Stats.DistanceOnFootMeters += Meters;
	}
}
