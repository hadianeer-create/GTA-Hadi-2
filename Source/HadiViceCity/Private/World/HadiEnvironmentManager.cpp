// HADI: VICE CITY
#include "World/HadiEnvironmentManager.h"

#include "Core/HadiTimeOfDay.h"
#include "World/HadiMaterialSubsystem.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/World.h"

AHadiEnvironmentManager::AHadiEnvironmentManager()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;

	Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
	Sun->SetupAttachment(Root);
	Sun->SetMobility(EComponentMobility::Movable);
	Sun->SetIntensity(10.0f);
	Sun->SetCastShadows(true);
	Sun->SetUseTemperature(true);
	Sun->SetLightSourceAngle(0.55f);

	Moon = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Moon"));
	Moon->SetupAttachment(Root);
	Moon->SetMobility(EComponentMobility::Movable);
	Moon->SetIntensity(0.0f);
	Moon->SetLightColor(FLinearColor(0.55f, 0.65f, 1.0f));
	Moon->SetCastShadows(true);

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(Root);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(Root);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->bRealTimeCapture = true;
	SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;
	SkyLight->SetIntensity(1.0f);

	Clouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("Clouds"));
	Clouds->SetupAttachment(Root);

	Fog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("Fog"));
	Fog->SetupAttachment(Root);
	Fog->SetFogDensity(0.008f);
	Fog->SetFogHeightFalloff(0.15f);
	Fog->SetVolumetricFog(true);

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(Root);
	PostProcess->bUnbound = true;
	PostProcess->Priority = -1.0f; // user / designer volumes win
}

void AHadiEnvironmentManager::BeginPlay()
{
	Super::BeginPlay();
	Sun->SetAtmosphereSunLight(true);
	Sun->SetAtmosphereSunLightIndex(0);
	Moon->SetAtmosphereSunLight(true);
	Moon->SetAtmosphereSunLightIndex(1);

	FPostProcessSettings& Settings = PostProcess->Settings;
	Settings.bOverride_AutoExposureMinBrightness = true;
	Settings.AutoExposureMinBrightness = MinExposureEV100;
	Settings.bOverride_AutoExposureMaxBrightness = true;
	Settings.AutoExposureMaxBrightness = MaxExposureEV100;
	Settings.bOverride_AutoExposureSpeedUp = true;
	Settings.AutoExposureSpeedUp = 2.5f;
	Settings.bOverride_AutoExposureSpeedDown = true;
	Settings.AutoExposureSpeedDown = 1.5f;
	Settings.bOverride_BloomIntensity = true;
	Settings.BloomIntensity = 0.85f;
	Settings.bOverride_VignetteIntensity = true;
	Settings.VignetteIntensity = 0.35f;
	Settings.bOverride_FilmGrainIntensity = true;
	Settings.FilmGrainIntensity = 0.05f;
	Settings.bOverride_ColorSaturation = true;
	Settings.ColorSaturation = FVector4(1.08f, 1.08f, 1.08f, 1.0f);
	Settings.bOverride_MotionBlurAmount = true;
	Settings.MotionBlurAmount = 0.35f;

	ApplyLighting();
}

void AHadiEnvironmentManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bTimeFlows && DayLengthMinutes > 0.0f)
	{
		const float HoursPerSecond = 24.0f / (DayLengthMinutes * 60.0f);
		TimeOfDayHours = hadi::WrapHours(TimeOfDayHours + HoursPerSecond * DeltaSeconds);
	}
	ApplyLighting();
}

void AHadiEnvironmentManager::SetTimeOfDay(float Hours)
{
	TimeOfDayHours = hadi::WrapHours(Hours);
	ApplyLighting();
	SkyLight->RecaptureSky();
}

void AHadiEnvironmentManager::ApplyLighting()
{
	const hadi::SkyState Sky = hadi::ComputeSky(TimeOfDayHours);
	Sun->SetWorldRotation(FRotator(Sky.lightPitch, Sky.lightYaw, 0.0f));
	Sun->SetIntensity(Sky.sunIntensityLux);
	Sun->SetTemperature(Sky.sunTemperatureK);
	Moon->SetWorldRotation(FRotator(Sky.moonPitch, Sky.moonYaw, 0.0f));
	Moon->SetIntensity(Sky.moonIntensityLux);
	SkyLight->SetIntensity(Sky.skyLightIntensity);
	NightFactor = Sky.nightFactor;
	if (UHadiMaterialSubsystem* Materials = GetWorld()->GetSubsystem<UHadiMaterialSubsystem>())
	{
		Materials->SetNightFactor(NightFactor);
	}
}

FString AHadiEnvironmentManager::GetClockText() const
{
	return FString::Printf(TEXT("%02d:%02d"), hadi::ClockHour(TimeOfDayHours), hadi::ClockMinute(TimeOfDayHours));
}
