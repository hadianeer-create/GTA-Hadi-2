// HADI: VICE CITY
// Owns the shared base materials / meshes and hands out cached material instances per
// (surface type, colour). Emissive surfaces are re-lit as the night factor changes.
#pragma once

#include "CoreMinimal.h"
#include "Core/HadiTypes.h"
#include "Subsystems/WorldSubsystem.h"

#include "HadiMaterialSubsystem.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UStaticMesh;

UCLASS()
class HADIVICECITY_API UHadiMaterialSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Cached material instance for a surface family + sRGB colour.
	UMaterialInterface* GetMaterial(hadi::Mat Kind, const hadi::Color& Color);

	// /Engine/BasicShapes mesh for a core shape.
	UStaticMesh* GetShapeMesh(hadi::Shape Shape);

	// 0 = day, 1 = night. Updates every emissive instance.
	void SetNightFactor(float InNightFactor);
	float GetNightFactor() const { return NightFactor; }

	// True when the editor-generated /Game/Hadi materials were found (otherwise engine fallbacks are used).
	bool UsesGeneratedMaterials() const { return bGeneratedMaterials; }

private:
	struct FEmissiveEntry
	{
		TWeakObjectPtr<UMaterialInstanceDynamic> Material;
		float DayStrength = 0.0f;
		float NightStrength = 0.0f;
	};

	void EnsureLoaded();
	static UMaterialInterface* LoadMaterialIfExists(const TCHAR* PackagePath, const TCHAR* ObjectPath);

	UPROPERTY()
	TObjectPtr<UMaterialInterface> OpaqueBase;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> GlassBase;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> WaterBase;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMesh>> ShapeMeshes;

	UPROPERTY()
	TMap<uint64, TObjectPtr<UMaterialInstanceDynamic>> Cache;

	TArray<FEmissiveEntry> EmissiveEntries;
	float NightFactor = 0.0f;
	bool bLoaded = false;
	bool bGeneratedMaterials = false;
};
