// HADI: VICE CITY
#include "Gameplay/HadiInteractable.h"

#include "Characters/HadiCharacter.h"

void UHadiInteractionSubsystem::Register(AActor* Actor)
{
	if (Actor && Cast<IHadiInteractable>(Actor))
	{
		Interactables.AddUnique(Actor);
	}
}

void UHadiInteractionSubsystem::Unregister(AActor* Actor)
{
	Interactables.Remove(Actor);
}

AActor* UHadiInteractionSubsystem::FindBest(const AHadiCharacter* Character) const
{
	if (!Character)
	{
		return nullptr;
	}
	const FVector From = Character->GetActorLocation();
	const FVector Facing = Character->GetActorForwardVector();
	AActor* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	for (const TWeakObjectPtr<AActor>& Weak : Interactables)
	{
		AActor* Actor = Weak.Get();
		const IHadiInteractable* Interactable = Cast<IHadiInteractable>(Actor);
		if (!Interactable || !Interactable->CanInteract(Character))
		{
			continue;
		}
		const FVector ToTarget = Interactable->GetInteractionLocation() - From;
		const float Distance = static_cast<float>(ToTarget.Size2D());
		if (Distance > Interactable->GetInteractionRadius() || FMath::Abs(ToTarget.Z) > 250.0)
		{
			continue;
		}
		const float FacingDot = static_cast<float>(FVector::DotProduct(Facing, ToTarget.GetSafeNormal2D()));
		const float Score = Distance * (1.5f - 0.5f * FacingDot);
		if (Score < BestScore)
		{
			BestScore = Score;
			Best = Actor;
		}
	}
	return Best;
}
