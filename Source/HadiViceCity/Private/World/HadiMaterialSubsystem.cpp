// HADI: VICE CITY
#include "World/HadiMaterialSubsystem.h"

#include "HadiViceCity.h"
#include "World/HadiCoreBridge.h"

#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/PackageName.h"

namespace HadiMaterialParams
{
// Parameter names shared with Content/Python/hadi_setup.py.
const FName BaseColor(TEXT("BaseColor"));
const FName LegacyColor(TEXT("Color")); // /Engine/BasicShapes/BasicShapeMaterial fallback
const FName Roughness(TEXT("Roughness"));
const FName Metallic(TEXT("Metallic"));
const FName Specular(TEXT("Specular"));
const FName EmissiveColor(TEXT("EmissiveColor"));
const FName EmissiveStrength(TEXT("EmissiveStrength"));
const FName Opacity(TEXT("Opacity"));
const FName Tiling(TEXT("Tiling"));

// Emissive strengths (scene-referred luminance) for day / night.
constexpr float WindowDay = 0.0f;
constexpr float WindowNight = 6.0f;
constexpr float NeonDay = 1.5f;
constexpr float NeonNight = 28.0f;
constexpr float LightDay = 0.3f;
constexpr float LightNight = 45.0f;
} // namespace HadiMaterialParams

UMaterialInterface* UHadiMaterialSubsystem::LoadMaterialIfExists(const TCHAR* PackagePath, const TCHAR* ObjectPath)
{
	if (!FPackageName::DoesPackageExist(PackagePath))
	{
		return nullptr;
	}
	return LoadObject<UMaterialInterface>(nullptr, ObjectPath);
}

void UHadiMaterialSubsystem::EnsureLoaded()
{
	if (bLoaded)
	{
		return;
	}
	bLoaded = true;

	OpaqueBase = LoadMaterialIfExists(TEXT("/Game/Hadi/Materials/M_Hadi_Opaque"), TEXT("/Game/Hadi/Materials/M_Hadi_Opaque.M_Hadi_Opaque"));
	GlassBase = LoadMaterialIfExists(TEXT("/Game/Hadi/Materials/M_Hadi_Glass"), TEXT("/Game/Hadi/Materials/M_Hadi_Glass.M_Hadi_Glass"));
	WaterBase = LoadMaterialIfExists(TEXT("/Game/Hadi/Materials/M_Hadi_Water"), TEXT("/Game/Hadi/Materials/M_Hadi_Water.M_Hadi_Water"));
	bGeneratedMaterials = OpaqueBase != nullptr;

	if (!OpaqueBase)
	{
		UE_LOG(LogHadi, Warning,
			TEXT("Generated materials not found in /Game/Hadi/Materials. Open the project in the editor once so Content/Python/init_unreal.py can create them. Using engine fallback materials."));
		OpaqueBase = LoadMaterialIfExists(TEXT("/Engine/BasicShapes/BasicShapeMaterial"), TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	if (!OpaqueBase)
	{
		OpaqueBase = UMaterial::GetDefaultMaterial(MD_Surface);
	}

	static const TCHAR* const MeshPaths[] = {
		TEXT("/Engine/BasicShapes/Cube.Cube"),
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),
		TEXT("/Engine/BasicShapes/Sphere.Sphere"),
		TEXT("/Engine/BasicShapes/Cone.Cone"),
	};
	for (const TCHAR* Path : MeshPaths)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path);
		if (!Mesh)
		{
			UE_LOG(LogHadi, Error, TEXT("Missing engine mesh %s"), Path);
		}
		ShapeMeshes.Add(Mesh);
	}
}

UStaticMesh* UHadiMaterialSubsystem::GetShapeMesh(hadi::Shape Shape)
{
	EnsureLoaded();
	const int32 Index = static_cast<int32>(Shape);
	return ShapeMeshes.IsValidIndex(Index) ? ShapeMeshes[Index].Get() : nullptr;
}

