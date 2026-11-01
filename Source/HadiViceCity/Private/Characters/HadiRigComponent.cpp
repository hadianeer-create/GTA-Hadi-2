// HADI: VICE CITY
#include "Characters/HadiRigComponent.h"

#include "World/HadiCoreBridge.h"
#include "World/HadiMaterialSubsystem.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UHadiRigComponent::UHadiRigComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // driven by AHadiCharacter::Tick
}

void UHadiRigComponent::BeginPlay()
{
	Super::BeginPlay();
	BuildRig();
}

void UHadiRigComponent::BuildRig()
{
	if (bBuilt || !GetWorld() || !GetOwner())
	{
		return;
	}
	UHadiMaterialSubsystem* Materials = GetWorld()->GetSubsystem<UHadiMaterialSubsystem>();
	if (!Materials)
	{
		return;
	}
	bBuilt = true;
	AActor* Owner = GetOwner();

	// One pivot per joint, parented exactly like the core rig.
	JointComponents.SetNum(hadi::kJointCount);
	for (int32 Index = 0; Index < hadi::kJointCount; ++Index)
	{
		const hadi::RigJoint& Joint = hadi::GetJoint(static_cast<hadi::Joint>(Index));
		USceneComponent* Pivot = NewObject<USceneComponent>(Owner, *FString::Printf(TEXT("Joint_%s"), ANSI_TO_TCHAR(Joint.name)));
		USceneComponent* Parent = Index == 0 ? static_cast<USceneComponent*>(this) : JointComponents[static_cast<int32>(Joint.parent)].Get();
		Pivot->SetupAttachment(Parent);
		Pivot->SetRelativeLocation(Index == 0 ? FVector::ZeroVector : HadiBridge::ToFVector(Joint.offset));
		Pivot->RegisterComponent();
		Owner->AddInstanceComponent(Pivot);
		JointComponents[Index] = Pivot;
	}

	// Body parts.
	const std::vector<hadi::RigSegment>& Segments = hadi::GetRigSegments();
	SegmentComponents.Reset();
	for (size_t Index = 0; Index < Segments.size(); ++Index)
	{
		const hadi::RigSegment& Seg = Segments[Index];
		UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(Owner, *FString::Printf(TEXT("BodyPart_%d"), static_cast<int32>(Index)));
		Part->SetupAttachment(JointComponents[static_cast<int32>(Seg.joint)].Get());
		Part->SetStaticMesh(Materials->GetShapeMesh(Seg.shape));
		Part->SetRelativeLocation(HadiBridge::ToFVector(Seg.center));
		Part->SetRelativeRotation(HadiBridge::ToFRotator(Seg.rot));
		Part->SetRelativeScale3D(HadiBridge::ToFVector(Seg.size) / 100.0);
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetGenerateOverlapEvents(false);
		Part->SetCanEverAffectNavigation(false);
		Part->bReceivesDecals = false;
		// Tiny facial parts don't need to cast shadows.
		Part->SetCastShadow(Seg.face == hadi::FacePart::None && Seg.size.x * Seg.size.y * Seg.size.z > 30.0f);
		Part->RegisterComponent();
		Owner->AddInstanceComponent(Part);
		SegmentComponents.Add(Part);
	}
	ApplyOutfit(OutfitIndex);
}

void UHadiRigComponent::ApplyOutfit(int32 InOutfitIndex)
{
	OutfitIndex = FMath::Clamp(InOutfitIndex, 0, hadi::OutfitCount() - 1);
	if (!bBuilt)
	{
		return;
	}
	UHadiMaterialSubsystem* Materials = GetWorld()->GetSubsystem<UHadiMaterialSubsystem>();
	const hadi::Outfit& Outfit = hadi::GetOutfit(OutfitIndex);
	const std::vector<hadi::RigSegment>& Segments = hadi::GetRigSegments();
	for (int32 Index = 0; Index < SegmentComponents.Num(); ++Index)
	{
		UStaticMeshComponent* Part = SegmentComponents[Index];
		const hadi::RigSegment& Seg = Segments[static_cast<size_t>(Index)];
		const bool bVisible = hadi::IsSegmentVisible(Seg.vis, Outfit);
		Part->SetVisibility(bVisible && !bHidden);
		if (Materials)
		{
			Part->SetMaterial(0, Materials->GetMaterial(Seg.mat, hadi::ResolveSlotColor(Seg.slot, Outfit)));
		}
	}
}

void UHadiRigComponent::SetRigHidden(bool bInHidden)
{
	bHidden = bInHidden;
	ApplyOutfit(OutfitIndex);
}

void UHadiRigComponent::UpdateAnimation(const hadi::AnimInput& Input, float DeltaTime)
{
	if (!bBuilt)
	{
		return;
	}
	hadi::UpdateAnimState(AnimState, Input, DeltaTime);
	ApplyPose(hadi::ComputePose(AnimState));
}

void UHadiRigComponent::ApplyPose(const hadi::Pose& Pose)
{
	for (int32 Index = 0; Index < JointComponents.Num(); ++Index)
	{
		USceneComponent* Pivot = JointComponents[Index];
		const FRotator Rotation = HadiBridge::ToFRotator(Pose.joints[Index]);
		if (Index == static_cast<int32>(hadi::Joint::Pelvis))
		{
			const hadi::RigJoint& Joint = hadi::GetJoint(hadi::Joint::Pelvis);
			Pivot->SetRelativeLocationAndRotation(HadiBridge::ToFVector(Joint.offset + Pose.pelvisOffset), Rotation);
		}
		else
		{
			Pivot->SetRelativeRotation(Rotation);
		}
	}

	// Facial animation (brows, eyes, mouth).
	const std::vector<hadi::RigSegment>& Segments = hadi::GetRigSegments();
	for (int32 Index = 0; Index < SegmentComponents.Num(); ++Index)
	{
		const hadi::RigSegment& Seg = Segments[static_cast<size_t>(Index)];
		if (Seg.face == hadi::FacePart::None)
		{
			continue;
		}
		const hadi::SegmentTransform T = hadi::ApplyFace(Seg, Pose.face);
		UStaticMeshComponent* Part = SegmentComponents[Index];
		Part->SetRelativeLocationAndRotation(HadiBridge::ToFVector(T.center), HadiBridge::ToFRotator(T.rot));
		Part->SetRelativeScale3D(HadiBridge::ToFVector(T.size) / 100.0);
	}
}
