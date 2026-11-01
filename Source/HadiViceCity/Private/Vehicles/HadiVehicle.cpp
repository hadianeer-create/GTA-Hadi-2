// HADI: VICE CITY
#include "Vehicles/HadiVehicle.h"

#include "Characters/HadiCharacter.h"
#include "Game/HadiGameInstance.h"
#include "Game/HadiGameMode.h"
#include "World/HadiCoreBridge.h"
#include "World/HadiMaterialSubsystem.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

#define LOCTEXT_NAMESPACE "HadiVehicle"

AHadiVehicle::AHadiVehicle()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	AutoPossessAI = EAutoPossessAI::Disabled;
	DisplayName = LOCTEXT("VentoGT", "Vento GT");

	Chassis = CreateDefaultSubobject<UBoxComponent>(TEXT("Chassis"));
	Chassis->InitBoxExtent(FVector(225.0f, 92.0f, 30.0f));
	Chassis->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	Chassis->SetSimulatePhysics(true);
	Chassis->SetLinearDamping(0.02f);
	Chassis->SetAngularDamping(0.8f);
	Chassis->SetCanEverAffectNavigation(false);
	RootComponent = Chassis;

	CabinCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CabinCollision"));
	CabinCollision->SetupAttachment(Chassis);
	CabinCollision->InitBoxExtent(FVector(80.0f, 76.0f, 32.0f));
	CabinCollision->SetRelativeLocation(FVector(-40.0f, 0.0f, 58.0f));
	CabinCollision->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(Chassis);
	CameraArm->TargetArmLength = DesiredArmLength;
	CameraArm->TargetOffset = FVector(0.0f, 0.0f, 120.0f);
	CameraArm->bUsePawnControlRotation = true;
	CameraArm->bInheritRoll = false;
	CameraArm->bEnableCameraLag = true;
	CameraArm->CameraLagSpeed = 9.0f;
	CameraArm->ProbeSize = 16.0f;

	ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	ChaseCamera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	ChaseCamera->SetFieldOfView(85.0f);

	InteriorCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InteriorCamera"));
	InteriorCamera->SetupAttachment(Chassis);
	InteriorCamera->SetRelativeLocation(FVector(-20.0f, -40.0f, 74.0f));
	InteriorCamera->SetRelativeRotation(FRotator(-4.0f, 0.0f, 0.0f));
	InteriorCamera->SetFieldOfView(90.0f);
	InteriorCamera->SetAutoActivate(false);

	// Wheel layout: front-left, front-right, rear-left, rear-right. Rear-wheel drive.
	const FVector Mounts[4] = {FVector(138.0f, -80.0f, 0.0f), FVector(138.0f, 80.0f, 0.0f), FVector(-138.0f, -80.0f, 0.0f), FVector(-138.0f, 80.0f, 0.0f)};
	Wheels.SetNum(4);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		Wheels[Index].LocalMount = Mounts[Index];
		Wheels[Index].bFront = Index < 2;
		Wheels[Index].bDriven = Index >= 2;
	}
}

void AHadiVehicle::BeginPlay()
{
	Super::BeginPlay();
	Chassis->SetMassOverrideInKg(NAME_None, MassKg, true);
	Chassis->SetCenterOfMass(FVector(0.0f, 0.0f, -18.0f));
	BuildBody();
	if (UHadiInteractionSubsystem* Interactions = GetWorld()->GetSubsystem<UHadiInteractionSubsystem>())
	{
		Interactions->Register(this);
	}
}

void AHadiVehicle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UHadiInteractionSubsystem* Interactions = GetWorld()->GetSubsystem<UHadiInteractionSubsystem>())
	{
		Interactions->Unregister(this);
	}
	Super::EndPlay(EndPlayReason);
}

UStaticMeshComponent* AHadiVehicle::AddPart(USceneComponent* Parent, hadi::Shape Shape, hadi::Mat Mat, const hadi::Color& Color, const FVector& Location,
	const FVector& Size, const FRotator& Rotation)
{
	UHadiMaterialSubsystem* Materials = GetWorld()->GetSubsystem<UHadiMaterialSubsystem>();
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	Part->SetupAttachment(Parent);
	Part->SetStaticMesh(Materials->GetShapeMesh(Shape));
	Part->SetMaterial(0, Materials->GetMaterial(Mat, Color));
	Part->SetRelativeLocation(Location);
	Part->SetRelativeRotation(Rotation);
	Part->SetRelativeScale3D(Size / 100.0);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetGenerateOverlapEvents(false);
	Part->SetCanEverAffectNavigation(false);
	Part->RegisterComponent();
	AddInstanceComponent(Part);
	return Part;
}

