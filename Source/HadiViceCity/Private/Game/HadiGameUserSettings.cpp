// HADI: VICE CITY
#include "Game/HadiGameUserSettings.h"

#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

namespace HadiSettingsLocal
{
void SetCVar(const TCHAR* Name, int32 Value)
{
	if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
	{
		Variable->Set(Value, ECVF_SetByGameSetting);
	}
}
} // namespace HadiSettingsLocal

UHadiGameUserSettings* UHadiGameUserSettings::GetHadiSettings()
{
	return GEngine ? Cast<UHadiGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

void UHadiGameUserSettings::ApplyHadiSettings()
{
	HadiSettingsLocal::SetCVar(TEXT("r.MotionBlurQuality"), bMotionBlur ? 4 : 0);
	HadiSettingsLocal::SetCVar(TEXT("r.DepthOfFieldQuality"), bDepthOfField ? 2 : 0);
	HadiSettingsLocal::SetCVar(TEXT("r.Lumen.HardwareRayTracing"), bHardwareRayTracing ? 1 : 0);
	FieldOfView = FMath::Clamp(FieldOfView, 60.0f, 110.0f);
	MouseSensitivity = FMath::Clamp(MouseSensitivity, 0.1f, 4.0f);
}
