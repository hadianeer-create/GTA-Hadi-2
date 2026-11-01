// HADI: VICE CITY
#pragma once

#include "CoreMinimal.h"
#include "Game/HadiProfile.h"
#include "GameFramework/SaveGame.h"

#include "HadiSaveGame.generated.h"

namespace HadiSave
{
inline constexpr int32 UserIndex = 0;
inline constexpr int32 CurrentVersion = 1;
inline const TCHAR* const SlotName = TEXT("HadiViceCity_Slot0");
} // namespace HadiSave

UCLASS()
class HADIVICECITY_API UHadiSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	FHadiProfile Profile;

	UPROPERTY()
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY()
	float PlayerYaw = 0.0f;

	UPROPERTY()
	float TimeOfDayHours = 9.0f;

	UPROPERTY()
	FDateTime SavedAt;

	UPROPERTY()
	FString LocationName;
};