void AHadiVehicle::BuildBody()
{
	if (bBuilt || !GetWorld()->GetSubsystem<UHadiMaterialSubsystem>())
	{
		return;
	}
	bBuilt = true;
	using hadi::Mat;
	using hadi::Shape;
	namespace pal = hadi::pal;
	USceneComponent* Root = Chassis;
	const hadi::Color Paint = PaintColor;
	const hadi::Color Trim = pal::Charcoal;
	const hadi::Color Tint = pal::GlassDark;

	// Painted shell.
	PaintedParts.Add(AddPart(Root, Shape::Cube, Mat::Glossy, Paint, FVector(0, 0, -2), FVector(450, 186, 50)));
	PaintedParts.Add(AddPart(Root, Shape::Cube, Mat::Glossy, Paint, FVector(150, 0, 25), FVector(150, 178, 10), FRotator(-4, 0, 0)));
	PaintedParts.Add(AddPart(Root, Shape::Cube, Mat::Glossy, Paint, FVector(-165, 0, 27), FVector(110, 178, 12)));
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		PaintedParts.Add(AddPart(Root, Shape::Cube, Mat::Glossy, Paint, FVector(135, 89.0 * Side, 14), FVector(110, 16, 26)));
		PaintedParts.Add(AddPart(Root, Shape::Cube, Mat::Glossy, Paint, FVector(-135, 89.0 * Side, 16), FVector(116, 20, 30)));
		PaintedParts.Add(AddPart(Root, Shape::Cube, Mat::Glossy, Paint, FVector(48, 90.0 * Side, 44), FVector(14, 12, 10)));
		AddPart(Root, Shape::Cube, Mat::Matte, Trim, FVector(0, 93.0 * Side, -22), FVector(240, 6, 10));
		AddPart(Root, Shape::Cube, Mat::Light, pal::NeonWhite, FVector(220, 70.0 * Side, 10), FVector(8, 34, 10));
		AddPart(Root, Shape::Cube, Mat::Light, pal::NeonRed, FVector(-226, 66.0 * Side, 14), FVector(6, 42, 10));
		AddPart(Root, Shape::Cylinder, Mat::Metal, pal::Metal, FVector(-229, 48.0 * Side, -20), FVector(10, 10, 16), FRotator(90, 0, 0));
		AddPart(Root, Shape::Cube, Mat::Glossy, Trim, FVector(-205, 58.0 * Side, 41), FVector(10, 6, 22));
	}
	PaintedParts.Add(AddPart(Root, Shape::Cube, Mat::Glossy, Paint, FVector(-40, 0, 90), FVector(140, 150, 6)));
	AddPart(Root, Shape::Cube, Mat::Glossy, Trim, FVector(-205, 0, 53), FVector(30, 172, 4));
	AddPart(Root, Shape::Cube, Mat::Matte, pal::Black, FVector(226, 0, 0), FVector(6, 110, 22));
	AddPart(Root, Shape::Cube, Mat::Matte, Trim, FVector(222, 0, -20), FVector(14, 188, 16));
	AddPart(Root, Shape::Cube, Mat::Matte, Trim, FVector(-222, 0, -18), FVector(14, 188, 18));

	// Glasshouse (see-through so the driver is visible).
	AddPart(Root, Shape::Cube, Mat::GlassClear, Tint, FVector(-40, 0, 56), FVector(140, 154, 62));
	AddPart(Root, Shape::Cube, Mat::GlassClear, Tint, FVector(55, 0, 58), FVector(76, 152, 3), FRotator(-48.7, 0, 0));
	AddPart(Root, Shape::Cube, Mat::GlassClear, Tint, FVector(-130, 0, 60), FVector(66, 152, 3), FRotator(53, 0, 0));

	// Interior.
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		AddPart(Root, Shape::Cube, Mat::Matte, pal::Black, FVector(-28, 40.0 * Side, -12), FVector(55, 50, 14));
		AddPart(Root, Shape::Cube, Mat::Matte, pal::Black, FVector(-60, 40.0 * Side, 16), FVector(12, 48, 62), FRotator(-14, 0, 0));
	}
	AddPart(Root, Shape::Cube, Mat::Matte, pal::Charcoal, FVector(58, 0, 30), FVector(30, 150, 18));
	AddPart(Root, Shape::Cylinder, Mat::Matte, pal::Black, FVector(30, -40, 36), FVector(36, 36, 4), FRotator(-65, 0, 0));

	// Wheels: pivot (suspension + steering) -> spin -> tyre / rim / spoke.
	for (FHadiWheel& Wheel : Wheels)
	{
		USceneComponent* Pivot = NewObject<USceneComponent>(this);
		Pivot->SetupAttachment(Root);
		Pivot->SetRelativeLocation(Wheel.LocalMount - FVector(0, 0, RestLength));
		Pivot->RegisterComponent();
		AddInstanceComponent(Pivot);
		USceneComponent* Spin = NewObject<USceneComponent>(this);
		Spin->SetupAttachment(Pivot);
		Spin->RegisterComponent();
		AddInstanceComponent(Spin);
		Wheel.Pivot = Pivot;
		Wheel.Spin = Spin;
		const float Diameter = WheelRadius * 2.0f;
		AddPart(Spin, Shape::Cylinder, Mat::Matte, pal::Rubber, FVector::ZeroVector, FVector(Diameter, Diameter, 26), FRotator(0, 0, 90));
		AddPart(Spin, Shape::Cylinder, Mat::Metal, pal::Metal, FVector::ZeroVector, FVector(Diameter * 0.64f, Diameter * 0.64f, 27.5f), FRotator(0, 0, 90));
		AddPart(Spin, Shape::Cube, Mat::Metal, pal::DarkMetal, FVector::ZeroVector, FVector(Diameter * 0.6f, 28.5f, 7), FRotator::ZeroRotator);
	}

	// Headlights (switched on automatically at night).
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		USpotLightComponent* Light = NewObject<USpotLightComponent>(this);
		Light->SetupAttachment(Root);
		Light->SetRelativeLocation(FVector(228, 70.0 * Side, 10));
		Light->SetRelativeRotation(FRotator(-5, 0, 0));
		Light->SetMobility(EComponentMobility::Movable);
		Light->SetIntensityUnits(ELightUnits::Candelas);
		Light->SetIntensity(900.0f);
		Light->SetAttenuationRadius(3500.0f);
		Light->SetInnerConeAngle(16.0f);
		Light->SetOuterConeAngle(32.0f);
		Light->SetLightColor(FLinearColor(1.0f, 0.93f, 0.82f));
		Light->SetCastShadows(false);
		Light->SetVisibility(false);
		Light->RegisterComponent();
		AddInstanceComponent(Light);
		Headlights.Add(Light);
	}
	SetPaintColor(PaintColor);
}

