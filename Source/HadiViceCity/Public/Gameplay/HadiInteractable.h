// HADI: VICE CITY
// Anything Hadi can use with the Interact key: shops, doors, vehicles, the safehouse...
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/Interface.h"

#include "HadiInteractable.generated.h"

class AHadiCharacter;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UHadiInteractable : public UInterface
{
	GENERATED_BODY()
};

class HADIVICECITY_API IHadiInteractable
{
	GENERATED_BODY()

public:
	// Text shown next to the key prompt, e.g. "Enter Vento GT".
	virtual FText GetInteractionPrompt(const AHadiCharacter* /*Character*/) const { return FText::GetEmpty(); }
	virtual bool CanInteract(const AHadiCharacter* /*Character*/) const { return true; }
	virtual void Interact(AHadiCharacter* /*Character*/) {}
	virtual FVector GetInteractionLocation() const { return FVector::ZeroVector; }
	virtual float GetInteractionRadius() const { return 250.0f; }
};

// Registry of interactables in the world; the character asks it for the best candidate.
UCLASS()
class HADIVICECITY_API UHadiInteractionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void Register(AActor* Actor);
	void Unregister(AActor* Actor);

	// Closest usable interactable in range, preferring the ones Hadi is facing.
	AActor* FindBest(const AHadiCharacter* Character) const;

private:
	TArray<TWeakObjectPtr<AActor>> Interactables;
};
