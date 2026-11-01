// HADI: VICE CITY
// Drivable car built on a Chaos rigid body with ray-cast suspension and a friction-circle tyre
// model (spring/damper per wheel, anti-roll bars, engine curve, brakes, handbrake drift, drag,
// downforce, weather grip). The body is assembled from primitives so it needs no imported assets.
#pragma once

#include "CoreMinimal.h"
#include "Core/HadiTypes.h"
#include "GameFramework/Pawn.h"
#include "Gameplay/HadiInteractable.h"

#include "HadiVehicle.generated.h"

class AHadiCharacter;
class UBoxComponent;
class UCameraComponent;
class USpotLightComponent;
class USpringArmComponent;
class UStaticMeshComponent;

USTRUCT()
struct FHadiWheel
{
	GENERATED_BODY()

	FVector LocalMount = FVector::ZeroVector;
	bool bFront = false;
	bool bDriven = false;
	bool bGrounded = false;
	float Compression = 0.0f;
	float LastCompression = 0.0f;
	float SpinDegrees = 0.0f;
	float VisualOffset = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> Pivot;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> Spin;
};

UCLASS()
class HADIVICECITY_API AHadiVehicle : public APawn, public IHadiInteractable
{
	GENERATED_BODY()

public:
	AHadiVehicle();

	virtual void Tick(float DeltaSeconds) override;

	// IHadiInteractable
	virtual FText GetInteractionPrompt(const AHadiCharacter* Character) const override;
	virtual bool CanInteract(const AHadiCharacter* Character) const override;
	virtual void Interact(AHadiCharacter* Character) override;
	virtual FVector GetInteractionLocation() const override;
	virtual float GetInteractionRadius() const override { return 330.0f; }

	// Driver
	bool EnterVehicle(AHadiCharacter* Character);
	void RequestExit();
	AHadiCharacter* GetDriver() const { return Driver; }

	// Controls (routed by AHadiPlayerController)
	void SetThrottle(float Value) { ThrottleInput = FMath::Clamp(Value, 0.0f, 1.0f); }
	void SetBrake(float Value) { BrakeInput = FMath::Clamp(Value, 0.0f, 1.0f); }
	void SetSteer(float Value) { SteerInput = FMath::Clamp(Value, -1.0f, 1.0f); }
	void SetHandbrake(bool bOn) { bHandbrake = bOn; }
	void ToggleCamera();
	void NotifyLookInput() { TimeSinceLook = 0.0f; }
	void ZoomCamera(float Delta);

	// HUD
	float GetSpeedKmh() const;
	float GetSteer() const { return CurrentSteer; }
	int32 GetGear() const;
	float GetRpmFraction() const;
	bool IsFirstPerson() const { return bFirstPerson; }

	void SetPaintColor(const hadi::Color& Color);
	void SetGripMultiplier(float Multiplier) { WeatherGrip = Multiplier; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UBoxComponent> Chassis;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UBoxComponent> CabinCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<USpringArmComponent> CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UCameraComponent> ChaseCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<UCameraComponent> InteriorCamera;

	UPROPERTY(EditAnywhere, Category = "Vehicle")
	FText DisplayName;

	// ---- Handling ----
	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float MassKg = 1380.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float SpringStiffness = 34000.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float DamperStrength = 2800.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float RestLength = 22.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float WheelRadius = 33.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float AntiRollStiffness = 16000.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float EngineForce = 780000.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float BrakeForce = 1500000.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float TopSpeedKmh = 215.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float TyreGrip = 1.15f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float MaxSteerAngle = 34.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float HighSpeedSteerAngle = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float DragCoefficient = 0.0095f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Handling")
	float DownforceCoefficient = 0.004f;

	// Where the driver's capsule centre sits relative to the chassis.
	UPROPERTY(EditAnywhere, Category = "Vehicle")
	FVector SeatOffset = FVector(-28.0f, -40.0f, 44.0f);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BuildBody();
	UStaticMeshComponent* AddPart(USceneComponent* Parent, hadi::Shape Shape, hadi::Mat Mat, const hadi::Color& Color, const FVector& Location, const FVector& Size,
		const FRotator& Rotation = FRotator::ZeroRotator);
	void SimulateWheels(float DeltaSeconds);
	void UpdateVisuals(float DeltaSeconds);
	void UpdateCamera(float DeltaSeconds);
	bool FindExitLocation(FVector& OutLocation) const;

	UPROPERTY(Transient)
	TObjectPtr<AHadiCharacter> Driver;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> PaintedParts;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USpotLightComponent>> Headlights;

	UPROPERTY()
	TArray<FHadiWheel> Wheels;

	hadi::Color PaintColor = hadi::pal::Coral;
	float ThrottleInput = 0.0f;
	float BrakeInput = 0.0f;
	float SteerInput = 0.0f;
	float CurrentSteer = 0.0f;
	bool bHandbrake = false;
	bool bFirstPerson = false;
	bool bBuilt = false;
	float TimeSinceLook = 10.0f;
	float DesiredArmLength = 620.0f;
	float WeatherGrip = 1.0f;
	float ForwardSpeed = 0.0f; // cm/s along the chassis
	float SteerDegrees = 0.0f;
	float FlipTimer = 0.0f;
};
