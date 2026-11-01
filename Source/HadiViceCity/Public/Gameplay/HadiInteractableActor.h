// HADI: VICE CITY
// Base class for world locations Hadi can use (shops, safehouse door...). Draws a glowing ground
// ring + floating marker so they read clearly in the world, like classic open-world games.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/HadiInteractable.h"

#include "HadiInteractableActor.generated.h"

class UStaticMeshComponent;

UCLASS(Abstract)
class HADIVICECITY_API AHadiInteractableActor : public AActor, public IHadiInteractable
{
	GENERATED_BODY()

public:
	AHadiInteractableActor();

	virtual void Tick(float DeltaSeconds) override;

	virtual FVector GetInteractionLocation() const override { return GetActorLocation(); }
	virtual float GetInteractionRadius() const override { return 260.0f; }

	const FText& GetDisplayName() const { return DisplayName; }

	UPROPERTY(EditAnywhere, Category = "Hadi")
	FText DisplayName;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetMarkerColor(const FLinearColor& Color);

	UPROPERTY(VisibleAnywhere, Category = "Hadi")
	TObjectPtr<UStaticMeshComponent> Ring;

	UPROPERTY(VisibleAnywhere, Category = "Hadi")
	TObjectPtr<UStaticMeshComponent> Icon;

	FLinearColor MarkerColor = FLinearColor(1.0f, 0.8f, 0.2f);

private:
	float BobTime = 0.0f;
};