void AHadiVehicle::SetPaintColor(const hadi::Color& Color)
{
	PaintColor = Color;
	UHadiMaterialSubsystem* Materials = GetWorld() ? GetWorld()->GetSubsystem<UHadiMaterialSubsystem>() : nullptr;
	if (!Materials)
	{
		return;
	}
	UMaterialInterface* Material = Materials->GetMaterial(hadi::Mat::Glossy, Color);
	for (UStaticMeshComponent* Part : PaintedParts)
	{
		if (Part)
		{
			Part->SetMaterial(0, Material);
		}
	}
}

// ------------------------------------------------------------------------------ simulation
void AHadiVehicle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bBuilt || DeltaSeconds <= 0.0f)
	{
		return;
	}
	SimulateWheels(DeltaSeconds);
	UpdateVisuals(DeltaSeconds);
	if (Controller)
	{
		UpdateCamera(DeltaSeconds);
	}
	if (Driver)
	{
		if (UHadiGameInstance* GameInstance = GetGameInstance<UHadiGameInstance>())
		{
			GameInstance->Profile.Stats.DistanceDrivenMeters += FMath::Abs(ForwardSpeed) * DeltaSeconds / 100.0f;
			GameInstance->Profile.Stats.TopSpeedKmh = FMath::Max(GameInstance->Profile.Stats.TopSpeedKmh, GetSpeedKmh());
		}
	}
}

