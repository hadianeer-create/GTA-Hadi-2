// HADI: VICE CITY
#include "World/HadiCityGenerator.h"

#include "HadiViceCity.h"
#include "Gameplay/HadiCollectible.h"
#include "Gameplay/HadiSafehouse.h"
#include "Gameplay/HadiShop.h"
#include "Vehicles/HadiVehicle.h"
#include "World/HadiCoreBridge.h"
#include "World/HadiMaterialSubsystem.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"

// ------------------------------------------------------------------------------- city cell
AHadiCityCell::AHadiCityCell()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	RootComponent = Root;
}

UInstancedStaticMeshComponent* AHadiCityCell::GetOrCreateBatch(uint64 Key, UStaticMesh* Mesh, UMaterialInterface* Material, bool bCollision, bool bShadow,
	float CullDistance)
{
	if (TObjectPtr<UInstancedStaticMeshComponent>* Found = Batches.Find(Key))
	{
		return Found->Get();
	}
	UInstancedStaticMeshComponent* Batch = NewObject<UInstancedStaticMeshComponent>(this);
	Batch->SetMobility(EComponentMobility::Static);
	Batch->SetupAttachment(GetRootComponent());
	Batch->SetStaticMesh(Mesh);
	Batch->SetMaterial(0, Material);
	if (bCollision)
	{
		Batch->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	}
	else
	{
		Batch->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Batch->SetCanEverAffectNavigation(false);
	}
	Batch->SetCastShadow(bShadow);
	Batch->SetGenerateOverlapEvents(false);
	if (CullDistance > 0.0f)
	{
		Batch->SetCullDistances(FMath::RoundToInt(CullDistance * 0.85f), FMath::RoundToInt(CullDistance));
	}
	Batch->RegisterComponent();
	AddInstanceComponent(Batch);
	Batches.Add(Key, Batch);
	return Batch;
}

void AHadiCityCell::SetStreamedIn(bool bIn)
{
	if (bStreamedIn == bIn)
	{
		return;
	}
	bStreamedIn = bIn;
	// Visual streaming only: collision stays so parked physics objects never fall through.
	SetActorHiddenInGame(!bIn);
}

// ------------------------------------------------------------------------- city generator
AHadiCityGenerator::AHadiCityGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;
	VehicleClass = AHadiVehicle::StaticClass();
}

void AHadiCityGenerator::Generate()
{
	if (bGenerated)
	{
		return;
	}
	const double StartTime = FPlatformTime::Seconds();
	hadi::CityParams Params;
	if (Seed != 0)
	{
		Params.seed = static_cast<uint64>(Seed);
	}
	Model = hadi::GenerateCity(Params);
	SpawnGeometry();
	CreateLightPools();
	SpawnGameplayActors();
	bGenerated = true;
	UE_LOG(LogHadi, Log, TEXT("Vice City generated: %d primitives -> %d instances in %d cells, %d street lights, %.2f s"), static_cast<int32>(Model.prims.size()),
		InstanceCount, Cells.Num(), StreetLightPositions.Num(), FPlatformTime::Seconds() - StartTime);
}

AHadiCityCell* AHadiCityGenerator::GetOrCreateCell(int32 CellIndex)
{
	if (TObjectPtr<AHadiCityCell>* Found = Cells.Find(CellIndex))
	{
		return Found->Get();
	}
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHadiCityCell* Cell = GetWorld()->SpawnActor<AHadiCityCell>(AHadiCityCell::StaticClass(), FTransform::Identity, Params);
	if (CellIndex < 0)
	{
		Cell->bAlwaysLoaded = true;
	}
	else
	{
		Cell->BoundsMin = HadiBridge::ToFVector2D(Model.CellMin(CellIndex));
		Cell->BoundsMax = HadiBridge::ToFVector2D(Model.CellMax(CellIndex));
	}
	Cells.Add(CellIndex, Cell);
	return Cell;
}

