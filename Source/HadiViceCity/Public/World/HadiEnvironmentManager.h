// HADI: VICE CITY
// Sky, sun, moon, clouds, fog, exposure and the 24-hour day/night cycle. Everything is a
// movable component so Lumen and the sky atmosphere react in real time.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "HadiEnvironmentManager.generated.h"

class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class UPostProcessComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UVolumetricCloudComponent;

UCLASS()
class HADIVICECITY_API AHadiEnvironmentManager : public AActor
{
	GENERATED_BODY()

public:
	AHadiEnvironmentManager();

	virtual void Tick(float DeltaSeconds) override;

	void SetTimeOfDay(float Hours);
	float GetTimeOfDay() const { return TimeOfDayHours; }
	void AdvanceHours(float Hours) { SetTimeOfDay(TimeOfDayHours + Hours); }
	float GetNightFactor() const { return NightFactor; }
	FString GetClockText() const;

	// Real-time minutes for one in-game day (default 48 => one game hour every 2 minutes).
	UPROPERTY(EditAnywhere, Category = "Environment")
	float DayLengthMinutes = 48.0f;

	UPROPERTY(EditAnywhere, Category = "Environment")
	bool bTimeFlows = true;

	UPROPERTY(EditAnywhere, Category = "Environment")
	float TimeOfDayHours = 9.0f;

	UPROPERTY(EditAnywhere, Category = "Environment|Exposure")
	float MinExposureEV100 = -1.0f;

	UPROPERTY(EditAnywhere, Category = "Environment|Exposure")
	float MaxExposureEV100 = 9.0f;

	UPROPERTY(VisibleAnywhere, Category = "Environment")
	TObjectPtr<UDirectionalLightComponent> Sun;

	UPROPERTY(VisibleAnywhere, Category = "Environment")
	TObjectPtr<UDirectionalLightComponent> Moon;

	UPROPERTY(VisibleAnywhere, Category = "Environment")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(VisibleAnywhere, Category = "Environment")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, Category = "Environment")
	TObjectPtr<UVolumetricCloudComponent> Clouds;

	UPROPERTY(VisibleAnywhere, Category = "Environment")
	TObjectPtr<UExponentialHeightFogComponent> Fog;

	UPROPERTY(VisibleAnywhere, Category = "Environment")
	TObjectPtr<UPostProcessComponent> PostProcess;

protected:
	virtual void BeginPlay() override;

private:
	void ApplyLighting();

	float NightFactor = 0.0f;
	float SkyRecaptureTimer = 0.0f;
};
