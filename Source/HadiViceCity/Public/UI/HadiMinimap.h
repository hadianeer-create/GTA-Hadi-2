// HADI: VICE CITY
// Rotating, player-centred minimap rendered into a canvas render target from the city model's
// 2D map shapes, with a GPS route line, blips, an objective marker and a north indicator.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "HadiMinimap.generated.h"

class UCanvas;
class UCanvasRenderTarget2D;
class UFont;
class UTexture;

namespace hadi
{
struct CityModel;
}

struct FHadiMinimapBlip
{
	FVector2D World = FVector2D::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	FString Letter;
	bool bClampToEdge = false;
};

struct FHadiMinimapState
{
	FVector2D Center = FVector2D::ZeroVector; // world cm
	float CameraYaw = 0.0f;
	float PlayerYaw = 0.0f;
	float PixelsPerCm = 0.012f;
	TArray<FVector2D> Route;
	TArray<FHadiMinimapBlip> Blips;
};

UCLASS()
class HADIVICECITY_API UHadiMinimap : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UObject* WorldContext, const hadi::CityModel* InModel, UFont* InFont, int32 Width, int32 Height);
	void Render(const FHadiMinimapState& InState);
	UTexture* GetTexture() const;
	bool IsReady() const { return Target != nullptr && Model != nullptr; }

	UFUNCTION()
	void OnDraw(UCanvas* Canvas, int32 Width, int32 Height);

private:
	UPROPERTY(Transient)
	TObjectPtr<UCanvasRenderTarget2D> Target;

	UPROPERTY(Transient)
	TObjectPtr<UFont> Font;

	const hadi::CityModel* Model = nullptr;
	TArray<int32> DrawOrder;
	FHadiMinimapState State;
};
