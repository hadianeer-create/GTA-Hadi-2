// HADI: VICE CITY
// Persistent player progression (money, wardrobe, collectibles, story progress, statistics).
#pragma once

#include "CoreMinimal.h"

#include "HadiProfile.generated.h"

USTRUCT(BlueprintType)
struct FHadiStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	float DistanceOnFootMeters = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	float DistanceDrivenMeters = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	float DistanceSwumMeters = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	float PlayTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	float TopSpeedKmh = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	int32 MoneyEarned = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	int32 MoneySpent = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	int32 MealsEaten = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	int32 OutfitsBought = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	int32 LedgesClimbed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	int32 TimesBlackedOut = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi|Stats")
	int32 VehiclesEntered = 0;
};

USTRUCT(BlueprintType)
struct FHadiProfile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hadi")
	int32 Money = 500;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi")
	float Health = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi")
	int32 OutfitIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi")
	TArray<int32> OwnedOutfits = {0};

	UPROPERTY(BlueprintReadOnly, Category = "Hadi")
	TArray<int32> CollectedFlamingos;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi")
	int32 ObjectiveStep = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hadi")
	FHadiStats Stats;
};