void AHadiVehicle::SimulateWheels(float DeltaSeconds)
{
	if (!Chassis->IsSimulatingPhysics())
	{
		return;
	}
	UWorld* World = GetWorld();
	const FTransform& Transform = Chassis->GetComponentTransform();
	const FVector Up = Transform.GetUnitAxis(EAxis::Z);
	const FVector Forward = Transform.GetUnitAxis(EAxis::X);
	const FVector Velocity = Chassis->GetPhysicsLinearVelocity();
	ForwardSpeed = static_cast<float>(FVector::DotProduct(Velocity, Forward));
	const float AbsSpeed = FMath::Abs(ForwardSpeed);

	// Speed-sensitive steering.
	const float MaxSteer = FMath::Lerp(MaxSteerAngle, HighSpeedSteerAngle, FMath::Clamp(AbsSpeed / 3000.0f, 0.0f, 1.0f));
	CurrentSteer = FMath::FInterpTo(CurrentSteer, Driver ? SteerInput : 0.0f, DeltaSeconds, 6.0f);
	SteerDegrees = CurrentSteer * MaxSteer;

	// Throttle / brake / reverse.
	float Drive = 0.0f;
	float Brake = 0.0f;
	if (Driver)
	{
		if (ThrottleInput > 0.01f)
		{
			if (ForwardSpeed < -150.0f)
			{
				Brake = ThrottleInput;
			}
			else
			{
				Drive = ThrottleInput;
			}
		}
		if (BrakeInput > 0.01f)
		{
			if (ForwardSpeed > 150.0f)
			{
				Brake = FMath::Max(Brake, BrakeInput);
			}
			else
			{
				Drive -= BrakeInput * 0.55f;
			}
		}
	}
	else
	{
		Brake = 1.0f; // parked
	}
	const float TopSpeed = TopSpeedKmh / 0.036f;
	const float EngineCurve = AbsSpeed > TopSpeed ? 0.0f : 0.25f + 0.75f * FMath::Clamp(1.0f - AbsSpeed / TopSpeed, 0.0f, 1.0f);
	const float DriveForce = Drive * EngineForce * EngineCurve;

	const float Mass = Chassis->GetMass();
	const float WheelMass = Mass * 0.25f;
	const float Tau = FMath::Max(0.08f, DeltaSeconds * 1.5f);
	const float RayLength = RestLength + WheelRadius;
	FCollisionQueryParams Params(FName(TEXT("HadiWheel")), false, this);
	if (Driver)
	{
		Params.AddIgnoredActor(Driver);
	}

	FHitResult Hits[4];
	FVector Mounts[4];
	int32 DrivenGrounded = 0;
	for (int32 Index = 0; Index < Wheels.Num() && Index < 4; ++Index)
	{
		FHadiWheel& Wheel = Wheels[Index];
		Mounts[Index] = Transform.TransformPosition(Wheel.LocalMount);
		Wheel.LastCompression = Wheel.Compression;
		Wheel.bGrounded = World->LineTraceSingleByChannel(Hits[Index], Mounts[Index], Mounts[Index] - Up * RayLength, ECC_Visibility, Params);
		Wheel.Compression = Wheel.bGrounded ? RayLength - static_cast<float>(Hits[Index].Distance) : 0.0f;
		if (Wheel.bGrounded && Wheel.bDriven)
		{
			++DrivenGrounded;
		}
	}

	// Anti-roll bars (front pair 0/1, rear pair 2/3).
	for (int32 Axle = 0; Axle < 2; ++Axle)
	{
		const int32 Left = Axle * 2;
		const int32 Right = Left + 1;
		const float RollForce = (Wheels[Left].Compression - Wheels[Right].Compression) * AntiRollStiffness;
		if (Wheels[Left].bGrounded)
		{
			Chassis->AddForceAtLocation(-Up * RollForce, Mounts[Left]);
		}
		if (Wheels[Right].bGrounded)
		{
			Chassis->AddForceAtLocation(Up * RollForce, Mounts[Right]);
		}
	}

	const float Grip = TyreGrip * WeatherGrip;
	for (int32 Index = 0; Index < Wheels.Num() && Index < 4; ++Index)
	{
		const FHadiWheel& Wheel = Wheels[Index];
		if (!Wheel.bGrounded)
		{
			continue;
		}
		const FHitResult& Hit = Hits[Index];

		// Suspension.
		const float CompressionSpeed = (Wheel.Compression - Wheel.LastCompression) / DeltaSeconds;
		const float Load = FMath::Max(0.0f, SpringStiffness * Wheel.Compression + DamperStrength * CompressionSpeed);
		Chassis->AddForceAtLocation(Up * Load, Mounts[Index]);

		// Tyre forces in the contact plane.
		const FVector Normal = Hit.ImpactNormal;
		FVector WheelForward = Wheel.bFront ? Forward.RotateAngleAxis(SteerDegrees, Up) : Forward;
		WheelForward = FVector::VectorPlaneProject(WheelForward, Normal).GetSafeNormal();
		const FVector WheelRight = FVector::CrossProduct(Normal, WheelForward).GetSafeNormal();
		const FVector PointVelocity = Chassis->GetPhysicsLinearVelocityAtPoint(Hit.ImpactPoint);
		const float LongSpeed = static_cast<float>(FVector::DotProduct(PointVelocity, WheelForward));
		const float LatSpeed = static_cast<float>(FVector::DotProduct(PointVelocity, WheelRight));

		const float LatGrip = (!Wheel.bFront && bHandbrake) ? Grip * 0.45f : Grip;
		float Lateral = FMath::Clamp(-LatSpeed * WheelMass / Tau, -LatGrip * Load, LatGrip * Load);
		float Longitudinal = -LongSpeed * 12.0f; // rolling resistance
		if (Wheel.bDriven && DrivenGrounded > 0)
		{
			Longitudinal += DriveForce / static_cast<float>(DrivenGrounded);
		}
		if (Brake > 0.0f)
		{
			const float MaxBrake = BrakeForce * 0.25f * Brake;
			Longitudinal += FMath::Clamp(-LongSpeed * WheelMass / Tau, -MaxBrake, MaxBrake);
		}
		if (bHandbrake && !Wheel.bFront)
		{
			const float MaxBrake = BrakeForce * 0.3f;
			Longitudinal += FMath::Clamp(-LongSpeed * WheelMass / Tau, -MaxBrake, MaxBrake);
		}
		// Friction circle.
		const float Total = FMath::Sqrt(Longitudinal * Longitudinal + Lateral * Lateral);
		const float Cap = Grip * Load;
		if (Total > Cap && Total > UE_KINDA_SMALL_NUMBER)
		{
			const float Scale = Cap / Total;
			Longitudinal *= Scale;
			Lateral *= Scale;
		}
		Chassis->AddForceAtLocation(WheelForward * Longitudinal + WheelRight * Lateral, Hit.ImpactPoint + Up * 20.0f);
	}

	// Aerodynamics.
	const float SpeedSq = static_cast<float>(Velocity.SizeSquared());
	Chassis->AddForce(-Velocity * Velocity.Size() * DragCoefficient);
	Chassis->AddForce(-Up * SpeedSq * DownforceCoefficient);

	// Self-righting when stuck on the roof or side.
	if (Driver && Up.Z < 0.3 && AbsSpeed < 200.0f)
	{
		FlipTimer += DeltaSeconds;
		if (FlipTimer > 2.5f)
		{
			FlipTimer = 0.0f;
			SetActorLocationAndRotation(GetActorLocation() + FVector(0, 0, 120), FRotator(0.0f, GetActorRotation().Yaw, 0.0f), false, nullptr,
				ETeleportType::TeleportPhysics);
			Chassis->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Chassis->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		}
	}
	else
	{
		FlipTimer = 0.0f;
	}
}