void AHadiCityGenerator::SpawnGeometry()
{
	UHadiMaterialSubsystem* Materials = GetWorld()->GetSubsystem<UHadiMaterialSubsystem>();
	if (!Materials)
	{
		return;
	}

	// Group primitives by streaming cell and by (shape, material, colour, collision, shadow, cull).
	struct FBatchData
	{
		hadi::Prim Sample;
		TArray<FTransform> Transforms;
	};
	TMap<int32, TMap<uint64, FBatchData>> Groups;
	for (const hadi::Prim& Prim : Model.prims)
	{
		const uint64 Key = (uint64(static_cast<uint8>(Prim.shape)) << 40) | (uint64(static_cast<uint8>(Prim.mat)) << 32) | (uint64(HadiBridge::PackColor(Prim.color)) << 4) |
			(uint64(Prim.collision ? 1 : 0) << 3) | (uint64(Prim.castShadow ? 1 : 0) << 2) | uint64(static_cast<uint8>(Prim.cull));
		FBatchData& Batch = Groups.FindOrAdd(Prim.cell).FindOrAdd(Key);
		if (Batch.Transforms.Num() == 0)
		{
			Batch.Sample = Prim;
		}
		Batch.Transforms.Add(FTransform(HadiBridge::ToFRotator(Prim.rot), HadiBridge::ToFVector(Prim.center), HadiBridge::ToFVector(Prim.size) / 100.0));
	}

	for (TPair<int32, TMap<uint64, FBatchData>>& CellGroup : Groups)
	{
		AHadiCityCell* Cell = GetOrCreateCell(CellGroup.Key);
		for (TPair<uint64, FBatchData>& Pair : CellGroup.Value)
		{
			const hadi::Prim& Sample = Pair.Value.Sample;
			UInstancedStaticMeshComponent* Batch = Cell->GetOrCreateBatch(Pair.Key, Materials->GetShapeMesh(Sample.shape), Materials->GetMaterial(Sample.mat, Sample.color),
				Sample.collision, Sample.castShadow, hadi::CullDistanceCm(Sample.cull));
			Batch->AddInstances(Pair.Value.Transforms, false, true);
			InstanceCount += Pair.Value.Transforms.Num();
		}
	}
}

void AHadiCityGenerator::CreateLightPools()
{
	for (const hadi::Poi& Poi : Model.pois)
	{
		if (Poi.type == hadi::PoiType::StreetLight)
		{
			StreetLightPositions.Add(HadiBridge::ToFVector(Poi.pos));
		}
		else if (Poi.type == hadi::PoiType::NeonLight)
		{
			NeonPositions.Add(HadiBridge::ToFVector(Poi.pos));
			NeonColors.Add(HadiBridge::ToLinear(Poi.color));
		}
	}
	auto MakeLight = [this](float Radius, const FLinearColor& Color) {
		UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
		Light->SetupAttachment(GetRootComponent());
		Light->SetMobility(EComponentMobility::Movable);
		Light->SetIntensityUnits(ELightUnits::Candelas);
		Light->SetIntensity(0.0f);
		Light->SetAttenuationRadius(Radius);
		Light->SetSourceRadius(15.0f);
		Light->SetLightColor(Color);
		Light->SetCastShadows(false);
		Light->SetVisibility(false);
		Light->RegisterComponent();
		AddInstanceComponent(Light);
		return Light;
	};
	for (int32 Index = 0; Index < MaxStreetLights; ++Index)
	{
		StreetLightPool.Add(MakeLight(1900.0f, FLinearColor(1.0f, 0.78f, 0.52f)));
	}
	for (int32 Index = 0; Index < MaxNeonLights; ++Index)
	{
		NeonLightPool.Add(MakeLight(1400.0f, FLinearColor::White));
	}
}