UMaterialInterface* UHadiMaterialSubsystem::GetMaterial(hadi::Mat Kind, const hadi::Color& Color)
{
	using namespace HadiMaterialParams;
	EnsureLoaded();

	const uint64 Key = (uint64(static_cast<uint8>(Kind)) << 32) | uint64(HadiBridge::PackColor(Color));
	if (TObjectPtr<UMaterialInstanceDynamic>* Found = Cache.Find(Key))
	{
		return Found->Get();
	}

	UMaterialInterface* Parent = OpaqueBase;
	if (Kind == hadi::Mat::GlassClear && GlassBase)
	{
		Parent = GlassBase;
	}
	else if (Kind == hadi::Mat::Water && WaterBase)
	{
		Parent = WaterBase;
	}

	UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Parent, this);
	const FLinearColor Linear = HadiBridge::ToLinear(Color);
	FLinearColor Base = Linear;
	float Rough = 0.6f;
	float Metal = 0.0f;
	float Spec = 0.5f;
	float DayEmissive = 0.0f;
	float NightEmissive = 0.0f;
	bool bEmissive = false;

	switch (Kind)
	{
	case hadi::Mat::Matte: Rough = 0.85f; Spec = 0.4f; break;
	case hadi::Mat::Painted: Rough = 0.55f; break;
	case hadi::Mat::Glossy: Rough = 0.18f; Metal = 0.1f; Spec = 0.6f; break;
	case hadi::Mat::Metal: Rough = 0.32f; Metal = 1.0f; break;
	case hadi::Mat::Glass: Rough = 0.04f; Metal = 0.75f; break;
	case hadi::Mat::GlassClear: Rough = 0.03f; Metal = Parent == GlassBase ? 0.0f : 0.6f; break;
	case hadi::Mat::WindowLit:
		Base = FLinearColor(0.012f, 0.02f, 0.03f);
		Rough = 0.06f;
		Metal = 0.7f;
		bEmissive = true;
		DayEmissive = WindowDay;
		NightEmissive = WindowNight;
		break;
	case hadi::Mat::Neon:
		Rough = 0.3f;
		bEmissive = true;
		DayEmissive = NeonDay;
		NightEmissive = NeonNight;
		break;
	case hadi::Mat::Light:
		Base = Linear * 0.8f;
		Rough = 0.2f;
		bEmissive = true;
		DayEmissive = LightDay;
		NightEmissive = LightNight;
		break;
	case hadi::Mat::Water: Rough = 0.04f; Metal = Parent == WaterBase ? 0.0f : 0.2f; break;
	case hadi::Mat::Foliage: Rough = 0.8f; Spec = 0.35f; break;
	case hadi::Mat::Sand: Rough = 0.95f; Spec = 0.3f; break;
	case hadi::Mat::Asphalt: Rough = 0.88f; Spec = 0.4f; break;
	default: break;
	}

	Mid->SetVectorParameterValue(BaseColor, Base);
	Mid->SetVectorParameterValue(LegacyColor, Base);
	Mid->SetScalarParameterValue(Roughness, Rough);
	Mid->SetScalarParameterValue(Metallic, Metal);
	Mid->SetScalarParameterValue(Specular, Spec);
	if (Kind == hadi::Mat::GlassClear)
	{
		Mid->SetScalarParameterValue(Opacity, 0.32f);
	}
	if (Kind == hadi::Mat::Water)
	{
		// The ocean slab is 60 km wide with 0..1 UVs; pools / fountains are a few metres.
		const bool bOcean = HadiBridge::PackColor(Color) == HadiBridge::PackColor(hadi::pal::Ocean);
		Mid->SetScalarParameterValue(Tiling, bOcean ? 30000.0f : 4.0f);
	}
	if (bEmissive)
	{
		Mid->SetVectorParameterValue(EmissiveColor, Linear);
		Mid->SetScalarParameterValue(EmissiveStrength, FMath::Lerp(DayEmissive, NightEmissive, NightFactor));
		FEmissiveEntry Entry;
		Entry.Material = Mid;
		Entry.DayStrength = DayEmissive;
		Entry.NightStrength = NightEmissive;
		EmissiveEntries.Add(Entry);
	}
	else
	{
		Mid->SetScalarParameterValue(EmissiveStrength, 0.0f);
	}

	Cache.Add(Key, Mid);
	return Mid;
}

void UHadiMaterialSubsystem::SetNightFactor(float InNightFactor)
{
	const float Clamped = FMath::Clamp(InNightFactor, 0.0f, 1.0f);
	if (FMath::Abs(Clamped - NightFactor) < 0.005f)
	{
		return;
	}
	NightFactor = Clamped;
	for (const FEmissiveEntry& Entry : EmissiveEntries)
	{
		if (UMaterialInstanceDynamic* Mid = Entry.Material.Get())
		{
			Mid->SetScalarParameterValue(HadiMaterialParams::EmissiveStrength, FMath::Lerp(Entry.DayStrength, Entry.NightStrength, NightFactor));
		}
	}
}
