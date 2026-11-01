// HADI: VICE CITY
#include "Gameplay/HadiCollectible.h"

#include "Game/HadiGameMode.h"
#include "Vehicles/HadiVehicle.h"
#include "World/HadiMaterialSubsystem.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

namespace HadiCollectibleLocal
{
UStaticMeshComponent* AddShape(AActor* Owner, USceneComponent* Parent, UHadiMaterialSubsystem* Materials, hadi::Shape Shape, hadi::Mat Mat, const hadi::Color& Color,
	const FVector& Location, const FVector& Size, const FRotator& Rotation)
{
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(Owner);
	Part->SetupAttachment(Parent);
	Part->SetStaticMesh(Materials->GetShapeMesh(Shape));
	Part->SetMaterial(0, Materials->GetMaterial(Mat, Color));
	Part->SetRelativeLocation(Location);
	Part->SetRelativeRotation(Rotation);
	Part->SetRelativeScale3D(Size / 100.0);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetCastShadow(false);
	Part->RegisterComponent();
	Owner->AddInstanceComponent(Part);
	return Part;
}
} // namespace HadiCollectibleLocal

AHadiCollectible::AHadiCollectible()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Body = CreateDefaultSubobject<USceneComponent>(TEXT("Body"));
	Body->SetupAttachment(Root);
	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(Root);
	Glow->SetIntensityUnits(ELightUnits::Candelas);
	Glow->SetIntensity(25.0f);
	Glow->SetAttenuationRadius(450.0f);
	Glow->SetLightColor(FLinearColor(1.0f, 0.15f, 0.6f));
	Glow->SetCastShadows(false);
}

void AHadiCollectible::BeginPlay()
{
	Super::BeginPlay();
	BuildFlamingo();
}

void AHadiCollectible::BuildFlamingo()
{
	UHadiMaterialSubsystem* Materials = GetWorld()->GetSubsystem<UHadiMaterialSubsystem>();
	if (bBuilt || !Materials)
	{
		return;
	}
	bBuilt = true;
	using hadi::Mat;
	using hadi::Shape;
	const hadi::Color Pink = hadi::pal::NeonPink;
	using HadiCollectibleLocal::AddShape;
	// A stylised neon flamingo standing on one leg.
	AddShape(this, Body, Materials, Shape::Sphere, Mat::Neon, Pink, FVector(0, 0, 10), FVector(46, 26, 28), FRotator(8, 0, 0));
	AddShape(this, Body, Materials, Shape::Cylinder, Mat::Neon, Pink, FVector(16, 0, 32), FVector(5, 5, 36), FRotator(-25, 0, 0));
	AddShape(this, Body, Materials, Shape::Sphere, Mat::Neon, Pink, FVector(22, 0, 52), FVector(12, 10, 11), FRotator::ZeroRotator);
	AddShape(this, Body, Materials, Shape::Cone, Mat::Neon, hadi::pal::NeonWhite, FVector(31, 0, 49), FVector(5, 5, 12), FRotator(-70, 0, 0));
	AddShape(this, Body, Materials, Shape::Cylinder, Mat::Neon, Pink, FVector(0, 0, -22), FVector(2.5f, 2.5f, 42), FRotator::ZeroRotator);
	AddShape(this, Body, Materials, Shape::Cylinder, Mat::Neon, Pink, FVector(-4, 0, -10), FVector(2.5f, 2.5f, 22), FRotator(60, 0, 0));
	AddShape(this, Body, Materials, Shape::Cylinder, Mat::Neon, Pink, FVector(0, 0, -44), FVector(30, 30, 2), FRotator::ZeroRotator);
}

void AHadiCollectible::SetCollected(bool bInCollected)
{
	bCollected = bInCollected;
	SetActorHiddenInGame(bCollected);
	SetActorTickEnabled(!bCollected);
	Glow->SetVisibility(!bCollected);
}

void AHadiCollectible::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bCollected)
	{
		return;
	}
	Time += DeltaSeconds;
	Body->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 10.0f * FMath::Sin(Time * 2.0f)), FRotator(0.0f, Time * 60.0f, 0.0f));

	const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn)
	{
		return;
	}
	// Collect on foot, or by driving through it.
	const float Radius = Pawn->IsA<AHadiVehicle>() ? PickupRadius + 180.0f : PickupRadius;
	if (FVector::DistSquared(Pawn->GetActorLocation(), GetActorLocation()) < FMath::Square(Radius + 90.0f))
	{
		if (AHadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AHadiGameMode>())
		{
			GameMode->CollectFlamingo(CollectibleId);
		}
		SetCollected(true);
	}
}

// ------------------------------------------------------------------------- waypoint marker
AHadiWaypointMarker::AHadiWaypointMarker()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Column = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Column"));
	Column->SetupAttachment(Root);
	Column->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	Column->SetRelativeScale3D(FVector(1.6f, 1.6f, 2.4f));
	Column->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Column->SetCastShadow(false);
	Ring = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ring"));
	Ring->SetupAttachment(Root);
	Ring->SetRelativeLocation(FVector(0.0f, 0.0f, 3.0f));
	Ring->SetRelativeScale3D(FVector(1.9f, 1.9f, 0.04f));
	Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Ring->SetCastShadow(false);
}

void AHadiWaypointMarker::BeginPlay()
{
	Super::BeginPlay();
	if (UHadiMaterialSubsystem* Materials = GetWorld()->GetSubsystem<UHadiMaterialSubsystem>())
	{
		Column->SetStaticMesh(Materials->GetShapeMesh(hadi::Shape::Cylinder));
		Column->SetMaterial(0, Materials->GetMaterial(hadi::Mat::GlassClear, hadi::pal::LaneYellow));
		Ring->SetStaticMesh(Materials->GetShapeMesh(hadi::Shape::Cylinder));
		Ring->SetMaterial(0, Materials->GetMaterial(hadi::Mat::Neon, hadi::pal::LaneYellow));
	}
	HideMarker();
}

void AHadiWaypointMarker::ShowAt(const FVector& GroundLocation)
{
	SetActorLocation(GroundLocation);
	SetActorHiddenInGame(false);
}

void AHadiWaypointMarker::HideMarker()
{
	SetActorHiddenInGame(true);
}

void AHadiWaypointMarker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Time += DeltaSeconds;
	const float Pulse = 1.0f + 0.06f * FMath::Sin(Time * 3.0f);
	Ring->SetRelativeScale3D(FVector(1.9f * Pulse, 1.9f * Pulse, 0.04f));
}