void AHadiVehicle::UpdateVisuals(float DeltaSeconds)
{
	const float RayLength = RestLength + WheelRadius;
	for (FHadiWheel& Wheel : Wheels)
	{
		if (!Wheel.Pivot || !Wheel.Spin)
		{
			continue;
		}
		const float CenterDrop = Wheel.bGrounded ? (RayLength - Wheel.Compression) - WheelRadius : RestLength;
		Wheel.VisualOffset = FMath::FInterpTo(Wheel.VisualOffset, CenterDrop, DeltaSeconds, 30.0f);
		Wheel.Pivot->SetRelativeLocationAndRotation(Wheel.LocalMount - FVector(0, 0, Wheel.VisualOffset), FRotator(0.0f, Wheel.bFront ? SteerDegrees : 0.0f, 0.0f));
		const float SpinRate = (bHandbrake && !Wheel.bFront) ? 0.0f : ForwardSpeed / WheelRadius;
		Wheel.SpinDegrees = FMath::Fmod(Wheel.SpinDegrees + FMath::RadiansToDegrees(SpinRate) * DeltaSeconds, 360.0f);
		Wheel.Spin->SetRelativeRotation(FRotator(-Wheel.SpinDegrees, 0.0f, 0.0f));
	}

	// Headlights follow the day/night cycle.
	const UHadiMaterialSubsystem* Materials = GetWorld()->GetSubsystem<UHadiMaterialSubsystem>();
	const bool bNight = Materials && Materials->GetNightFactor() > 0.45f;
	for (USpotLightComponent* Light : Headlights)
	{
		if (Light && Light->IsVisible() != bNight)
		{
			Light->SetVisibility(bNight);
		}
	}
}

