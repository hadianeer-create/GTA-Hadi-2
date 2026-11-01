// HADI: VICE CITY
// Neon Flamingo collectible (12 hidden around Coral Key) and the objective waypoint marker.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "HadiCollectible.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;

UCLASS()
class HADIVICECITY_API AHadiCollectible : public AActor
{
	GENERATED_BODY()

public:
	AHadiCollectible();

	virtual void Tick(float DeltaSeconds) override;

	void SetCollected(bool bInCollected);
	bool IsCollected() const { return bCollected; }

	UPROPERTY(VisibleAnywhere, Category = "Hadi")
	int32 CollectibleId = 0;

	UPROPERTY(EditAnywhere, Category = "Hadi")
	float PickupRadius = 170.0f;

protected:
	virtual void BeginPlay() override;

private:
	void BuildFlamingo();

	UPROPERTY(VisibleAnywhere, Category = "Hadi")
	TObjectPtr<USceneComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "Hadi")
	TObjectPtr<UPointLightComponent> Glow;

	float Time = 0.0f;
	bool bCollected = false;
	bool bBuilt = false;
};

UCLASS()
class HADIVICECITY_API AHadiWaypointMarker : public AActor
{
	GENERATED_BODY()

public:
	AHadiWaypointMarker();

	virtual void Tick(float DeltaSeconds) override;
	void ShowAt(const FVector& GroundLocation);
	void HideMarker();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Hadi")
	TObjectPtr<UStaticMeshComponent> Column;

	UPROPERTY(VisibleAnywhere, Category = "Hadi")
	TObjectPtr<UStaticMeshComponent> Ring;

	float Time = 0.0f;
};
