// HADI: VICE CITY
#include "Gameplay/HadiSafehouse.h"

#include "Characters/HadiCharacter.h"
#include "Player/HadiPlayerController.h"

#define LOCTEXT_NAMESPACE "HadiSafehouse"

AHadiSafehouse::AHadiSafehouse()
{
	DisplayName = LOCTEXT("Name", "Hadi's Apartment");
	MarkerColor = FLinearColor(0.1f, 0.85f, 1.0f);
}

FText AHadiSafehouse::GetInteractionPrompt(const AHadiCharacter* /*Character*/) const
{
	return LOCTEXT("Enter", "Enter Hadi's apartment");
}

void AHadiSafehouse::Interact(AHadiCharacter* Character)
{
	if (AHadiPlayerController* PlayerController = Character ? Cast<AHadiPlayerController>(Character->GetController()) : nullptr)
	{
		PlayerController->OpenSafehouseMenu();
	}
}

#undef LOCTEXT_NAMESPACE