void AHadiCityGenerator::SpawnGameplayActors()
{
	UWorld* World = GetWorld();
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	static const hadi::Color CarColors[] = {hadi::pal::Coral, hadi::pal::Aqua, hadi::Rgb(30, 34, 40), hadi::Rgb(236, 236, 240), hadi::Rgb(200, 30, 40),
		hadi::pal::Lavender};

	for (const hadi::Poi& Poi : Model.pois)
	{
		const FVector Location = HadiBridge::ToFVector(Poi.pos);
		const FRotator Rotation(0.0f, Poi.yaw, 0.0f);
		switch (Poi.type)
		{
		case hadi::PoiType::Restaurant:
		case hadi::PoiType::ClothingStore:
		{
			AHadiShop* Shop = World->SpawnActor<AHadiShop>(AHadiShop::StaticClass(), Location, Rotation, Params);
			if (Shop)
			{
				Shop->InitShop(Poi.type == hadi::PoiType::Restaurant ? EHadiShopType::Restaurant : EHadiShopType::Clothing, FText::FromString(ANSI_TO_TCHAR(Poi.name)),
					HadiBridge::ToLinear(Poi.color));
			}
			break;
		}
		case hadi::PoiType::Safehouse:
			World->SpawnActor<AHadiSafehouse>(AHadiSafehouse::StaticClass(), Location, Rotation, Params);
			break;
		case hadi::PoiType::Collectible:
		{
			AHadiCollectible* Collectible = World->SpawnActor<AHadiCollectible>(AHadiCollectible::StaticClass(), Location + FVector(0.0f, 0.0f, 70.0f), Rotation, Params);
			if (Collectible)
			{
				Collectible->CollectibleId = Poi.id;
				Collectibles.Add(Collectible);
			}
			break;
		}
		case hadi::PoiType::VehicleSpawn:
		{
			UClass* Class = VehicleClass ? VehicleClass.Get() : AHadiVehicle::StaticClass();
			AHadiVehicle* Vehicle = World->SpawnActor<AHadiVehicle>(Class, Location + FVector(0.0f, 0.0f, 55.0f), Rotation, Params);
			if (Vehicle)
			{
				Vehicle->SetPaintColor(CarColors[FMath::Abs(Poi.id) % UE_ARRAY_COUNT(CarColors)]);
				Vehicles.Add(Vehicle);
			}
			break;
		}
		default: break;
		}
	}
}

bool AHadiCityGenerator::GetPoi(hadi::PoiType Type, int32 Id, FVector& OutLocation, float& OutYaw) const
{
	if (const hadi::Poi* Poi = Model.FindPoi(Type, Id))
	{
		OutLocation = HadiBridge::ToFVector(Poi->pos);
		OutYaw = Poi->yaw;
		return true;
	}
	return false;
}

FString AHadiCityGenerator::GetLocationName(const FVector& Location) const
{
	const hadi::Vec2 P = HadiBridge::ToHadi2D(Location);
	const FString District = ANSI_TO_TCHAR(Model.DistrictAt(P));
	const FString Street = ANSI_TO_TCHAR(Model.StreetAt(P));
	return Street.IsEmpty() ? District : FString::Printf(TEXT("%s, %s"), *Street, *District);
}

bool AHadiCityGenerator::BuildRoute(const FVector& From, const FVector& To, TArray<FVector2D>& OutPoints) const
{
	OutPoints.Reset();
	std::vector<hadi::Vec2> Points;
	if (!Model.graph.BuildRoute(HadiBridge::ToHadi2D(From), HadiBridge::ToHadi2D(To), Points))
	{
		return false;
	}
	for (const hadi::Vec2& Point : Points)
	{
		OutPoints.Add(HadiBridge::ToFVector2D(Point));
	}
	return true;
}

void AHadiCityGenerator::ApplyCollectedFlamingos(const TArray<int32>& Collected)
{
	for (AHadiCollectible* Collectible : Collectibles)
	{
		if (Collectible)
		{
			Collectible->SetCollected(Collected.Contains(Collectible->CollectibleId));
		}
	}
}

