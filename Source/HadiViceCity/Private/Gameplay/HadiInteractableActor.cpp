// HADI: VICE CITY
#include "Gameplay/HadiInteractableActor.h"

#include "World/HadiMaterialSubsystem.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

namespace HadiInteractableLocal
{
hadi::Color ToCoreColor(const FLinearColor& Linear)
{
	const FColor Srgb = Linear.ToFColor(true);
	return hadi::Color{Srgb.R / 255.0f, Srgb.G / 255.0f, Srgb.B / 255.0f};
}
} // namespace HadiInteractableLocal

AHadiInteractableActor::AHadiInteractableActor()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Ring = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ring"));
	Ring->SetupAttachment(Root);
	Ring->SetRelativeLocation(FVector(0.0f, 0.0f, 4.0f));
	Ring->SetRelativeScale3D(FVector(1.6f, 1.6f, 0.03f));
	Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Ring->SetCastShadow(false);

	Icon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Icon"));
	Icon->SetupAttachment(Root);
	Icon->SetRelativeLocation(FVector(0.0f, 0.0f, 190.0f));
	Icon->SetRelativeScale3D(FVector(0.32f, 0.32f, 0.32f));
	Icon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Icon->SetCastShadow(false);
}

void AHadiInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	if (UHadiMaterialSubsystem* Materials = GetWorld()->GetSubsystem<UHadiMaterialSubsystem>())
	{
		Ring->SetStaticMesh(Materials->GetShapeMesh(hadi::Shape::Cylinder));
		Icon->SetStaticMesh(Materials->GetShapeMesh(hadi::Shape::Cone));
		Icon->SetRelativeRotation(FRotator(180.0f, 0.0f, 0.0f)); // point down at the door
	}
	SetMarkerColor(MarkerColor);
	if (UHadiInteractionSubsystem* Interactions = GetWorld()->GetSubsystem<UHadiInteractionSubsystem>())
	{
		Interactions->Register(this);
	}
}

void AHadiInteractableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UHadiInteractionSubsystem* Interactions = GetWorld()->GetSubsystem<UHadiInteractionSubsystem>())
	{
		Interactions->Unregister(this);
	}
	Super::EndPlay(EndPlayReason);
}

void AHadiInteractableActor::SetMarkerColor(const FLinearColor& Color)
{
	MarkerColor = Color;
	if (UHadiMaterialSubsystem* Materials = GetWorld() ? GetWorld()->GetSubsystem<UHadiMaterialSubsystem>() : nullptr)
	{
		UMaterialInterface* Material = Materials->GetMaterial(hadi::Mat::Neon, HadiInteractableLocal::ToCoreColor(Color));
		Ring->SetMaterial(0, Material);
		Icon->SetMaterial(0, Material);
	}
}

void AHadiInteractableActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	BobTime += DeltaSeconds;
	Icon->SetRelativeLocation(FVector(0.0f, 0.0f, 190.0f + 12.0f * FMath::Sin(BobTime * 2.2f)));
	Icon->SetRelativeRotation(FRotator(180.0f, BobTime * 70.0f, 0.0f));
}
