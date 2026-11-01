// HADI: VICE CITY
// Hadi - the playable protagonist. Third-person movement (walk / jog / sprint with stamina),
// jumping, swimming, ledge climbing, interaction, health, wardrobe and facial expressions.
#pragma once

#include "CoreMinimal.h"
#include "Core/HadiRig.h"
#include "GameFramework/Character.h"

#include "HadiCharacter.generated.h"

class AHadiVehicle;
class UCameraComponent;
class UHadiRigComponent;
class USpringArmComponent;

UCLASS()
class HADIVICECITY_API AHadiCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHadiCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void Landed(const FHitResult& Hit) override;

	// ---- Input (routed by AHadiPlayerController) ----
	void MoveInput(const FVector2D& Axis);
	void JumpPressed();
	void JumpReleased();
	void SetSprintHeld(bool bHeld);
	void ToggleWalk();
	void InteractPressed();
	void ZoomCamera(float Delta);

	// ---- Health ----
	void ApplyHadiDamage(float Amount);
	void Heal(float Amount);
	void SetHealthDirect(float NewHealth);
	void ReviveAt(const FVector& Location, float Yaw, float NewHealth);
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetStamina() const { return Stamina; }
	float GetMaxStamina() const { return MaxStamina; }
	bool IsDead() const { return bDead; }

	// ---- Wardrobe & face ----
	void SetOutfit(int32 OutfitIndex);
	int32 GetOutfit() const;
	void ShowExpression(hadi::Expression InExpression, float Duration);

	// ---- Vehicles ----
	void OnEnteredVehicle(AHadiVehicle* Vehicle, const FVector& SeatOffset);
	void OnExitedVehicle(const FVector& ExitLocation, float ExitYaw);
	AHadiVehicle* GetVehicle() const { return CurrentVehicle; }
	void SetRigHidden(bool bInHidden);

	// ---- Queries for HUD / game mode ----
	bool IsSwimming() const { return bSwimming; }
	bool IsMantling() const { return bMantling; }
	bool IsSprinting() const { return bSprinting; }
	bool IsWalkMode() const { return bWalkMode; }
	AActor* GetFocusedInteractable() const { return FocusedInteractable.Get(); }
	FText GetInteractionPromptText() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hadi|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hadi|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hadi|Body")
	TObjectPtr<UHadiRigComponent> Rig;

	// ---- Tuning ----
	UPROPERTY(EditAnywhere, Category = "Hadi|Movement")
	float WalkSpeed = 170.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Movement")
	float JogSpeed = 430.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Movement")
	float SprintSpeed = 690.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Movement")
	float SwimSpeed = 230.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Movement")
	float SwimSprintSpeed = 360.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Movement")
	float MantleMinHeight = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Movement")
	float MantleMaxHeight = 230.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Stamina")
	float StaminaDrainPerSecond = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Stamina")
	float StaminaRegenPerSecond = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Health")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Health")
	float RegenCap = 60.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Health")
	float RegenPerSecond = 4.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Health")
	float FallDamageSpeed = 1150.0f;

	UPROPERTY(EditAnywhere, Category = "Hadi|Health")
	float FallDamagePerSpeed = 0.09f;

protected:
	virtual void BeginPlay() override;

private:
	void UpdateStamina(float DeltaSeconds);
	void UpdateMovementSpeeds();
	void UpdateSwimming(float DeltaSeconds);
	void StartSwimming();
	void StopSwimming();
	bool TryMantle();
	void UpdateMantle(float DeltaSeconds);
	void UpdateInteraction(float DeltaSeconds);
	void UpdateHealth(float DeltaSeconds);
	void UpdateCamera(float DeltaSeconds);
	void UpdateAnimation(float DeltaSeconds);
	void UpdateStats(float DeltaSeconds);
	void Die();
	hadi::Expression ChooseExpression() const;

	UPROPERTY(Transient)
	TObjectPtr<AHadiVehicle> CurrentVehicle;

	TWeakObjectPtr<AActor> FocusedInteractable;

	float Health = 100.0f;
	float Stamina = 100.0f;
	float TimeSinceDamage = 100.0f;
	float InteractionTimer = 0.0f;
	float WaterLevel = -60.0f;
	float DesiredArmLength = 340.0f;
	float LastYaw = 0.0f;

	bool bSprintHeld = false;
	bool bSprinting = false;
	bool bStaminaExhausted = false;
	bool bWalkMode = false;
	bool bSwimming = false;
	bool bDead = false;

	bool bMantling = false;
	FVector MantleStart = FVector::ZeroVector;
	FVector MantleTarget = FVector::ZeroVector;
	float MantleTime = 0.0f;
	float MantleDuration = 0.6f;

	hadi::Expression Expression = hadi::Expression::Neutral;
	float ExpressionTimer = 0.0f;
};