void AHadiCityGenerator::SetVehicleGrip(float GripMultiplier)
{
	for (AHadiVehicle* Vehicle : Vehicles)
	{
		if (Vehicle)
		{
			Vehicle->SetGripMultiplier(GripMultiplier);
		}
	}
}

bool AHadiCityGenerator::GetViewLocation(FVector& OutLocation) const
{
	const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController && PlayerController->PlayerCameraManager)
	{
		OutLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
		return true;
	}
	return false;
}

void AHadiCityGenerator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bGenerated)
	{
		return;
	}
	FVector View;
	if (!GetViewLocation(View))
	{
		return;
	}
	StreamTimer -= DeltaSeconds;
	if (StreamTimer <= 0.0f)
	{
		StreamTimer = 0.25f;
		UpdateStreaming(View);
	}
	LightTimer -= DeltaSeconds;
	if (LightTimer <= 0.0f)
	{
		LightTimer = 0.4f;
		UpdateLights(View);
	}
}

void AHadiCityGenerator::UpdateStreaming(const FVector& ViewLocation)
{
	const FVector2D View(ViewLocation.X, ViewLocation.Y);
	for (TPair<int32, TObjectPtr<AHadiCityCell>>& Pair : Cells)
	{
		AHadiCityCell* Cell = Pair.Value;
		if (!Cell || Cell->bAlwaysLoaded)
		{
			continue;
		}
		const FVector2D Closest(FMath::Clamp(View.X, Cell->BoundsMin.X, Cell->BoundsMax.X), FMath::Clamp(View.Y, Cell->BoundsMin.Y, Cell->BoundsMax.Y));
		const double Distance = FVector2D::Distance(View, Closest);
		// Hysteresis avoids cells flickering in and out at the boundary.
		if (Cell->IsStreamedIn() && Distance > StreamOutDistance)
		{
			Cell->SetStreamedIn(false);
		}
		else if (!Cell->IsStreamedIn() && Distance < StreamInDistance)
		{
			Cell->SetStreamedIn(true);
		}
	}
}

void AHadiCityGenerator::UpdateLights(const FVector& ViewLocation)
{
	const UHadiMaterialSubsystem* Materials = GetWorld()->GetSubsystem<UHadiMaterialSubsystem>();
	const float Night = Materials ? Materials->GetNightFactor() : 0.0f;
	const bool bLightsOn = Night > 0.05f;

	auto AssignPool = [&ViewLocation, bLightsOn, Night](const TArray<FVector>& Positions, const TArray<FLinearColor>* Colors, TArray<TObjectPtr<UPointLightComponent>>& Pool,
						  float Candela) {
		TArray<int32> Order;
		Order.Reserve(Positions.Num());
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			Order.Add(Index);
		}
		Order.Sort([&Positions, &ViewLocation](int32 A, int32 B) {
			return FVector::DistSquared(Positions[A], ViewLocation) < FVector::DistSquared(Positions[B], ViewLocation);
		});
		for (int32 Slot = 0; Slot < Pool.Num(); ++Slot)
		{
			UPointLightComponent* Light = Pool[Slot];
			if (!Light)
			{
				continue;
			}
			const bool bUse = bLightsOn && Order.IsValidIndex(Slot);
			Light->SetVisibility(bUse);
			if (bUse)
			{
				const int32 Index = Order[Slot];
				Light->SetWorldLocation(Positions[Index]);
				Light->SetIntensity(Candela * Night);
				if (Colors)
				{
					Light->SetLightColor((*Colors)[Index]);
				}
			}
		}
	};
	AssignPool(StreetLightPositions, nullptr, StreetLightPool, StreetLightCandela);
	AssignPool(NeonPositions, &NeonColors, NeonLightPool, NeonLightCandela);
}
