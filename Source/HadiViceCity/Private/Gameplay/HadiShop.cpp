// HADI: VICE CITY
#include "Gameplay/HadiShop.h"

#include "Characters/HadiCharacter.h"
#include "Game/HadiGameInstance.h"
#include "Game/HadiGameMode.h"
#include "Player/HadiPlayerController.h"

#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "HadiShop"

AHadiShop::AHadiShop()
{
	auto AddItem = [this](const FText& Name, int32 Price, float Heal) {
		FHadiFoodItem Item;
		Item.Name = Name;
		Item.Price = Price;
		Item.Heal = Heal;
		Menu.Add(Item);
	};
	AddItem(LOCTEXT("Cafecito", "Cafecito"), 4, 10.0f);
	AddItem(LOCTEXT("Smoothie", "Mango Smoothie"), 9, 25.0f);
	AddItem(LOCTEXT("Sandwich", "Pressed Pork Sandwich"), 14, 45.0f);
	AddItem(LOCTEXT("Snapper", "Grilled Snapper Plate"), 26, 80.0f);
	AddItem(LOCTEXT("Special", "Hadi's Breakfast Special"), 18, 100.0f);
}

void AHadiShop::InitShop(EHadiShopType InType, const FText& InName, const FLinearColor& Color)
{
	ShopType = InType;
	DisplayName = InName;
	SetMarkerColor(Color);
}

FText AHadiShop::GetInteractionPrompt(const AHadiCharacter* /*Character*/) const
{
	return ShopType == EHadiShopType::Restaurant ? FText::Format(LOCTEXT("EatAt", "Eat at {0}"), DisplayName)
												 : FText::Format(LOCTEXT("ShopAt", "Shop at {0}"), DisplayName);
}

void AHadiShop::Interact(AHadiCharacter* Character)
{
	if (AHadiPlayerController* PlayerController = Character ? Cast<AHadiPlayerController>(Character->GetController()) : nullptr)
	{
		PlayerController->OpenShopMenu(this);
	}
}

bool AHadiShop::BuyFood(AHadiCharacter* Character, int32 ItemIndex, FText& OutMessage) const
{
	AHadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AHadiGameMode>();
	if (!Character || !GameMode || !Menu.IsValidIndex(ItemIndex))
	{
		return false;
	}
	const FHadiFoodItem& Item = Menu[ItemIndex];
	if (Character->GetHealth() >= Character->GetMaxHealth())
	{
		OutMessage = LOCTEXT("NotHungry", "Hadi isn't hungry right now.");
		return false;
	}
	if (!GameMode->SpendMoney(Item.Price))
	{
		OutMessage = LOCTEXT("NoMoney", "Not enough cash.");
		return false;
	}
	Character->Heal(Item.Heal);
	Character->ShowExpression(hadi::Expression::Happy, 2.5f);
	if (UHadiGameInstance* GameInstance = GetGameInstance<UHadiGameInstance>())
	{
		GameInstance->Profile.Stats.MealsEaten++;
	}
	GameMode->ReportEvent(AHadiGameMode::EventAteFood);
	OutMessage = FText::Format(LOCTEXT("Ate", "{0} - delicious. (+{1} health)"), Item.Name, FText::AsNumber(FMath::RoundToInt(Item.Heal)));
	return true;
}

#undef LOCTEXT_NAMESPACE
