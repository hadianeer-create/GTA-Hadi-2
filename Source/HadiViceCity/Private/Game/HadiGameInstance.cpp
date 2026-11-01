// HADI: VICE CITY
#include "Game/HadiGameInstance.h"

#include "Game/HadiGameUserSettings.h"

void UHadiGameInstance::Init()
{
	Super::Init();
	// Apply the saved graphics options (motion blur, DOF, ray tracing...) at startup.
	if (UHadiGameUserSettings* Settings = UHadiGameUserSettings::GetHadiSettings())
	{
		Settings->ApplyHadiSettings();
	}
}
