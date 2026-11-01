// HADI: VICE CITY
// Lives for the whole session (survives level reloads) and owns the active player profile.
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Game/HadiProfile.h"

#include "HadiGameInstance.generated.h"

UCLASS()
class HADIVICECITY_API UHadiGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi")
	FHadiProfile Profile;

	// Set when the player picked New Game / Continue; cleared when returning to the main menu.
	UPROPERTY(BlueprintReadOnly, Category = "Hadi")
	bool bSessionActive = false;

	void ResetProfile() { Profile = FHadiProfile(); }
};
