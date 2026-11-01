// HADI: VICE CITY
// Builds Hadi's body from the simulation-core rig definition and animates it every frame with
// the procedural animation solver (walk/run/sprint, jump, swim, drive, climb, facial expressions).
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/HadiRig.h"

#include "HadiRigComponent.generated.h"

class UStaticMeshComponent;

UCLASS(ClassGroup = (Hadi), meta = (BlueprintSpawnableComponent))
class HADIVICECITY_API UHadiRigComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UHadiRigComponent();

	// Creates the joint pivots and body-part meshes (safe to call more than once).
	void BuildRig();

	// Re-colours / shows / hides parts for one of the wardrobe outfits (hadi::GetOutfit).
	void ApplyOutfit(int32 InOutfitIndex);
	int32 GetOutfitIndex() const { return OutfitIndex; }

	// Advances the animation solver and pushes the pose onto the components.
	void UpdateAnimation(const hadi::AnimInput& Input, float DeltaTime);

	void SetRigHidden(bool bInHidden);

protected:
	virtual void BeginPlay() override;

private:
	void ApplyPose(const hadi::Pose& Pose);

	UPROPERTY(Transient)
	TArray<TObjectPtr<USceneComponent>> JointComponents;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> SegmentComponents;

	hadi::AnimState AnimState;
	int32 OutfitIndex = 0;
	bool bBuilt = false;
	bool bHidden = false;
};