void AHadiVehicle::UpdateCamera(float DeltaSeconds)
{
	TimeSinceLook += DeltaSeconds;
	CameraArm->TargetArmLength = FMath::FInterpTo(CameraArm->TargetArmLength, DesiredArmLength + FMath::Abs(ForwardSpeed) * 0.03f, DeltaSeconds, 3.0f);
	// GTA-style recentre behind the car after the player stops looking around.
	if (!bFirstPerson && TimeSinceLook > 1.2f && FMath::Abs(ForwardSpeed) > 300.0f)
	{
		const FRotator Current = Controller->GetControlRotation();
		const float TargetYaw = static_cast<float>(GetActorRotation().Yaw) + (ForwardSpeed < 0.0f ? 180.0f : 0.0f);
		const FRotator Target(-12.0f, TargetYaw, 0.0f);
		Controller->SetControlRotation(FMath::RInterpTo(Current, Target, DeltaSeconds, 2.5f));
	}
}

void AHadiVehicle::ToggleCamera()
{
	bFirstPerson = !bFirstPerson;
	ChaseCamera->SetActive(!bFirstPerson);
	InteriorCamera->SetActive(bFirstPerson);
	if (Driver)
	{
		Driver->SetRigHidden(bFirstPerson);
	}
}

void AHadiVehicle::ZoomCamera(float Delta)
{
	DesiredArmLength = FMath::Clamp(DesiredArmLength - Delta * 60.0f, 380.0f, 1100.0f);
}

float AHadiVehicle::GetSpeedKmh() const
{
	return FMath::Abs(ForwardSpeed) * 0.036f;
}

int32 AHadiVehicle::GetGear() const
{
	if (ForwardSpeed < -50.0f)
	{
		return -1;
	}
	const float Kmh = GetSpeedKmh();
	const float Bands[5] = {25.0f, 55.0f, 90.0f, 130.0f, 170.0f};
	int32 Gear = 1;
	for (const float Band : Bands)
	{
		if (Kmh > Band)
		{
			++Gear;
		}
	}
	return Gear;
}

float AHadiVehicle::GetRpmFraction() const
{
	const float Kmh = GetSpeedKmh();
	const float Bands[6] = {0.0f, 25.0f, 55.0f, 90.0f, 130.0f, 170.0f};
	const int32 Gear = FMath::Clamp(GetGear(), 1, 6);
	const float Low = Bands[Gear - 1];
	const float High = Gear < 6 ? Bands[Gear] : TopSpeedKmh;
	const float InGear = FMath::Clamp((Kmh - Low) / FMath::Max(1.0f, High - Low), 0.0f, 1.0f);
	return FMath::Clamp(0.18f + 0.8f * InGear + (ThrottleInput > 0.0f ? 0.04f : 0.0f), 0.0f, 1.0f);
}

