// HADI: VICE CITY
// Turns the simulation-core city model into the playable world:
//   * every primitive becomes an instance in a per-cell, per-material instanced mesh batch
//   * 200 m streaming cells are shown / hidden around the camera (distance streaming)
//   * street lights and neon spill lights come from small pools re-assigned to the nearest lamps
//   * shops, the safehouse, collectibles and parked cars are spawned from the model's POIs
#pragma once

#include "CoreMinimal.h"
#include "Core/HadiCity.h"
#include "GameFramework/Actor.h"

#include "HadiCityGenerator.generated.h"

class AHadiCollectible;
class AHadiVehicle;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UPointLightComponent;
class UStaticMesh;

UCLASS(NotPlaceable)
class HADIVICECITY_API AHadiCityCell : public AActor
{
	GENERATED_BODY()

public:
	AHadiCityCell();

	UInstancedStaticMeshComponent* GetOrCreateBatch(uint64 Key, UStaticMesh* Mesh, UMaterialInterface* Material, bool bCollision, bool bShadow, float CullDistance);
	void SetStreamedIn(bool bIn);
	bool IsStreamedIn() const { return bStreamedIn; }

	FVector2D BoundsMin = FVector2D::ZeroVector;
	FVector2D BoundsMax = FVector2D::ZeroVector;
	bool bAlwaysLoaded = false;

private:
	UPROPERTY(Transient)
	TMap<uint64, TObjectPtr<UInstancedStaticMeshComponent>> Batches;

	bool bStreamedIn = true;
};

UCLASS()
class HADIVICECITY_API AHadiCityGenerator : public AActor
{
	GENERATED_BODY()

public:
	AHadiCityGenerator();

	virtual void Tick(float DeltaSeconds) override;

	// Builds the whole district. Called by AHadiGameMode::StartPlay.
	void Generate();
	bool IsGenerated() const { return bGenerated; }

	const hadi::CityModel& GetModel() const { return Model; }
	float GetSeaLevel() const { return Model.seaLevel; }
	bool GetPoi(hadi::PoiType Type, int32 Id, FVector& OutLocation, float& OutYaw) const;
	FString GetLocationName(const FVector& Location) const;
	bool BuildRoute(const FVector& From, const FVector& To, TArray<FVector2D>& OutPoints) const;
	void ApplyCollectedFlamingos(const TArray<int32>& Collected);
	void SetVehicleGrip(float GripMultiplier);
	int32 GetInstanceCount() const { return InstanceCount; }

	// 0 = use the simulation core's default seed (the layout validated by Tools/CoreTests).
	UPROPERTY(EditAnywhere, Category = "City")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, Category = "City|Streaming")
	float StreamInDistance = 95000.0f;

	UPROPERTY(EditAnywhere, Category = "City|Streaming")
	float StreamOutDistance = 110000.0f;

	UPROPERTY(EditAnywhere, Category = "City|Lighting")
	int32 MaxStreetLights = 64;

	UPROPERTY(EditAnywhere, Category = "City|Lighting")
	int32 MaxNeonLights = 16;

	UPROPERTY(EditAnywhere, Category = "City|Lighting")
	float StreetLightCandela = 70.0f;

	UPROPERTY(EditAnywhere, Category = "City|Lighting")
	float NeonLightCandela = 120.0f;

	UPROPERTY(EditAnywhere, Category = "City")
	TSubclassOf<AHadiVehicle> VehicleClass;

private:
	void SpawnGeometry();
	void SpawnGameplayActors();
	void CreateLightPools();
	void UpdateStreaming(const FVector& ViewLocation);
	void UpdateLights(const FVector& ViewLocation);
	AHadiCityCell* GetOrCreateCell(int32 CellIndex);
	bool GetViewLocation(FVector& OutLocation) const;

	hadi::CityModel Model;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<AHadiCityCell>> Cells;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPointLightComponent>> StreetLightPool;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPointLightComponent>> NeonLightPool;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AHadiCollectible>> Collectibles;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AHadiVehicle>> Vehicles;

	TArray<FVector> StreetLightPositions;
	TArray<FVector> NeonPositions;
	TArray<FLinearColor> NeonColors;
	float StreamTimer = 0.0f;
	float LightTimer = 0.0f;
	int32 InstanceCount = 0;
	bool bGenerated = false;
};
