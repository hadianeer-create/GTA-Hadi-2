// HADI: VICE CITY
// Restaurants (buy food to restore health) and clothing stores (buy / wear outfits).
#pragma once

#include "CoreMinimal.h"
#include "Gameplay/HadiInteractableActor.h"

#include "HadiShop.generated.h"

class AHadiCharacter;

UENUM(BlueprintType)
enum class EHadiShopType : uint8
{
	Restaurant,
	Clothing
};

USTRUCT(BlueprintType)
struct FHadiFoodItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Hadi")
	FText Name;

	UPROPERTY(EditAnywhere, Category = "Hadi")
	int32 Price = 0;

	UPROPERTY(EditAnywhere, Category = "Hadi")
	float Heal = 0.0f;
};

UCLASS()
class HADIVICECITY_API AHadiShop : public AHadiInteractableActor
{
	GENERATED_BODY()

public:
	AHadiShop();

	void InitShop(EHadiShopType InType, const FText& InName, const FLinearColor& Color);

	EHadiShopType GetShopType() const { return ShopType; }
	const TArray<FHadiFoodItem>& GetMenu() const { return Menu; }

	// Returns true on success; OutMessage is shown to the player either way.
	bool BuyFood(AHadiCharacter* Character, int32 ItemIndex, FText& OutMessage) const;

	virtual FText GetInteractionPrompt(const AHadiCharacter* Character) const override;
	virtual void Interact(AHadiCharacter* Character) override;

	UPROPERTY(EditAnywhere, Category = "Hadi")
	EHadiShopType ShopType = EHadiShopType::Restaurant;

	UPROPERTY(EditAnywhere, Category = "Hadi")
	TArray<FHadiFoodItem> Menu;
};