// ---------------------------------------------------------------------------- enter / exit
FText AHadiVehicle::GetInteractionPrompt(const AHadiCharacter* /*Character*/) const
{
	return FText::Format(LOCTEXT("EnterPrompt", "Enter {0}"), DisplayName);
}

bool AHadiVehicle::CanInteract(const AHadiCharacter* Character) const
{
	return Driver == nullptr && Character && Character->GetVehicle() == nullptr && !Character->IsSwimming();
}

void AHadiVehicle::Interact(AHadiCharacter* Character)
{
	EnterVehicle(Character);
}

FVector AHadiVehicle::GetInteractionLocation() const
{
	return GetActorLocation();
}

bool AHadiVehicle::EnterVehicle(AHadiCharacter* Character)
{
	if (!Character || Driver)
	{
		return false;
	}
	AController* DriverController = Character->GetController();
	Driver = Character;
	bHandbrake = false;
	Character->OnEnteredVehicle(this, SeatOffset);
	if (DriverController)
	{
		DriverController->Possess(this);
		DriverController->SetControlRotation(FRotator(-12.0f, GetActorRotation().Yaw, 0.0f));
		if (APlayerController* PlayerController = Cast<APlayerController>(DriverController))
		{
			PlayerController->SetViewTarget(Character);
			PlayerController->SetViewTargetWithBlend(this, 0.6f, VTBlend_EaseInOut, 2.0f);
		}
	}
	Chassis->WakeAllRigidBodies();
	TimeSinceLook = 10.0f;
	if (UHadiGameInstance* GameInstance = GetGameInstance<UHadiGameInstance>())
	{
		GameInstance->Profile.Stats.VehiclesEntered++;
	}
	if (AHadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AHadiGameMode>())
	{
		GameMode->ReportEvent(AHadiGameMode::EventEnteredVehicle);
	}
	return true;
}

bool AHadiVehicle::FindExitLocation(FVector& OutLocation) const
{
	const FVector Base = GetActorLocation();
	const FVector Right = GetActorRightVector();
	const FVector Forward = GetActorForwardVector();
	const FVector Candidates[4] = {Base - Right * 200.0f, Base + Right * 200.0f, Base - Forward * 340.0f, Base + FVector(0, 0, 180)};
	FCollisionQueryParams Params(FName(TEXT("HadiVehicleExit")), false, this);
	if (Driver)
	{
		Params.AddIgnoredActor(Driver);
	}
	for (const FVector& Candidate : Candidates)
	{
		const FVector Test = Candidate + FVector(0.0f, 0.0f, 55.0f);
		if (!GetWorld()->OverlapBlockingTestByChannel(Test, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(34.0f, 90.0f), Params))
		{
			OutLocation = Test;
			return true;
		}
	}
	return false;
}

void AHadiVehicle::RequestExit()
{
	if (!Driver)
	{
		return;
	}
	AHadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AHadiGameMode>();
	if (GetSpeedKmh() > 30.0f)
	{
		if (GameMode)
		{
			GameMode->Notify(LOCTEXT("SlowDown", "Slow down to get out."));
		}
		return;
	}
	FVector ExitLocation;
	if (!FindExitLocation(ExitLocation))
	{
		if (GameMode)
		{
			GameMode->Notify(LOCTEXT("NoRoom", "There's no room to get out here."));
		}
		return;
	}
	if (bFirstPerson)
	{
		ToggleCamera();
	}
	AHadiCharacter* Character = Driver;
	AController* DriverController = Controller;
	Driver = nullptr;
	ThrottleInput = 0.0f;
	BrakeInput = 0.0f;
	SteerInput = 0.0f;
	bHandbrake = true;
	const float Yaw = static_cast<float>(GetActorRotation().Yaw);
	Character->OnExitedVehicle(ExitLocation, Yaw);
	if (DriverController)
	{
		DriverController->Possess(Character);
		DriverController->SetControlRotation(FRotator(-10.0f, Yaw, 0.0f));
		if (APlayerController* PlayerController = Cast<APlayerController>(DriverController))
		{
			PlayerController->SetViewTarget(this);
			PlayerController->SetViewTargetWithBlend(Character, 0.5f, VTBlend_EaseInOut, 2.0f);
		}
	}
}

#undef LOCTEXT_NAMESPACE
