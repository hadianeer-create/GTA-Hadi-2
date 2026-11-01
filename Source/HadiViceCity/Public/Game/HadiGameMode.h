// HADI: VICE CITY
// Session flow (main menu -> new game / continue -> play), world bootstrap, money, wardrobe,
// collectibles, the prologue objective chain with GPS waypoints, blackout/respawn and saving.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "HadiGameMode.generated.h"

class AHadiCharacter;
class AHadiCityGenerator;
class AHadiEnvironmentManager;
class AHadiHUD;
class AHadiWaypointMarker;
class UHadiGameInstance;

USTRUCT()
struct FHadiObjective
{
	GENERATED_BODY()

	FText Title;
	FText Description;
	FName RequiredEvent;   // completes when this event is reported...
	bool bHasTarget = false;
	FVector Target = FVector::ZeroVector;
	float Radius = 600.0f; // ...or, with no event, when the player reaches the target
	int32 Reward = 0;
	FText CompletionTitle;
	FText CompletionText;
};

UCLASS()
class HADIVICECITY_API AHadiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHadiGameMode();

	static const FName EventAteFood;
	static const FName EventEnteredVehicle;

	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	// ---- Session ----
	void StartNewGame(APlayerController* PlayerController);
	bool ContinueGame(APlayerController* PlayerController);
	bool SaveGame();
	bool HasSaveGame() const;
	FString GetSaveDescription() const;
	void ReturnToMainMenu();
	void SleepAtSafehouse();
	bool IsSessionActive() const;

	// ---- Economy & progression ----
	int32 GetMoney() const;
	void AddMoney(int32 Amount);
	bool SpendMoney(int32 Amount);
	bool OwnsOutfit(int32 OutfitIndex) const;
	bool BuyOrWearOutfit(AHadiCharacter* Character, int32 OutfitIndex, FText& OutMessage);
	void CollectFlamingo(int32 Id);
	int32 GetFlamingoCount() const;
	int32 GetFlamingoTotal() const { return 12; }

	// ---- Objectives ----
	void ReportEvent(FName Event);
	bool HasActiveObjective() const;
	FText GetObjectiveTitle() const;
	FText GetObjectiveText() const;
	bool GetObjectiveTarget(FVector& OutTarget) const;

	// ---- Player ----
	void OnPlayerDied(AHadiCharacter* Character);
	AHadiCharacter* GetHadi() const { return Hadi; }
	void Notify(const FText& Message);
	void ShowBigMessage(const FText& Title, const FText& Subtitle, float Duration);

	AHadiCityGenerator* GetCity() const { return City; }
	AHadiEnvironmentManager* GetEnvironment() const { return Environment; }
	float GetSeaLevel() const;

	UPROPERTY(EditAnywhere, Category = "Hadi")
	TSubclassOf<AHadiCharacter> HadiClass;

	UPROPERTY(EditAnywhere, Category = "Hadi")
	float AutosaveIntervalSeconds = 300.0f;

private:
	void BuildObjectives();
	void CompleteObjective();
	void UpdateObjectives();
	void RespawnAfterBlackout();
	void FinishSessionStart(APlayerController* PlayerController);
	AHadiCharacter* SpawnHadi(APlayerController* PlayerController, const FVector& GroundLocation, float Yaw);
	AHadiHUD* GetHadiHUD() const;
	UHadiGameInstance* GetHadiGameInstance() const;

	UPROPERTY(Transient)
	TObjectPtr<AHadiCityGenerator> City;

	UPROPERTY(Transient)
	TObjectPtr<AHadiEnvironmentManager> Environment;

	UPROPERTY(Transient)
	TObjectPtr<AHadiWaypointMarker> Waypoint;

	UPROPERTY(Transient)
	TObjectPtr<AHadiCharacter> Hadi;

	UPROPERTY(Transient)
	TArray<FHadiObjective> Objectives;

	FTimerHandle RespawnTimer;
	float AutosaveTimer = 0.0f;
};
