// HADI: VICE CITY
// Hadi's apartment door: save, sleep (skip 6 hours) and change into owned outfits.
#pragma once

#include "CoreMinimal.h"
#include "Gameplay/HadiInteractableActor.h"

#include "HadiSafehouse.generated.h"

UCLASS()
class HADIVICECITY_API AHadiSafehouse : public AHadiInteractableActor
{
	GENERATED_BODY()

public:
	AHadiSafehouse();

	virtual FText GetInteractionPrompt(const AHadiCharacter* Character) const override;
	virtual void Interact(AHadiCharacter* Character) override;
};
