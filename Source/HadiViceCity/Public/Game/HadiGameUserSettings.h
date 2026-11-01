// HADI: VICE CITY
// Player-facing options persisted in GameUserSettings.ini on top of the engine's scalability,
// resolution, window mode, v-sync and frame-rate limit.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"

#include "HadiGameUserSettings.generated.h"

UCLASS(config = GameUserSettings, configdonotcheckdefaults)
class HADIVICECITY_API UHadiGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	static UHadiGameUserSettings* GetHadiSettings();

	// Pushes the Hadi options into the renderer (console variables).
	void ApplyHadiSettings();

	UPROPERTY(Config)
	bool bMotionBlur = true;

	UPROPERTY(Config)
	bool bDepthOfField = true;

	// Lumen hardware ray tracing (needs a DX12 ray-tracing capable GPU; falls back to software Lumen).
	UPROPERTY(Config)
	bool bHardwareRayTracing = false;

	UPROPERTY(Config)
	float FieldOfView = 80.0f;

	UPROPERTY(Config)
	float MouseSensitivity = 1.0f;

	UPROPERTY(Config)
	bool bInvertLook = false;

	UPROPERTY(Config)
	bool bShowFPS = false;
};
