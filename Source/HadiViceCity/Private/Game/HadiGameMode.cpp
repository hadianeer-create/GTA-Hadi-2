// HADI: VICE CITY
#include "Game/HadiGameMode.h"

#include "HadiViceCity.h"
#include "Characters/HadiCharacter.h"
#include "Core/HadiRig.h"
#include "Game/HadiGameInstance.h"
#include "Game/HadiSaveGame.h"
#include "Gameplay/HadiCollectible.h"
#include "Player/HadiPlayerController.h"
#include "UI/HadiHUD.h"
#include "Vehicles/HadiVehicle.h"
#include "World/HadiCityGenerator.h"
#include "World/HadiEnvironmentManager.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "HadiGameMode"

const FName AHadiGameMode::EventAteFood(TEXT("AteFood"));
const FName AHadiGameMode::EventEnteredVehicle(TEXT("EnteredVehicle"));

AHadiGameMode::AHadiGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PlayerControllerClass = AHadiPlayerController::StaticClass();
	HUDClass = AHadiHUD::StaticClass();
	DefaultPawnClass = nullptr; // Hadi is spawned when the player picks New Game / Continue
	HadiClass = AHadiCharacter::StaticClass();
}

void AHadiGameMode::StartPlay()
{
	UWorld* World = GetWorld();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Use hand-placed world actors if a designer added them to the map; otherwise create them.
	for (TActorIterator<AHadiEnvironmentManager> It(World); It; ++It)
	{
		Environment = *It;
		break;
	}
	if (!Environment)
	{
		Environment = World->SpawnActor<AHadiEnvironmentManager>(AHadiEnvironmentManager::StaticClass(), FTransform::Identity, Params);
	}
	for (TActorIterator<AHadiCityGenerator> It(World); It; ++It)
	{
		City = *It;
		break;
	}
	if (!City)
	{
		City = World->SpawnActor<AHadiCityGenerator>(AHadiCityGenerator::StaticClass(), FTransform::Identity, Params);
	}
	City->Generate();
	Waypoint = World->SpawnActor<AHadiWaypointMarker>(AHadiWaypointMarker::StaticClass(), FTransform::Identity, Params);
	BuildObjectives();

	Super::StartPlay();
}

void AHadiGameMode::HandleStartingNewPlayer_Implementation(APlayerController* /*NewPlayer*/)
{
	// Intentionally do not RestartPlayer(): the player starts in the main menu with no pawn.
	// AHadiPlayerController::BeginPlay opens the menu; StartNewGame / ContinueGame spawn Hadi.
}

UHadiGameInstance* AHadiGameMode::GetHadiGameInstance() const
{
	return GetGameInstance<UHadiGameInstance>();
}

AHadiHUD* AHadiGameMode::GetHadiHUD() const
{
	const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	return PlayerController ? PlayerController->GetHUD<AHadiHUD>() : nullptr;
}

bool AHadiGameMode::IsSessionActive() const
{
	const UHadiGameInstance* GameInstance = GetHadiGameInstance();
	return GameInstance && GameInstance->bSessionActive && Hadi != nullptr;
}

float AHadiGameMode::GetSeaLevel() const
{
	return City ? City->GetSeaLevel() : -60.0f;
}

// --------------------------------------------------------------------------------- session
AHadiCharacter* AHadiGameMode::SpawnHadi(APlayerController* PlayerController, const FVector& GroundLocation, float Yaw)
{
	if (Hadi)
	{
		Hadi->Destroy();
		Hadi = nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	UClass* Class = HadiClass ? HadiClass.Get() : AHadiCharacter::StaticClass();
	Hadi = GetWorld()->SpawnActor<AHadiCharacter>(Class, GroundLocation + FVector(0.0f, 0.0f, 95.0f), FRotator(0.0f, Yaw, 0.0f), Params);
	if (Hadi && PlayerController)
	{
		PlayerController->Possess(Hadi);
		PlayerController->SetControlRotation(FRotator(-10.0f, Yaw, 0.0f));
	}
	return Hadi;
}

void AHadiGameMode::FinishSessionStart(APlayerController* PlayerController)
{
	UHadiGameInstance* GameInstance = GetHadiGameInstance();
	GameInstance->bSessionActive = true;
	Hadi->SetOutfit(GameInstance->Profile.OutfitIndex);
	City->ApplyCollectedFlamingos(GameInstance->Profile.CollectedFlamingos);
	AutosaveTimer = AutosaveIntervalSeconds;
	if (AHadiPlayerController* HadiController = Cast<AHadiPlayerController>(PlayerController))
	{
		HadiController->EnterGameplay();
	}
}

void AHadiGameMode::StartNewGame(APlayerController* PlayerController)
{
	UHadiGameInstance* GameInstance = GetHadiGameInstance();
	if (!GameInstance || !City)
	{
		return;
	}
	GameInstance->ResetProfile();
	Environment->SetTimeOfDay(8.5f);
	FVector Spawn;
	float Yaw = 0.0f;
	if (!City->GetPoi(hadi::PoiType::PlayerSpawn, -1, Spawn, Yaw))
	{
		Spawn = FVector(0.0f, 0.0f, 20.0f);
	}
	SpawnHadi(PlayerController, Spawn, Yaw);
	FinishSessionStart(PlayerController);
	ShowBigMessage(LOCTEXT("IntroTitle", "WELCOME HOME"),
		LOCTEXT("IntroSub", "Three years away. One-way ticket back to Vice City. Coral Key, 8:30 AM."), 6.0f);
	UE_LOG(LogHadi, Log, TEXT("New game started"));
}

bool AHadiGameMode::HasSaveGame() const
{
	return UGameplayStatics::DoesSaveGameExist(HadiSave::SlotName, HadiSave::UserIndex);
}

FString AHadiGameMode::GetSaveDescription() const
{
	const UHadiSaveGame* Save = Cast<UHadiSaveGame>(UGameplayStatics::LoadGameFromSlot(HadiSave::SlotName, HadiSave::UserIndex));
	if (!Save)
	{
		return FString();
	}
	return FString::Printf(TEXT("%s  -  $%d  -  %s"), *Save->LocationName, Save->Profile.Money, *Save->SavedAt.ToString(TEXT("%Y-%m-%d %H:%M")));
}

bool AHadiGameMode::ContinueGame(APlayerController* PlayerController)
{
	UHadiGameInstance* GameInstance = GetHadiGameInstance();
	UHadiSaveGame* Save = Cast<UHadiSaveGame>(UGameplayStatics::LoadGameFromSlot(HadiSave::SlotName, HadiSave::UserIndex));
	if (!GameInstance || !Save || !City)
	{
		return false;
	}
	GameInstance->Profile = Save->Profile;
	Environment->SetTimeOfDay(Save->TimeOfDayHours);
	SpawnHadi(PlayerController, Save->PlayerLocation - FVector(0.0f, 0.0f, 90.0f), Save->PlayerYaw);
	if (!Hadi)
	{
		return false;
	}
	Hadi->SetHealthDirect(GameInstance->Profile.Health);
	FinishSessionStart(PlayerController);
	Notify(FText::Format(LOCTEXT("Loaded", "Welcome back. {0}"), FText::FromString(Save->LocationName)));
	return true;
}

bool AHadiGameMode::SaveGame()
{
	UHadiGameInstance* GameInstance = GetHadiGameInstance();
	if (!IsSessionActive() || !GameInstance || Hadi->IsDead())
	{
		return false;
	}
	UHadiSaveGame* Save = Cast<UHadiSaveGame>(UGameplayStatics::CreateSaveGameObject(UHadiSaveGame::StaticClass()));
	GameInstance->Profile.Health = Hadi->GetHealth();
	GameInstance->Profile.OutfitIndex = Hadi->GetOutfit();
	Save->SaveVersion = HadiSave::CurrentVersion;
	Save->Profile = GameInstance->Profile;
	// When saving from a car, store a spot beside it so Hadi loads on foot.
	FVector Location = Hadi->GetActorLocation();
	if (AHadiVehicle* Vehicle = Hadi->GetVehicle())
	{
		Location = Vehicle->GetActorLocation() - Vehicle->GetActorRightVector() * 220.0f + FVector(0.0f, 0.0f, 60.0f);
	}
	Save->PlayerLocation = Location;
	Save->PlayerYaw = static_cast<float>(Hadi->GetActorRotation().Yaw);
	Save->TimeOfDayHours = Environment ? Environment->GetTimeOfDay() : 9.0f;
	Save->SavedAt = FDateTime::Now();
	Save->LocationName = City ? City->GetLocationName(Location) : FString();
	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, HadiSave::SlotName, HadiSave::UserIndex);
	Notify(bSaved ? LOCTEXT("Saved", "Game saved.") : LOCTEXT("SaveFailed", "Saving failed."));
	return bSaved;
}

void AHadiGameMode::ReturnToMainMenu()
{
	if (UHadiGameInstance* GameInstance = GetHadiGameInstance())
	{
		GameInstance->bSessionActive = false;
	}
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

void AHadiGameMode::SleepAtSafehouse()
{
	if (!IsSessionActive())
	{
		return;
	}
	Environment->AdvanceHours(6.0f);
	Hadi->Heal(Hadi->GetMaxHealth());
	ShowBigMessage(LOCTEXT("Slept", "6 HOURS LATER"), FText::FromString(Environment->GetClockText()), 3.0f);
	SaveGame();
}

// ------------------------------------------------------------------------------- economy
int32 AHadiGameMode::GetMoney() const
{
	const UHadiGameInstance* GameInstance = GetHadiGameInstance();
	return GameInstance ? GameInstance->Profile.Money : 0;
}

void AHadiGameMode::AddMoney(int32 Amount)
{
	if (UHadiGameInstance* GameInstance = GetHadiGameInstance())
	{
		GameInstance->Profile.Money += Amount;
		GameInstance->Profile.Stats.MoneyEarned += FMath::Max(0, Amount);
	}
}

bool AHadiGameMode::SpendMoney(int32 Amount)
{
	UHadiGameInstance* GameInstance = GetHadiGameInstance();
	if (!GameInstance || Amount < 0 || GameInstance->Profile.Money < Amount)
	{
		return false;
	}
	GameInstance->Profile.Money -= Amount;
	GameInstance->Profile.Stats.MoneySpent += Amount;
	return true;
}

bool AHadiGameMode::OwnsOutfit(int32 OutfitIndex) const
{
	const UHadiGameInstance* GameInstance = GetHadiGameInstance();
	return GameInstance && GameInstance->Profile.OwnedOutfits.Contains(OutfitIndex);
}

bool AHadiGameMode::BuyOrWearOutfit(AHadiCharacter* Character, int32 OutfitIndex, FText& OutMessage)
{
	UHadiGameInstance* GameInstance = GetHadiGameInstance();
	if (!Character || !GameInstance || OutfitIndex < 0 || OutfitIndex >= hadi::OutfitCount())
	{
		return false;
	}
	const hadi::Outfit& Outfit = hadi::GetOutfit(OutfitIndex);
	const FText Name = FText::FromString(ANSI_TO_TCHAR(Outfit.name));
	if (!OwnsOutfit(OutfitIndex))
	{
		if (!SpendMoney(Outfit.price))
		{
			OutMessage = FText::Format(LOCTEXT("CantAfford", "{0} costs ${1}. Not enough cash."), Name, FText::AsNumber(Outfit.price));
			return false;
		}
		GameInstance->Profile.OwnedOutfits.AddUnique(OutfitIndex);
		GameInstance->Profile.Stats.OutfitsBought++;
		OutMessage = FText::Format(LOCTEXT("Bought", "Bought {0} for ${1}."), Name, FText::AsNumber(Outfit.price));
	}
	else
	{
		OutMessage = FText::Format(LOCTEXT("Wearing", "Now wearing {0}."), Name);
	}
	GameInstance->Profile.OutfitIndex = OutfitIndex;
	Character->SetOutfit(OutfitIndex);
	Character->ShowExpression(hadi::Expression::Happy, 2.0f);
	return true;
}

void AHadiGameMode::CollectFlamingo(int32 Id)
{
	UHadiGameInstance* GameInstance = GetHadiGameInstance();
	if (!GameInstance || GameInstance->Profile.CollectedFlamingos.Contains(Id))
	{
		return;
	}
	GameInstance->Profile.CollectedFlamingos.Add(Id);
	AddMoney(100);
	if (Hadi)
	{
		Hadi->ShowExpression(hadi::Expression::Surprised, 1.2f);
	}
	const int32 Count = GetFlamingoCount();
	if (Count >= GetFlamingoTotal())
	{
		AddMoney(2500);
		ShowBigMessage(LOCTEXT("AllFlamingos", "ALL NEON FLAMINGOS FOUND"), LOCTEXT("AllFlamingosSub", "Coral Key has no secrets left. +$2,500"), 5.0f);
	}
	else
	{
		Notify(FText::Format(LOCTEXT("Flamingo", "Neon Flamingo found! {0}/{1}  +$100"), FText::AsNumber(Count), FText::AsNumber(GetFlamingoTotal())));
	}
}

int32 AHadiGameMode::GetFlamingoCount() const
{
	const UHadiGameInstance* GameInstance = GetHadiGameInstance();
	return GameInstance ? GameInstance->Profile.CollectedFlamingos.Num() : 0;
}

// ---------------------------------------------------------------------------- objectives
void AHadiGameMode::BuildObjectives()
{
	Objectives.Reset();
	FVector Cafe;
	FVector Car;
	FVector Marina;
	float Yaw = 0.0f;
	const bool bCafe = City->GetPoi(hadi::PoiType::Restaurant, hadi::poi_id::CoralCafe, Cafe, Yaw);
	const bool bCar = City->GetPoi(hadi::PoiType::VehicleSpawn, hadi::poi_id::StarterCar, Car, Yaw);
	const bool bMarina = City->GetPoi(hadi::PoiType::MarinaMeet, -1, Marina, Yaw);

	FHadiObjective Breakfast;
	Breakfast.Title = LOCTEXT("PrologueTitle", "Prologue: Fresh Start");
	Breakfast.Description = LOCTEXT("Breakfast", "Grab breakfast at the Coral Cafe on Seabreeze Boulevard.");
	Breakfast.RequiredEvent = EventAteFood;
	Breakfast.bHasTarget = bCafe;
	Breakfast.Target = Cafe;
	Objectives.Add(Breakfast);

	FHadiObjective Ride;
	Ride.Title = Breakfast.Title;
	Ride.Description = LOCTEXT("Ride", "Your old Vento GT is parked outside the cafe. Get in.");
	Ride.RequiredEvent = EventEnteredVehicle;
	Ride.bHasTarget = bCar;
	Ride.Target = Car;
	Objectives.Add(Ride);

	FHadiObjective Meet;
	Meet.Title = Breakfast.Title;
	Meet.Description = LOCTEXT("Meet", "Drive to Marina Row and meet Rafa at Pier 3.");
	Meet.bHasTarget = bMarina;
	Meet.Target = Marina;
	Meet.Radius = 900.0f;
	Meet.Reward = 500;
	Meet.CompletionTitle = LOCTEXT("PrologueDone", "PROLOGUE COMPLETE");
	Meet.CompletionText = LOCTEXT("RafaLine", "Rafa: \"Look who's back. Get some rest, hermano - tomorrow we talk business.\"  +$500");
	Objectives.Add(Meet);
}

bool AHadiGameMode::HasActiveObjective() const
{
	const UHadiGameInstance* GameInstance = GetHadiGameInstance();
	return GameInstance && Objectives.IsValidIndex(GameInstance->Profile.ObjectiveStep);
}

FText AHadiGameMode::GetObjectiveTitle() const
{
	if (HasActiveObjective())
	{
		return Objectives[GetHadiGameInstance()->Profile.ObjectiveStep].Title;
	}
	return LOCTEXT("FreeRoam", "Free roam");
}

FText AHadiGameMode::GetObjectiveText() const
{
	if (HasActiveObjective())
	{
		return Objectives[GetHadiGameInstance()->Profile.ObjectiveStep].Description;
	}
	return FText::Format(LOCTEXT("Explore", "Explore Vice City. Neon Flamingos found: {0}/{1}"), FText::AsNumber(GetFlamingoCount()),
		FText::AsNumber(GetFlamingoTotal()));
}

bool AHadiGameMode::GetObjectiveTarget(FVector& OutTarget) const
{
	if (!HasActiveObjective())
	{
		return false;
	}
	const FHadiObjective& Objective = Objectives[GetHadiGameInstance()->Profile.ObjectiveStep];
	OutTarget = Objective.Target;
	return Objective.bHasTarget;
}

void AHadiGameMode::ReportEvent(FName Event)
{
	if (!IsSessionActive() || !HasActiveObjective())
	{
		return;
	}
	const FHadiObjective& Objective = Objectives[GetHadiGameInstance()->Profile.ObjectiveStep];
	if (!Objective.RequiredEvent.IsNone() && Objective.RequiredEvent == Event)
	{
		CompleteObjective();
	}
}

void AHadiGameMode::CompleteObjective()
{
	UHadiGameInstance* GameInstance = GetHadiGameInstance();
	const FHadiObjective Objective = Objectives[GameInstance->Profile.ObjectiveStep];
	GameInstance->Profile.ObjectiveStep++;
	if (Objective.Reward > 0)
	{
		AddMoney(Objective.Reward);
	}
	if (!Objective.CompletionTitle.IsEmpty())
	{
		ShowBigMessage(Objective.CompletionTitle, Objective.CompletionText, 7.0f);
	}
	else if (HasActiveObjective())
	{
		Notify(GetObjectiveText());
	}
	SaveGame();
}

void AHadiGameMode::UpdateObjectives()
{
	FVector Target;
	const bool bTarget = GetObjectiveTarget(Target);
	if (Waypoint)
	{
		if (bTarget)
		{
			Waypoint->ShowAt(Target);
		}
		else
		{
			Waypoint->HideMarker();
		}
	}
	if (!bTarget || !HasActiveObjective())
	{
		return;
	}
	const FHadiObjective& Objective = Objectives[GetHadiGameInstance()->Profile.ObjectiveStep];
	const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (Objective.RequiredEvent.IsNone() && Pawn && FVector::Dist2D(Pawn->GetActorLocation(), Target) < Objective.Radius)
	{
		CompleteObjective();
	}
}

// ----------------------------------------------------------------------------------- tick
void AHadiGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!IsSessionActive())
	{
		return;
	}
	if (UHadiGameInstance* GameInstance = GetHadiGameInstance())
	{
		GameInstance->Profile.Stats.PlayTimeSeconds += DeltaSeconds;
	}
	UpdateObjectives();
	AutosaveTimer -= DeltaSeconds;
	if (AutosaveTimer <= 0.0f)
	{
		AutosaveTimer = AutosaveIntervalSeconds;
		if (!Hadi->IsDead() && !Hadi->IsSwimming())
		{
			SaveGame();
		}
	}
}

// ---------------------------------------------------------------------------- blackout
void AHadiGameMode::OnPlayerDied(AHadiCharacter* Character)
{
	if (UHadiGameInstance* GameInstance = GetHadiGameInstance())
	{
		GameInstance->Profile.Stats.TimesBlackedOut++;
	}
	ShowBigMessage(LOCTEXT("BlackedOut", "BLACKED OUT"), LOCTEXT("BlackedOutSub", "You wake up outside your apartment. The clinic bill was $100."), 4.0f);
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->FadeOutIn(3.2f);
	}
	(void)Character;
	GetWorldTimerManager().SetTimer(RespawnTimer, this, &AHadiGameMode::RespawnAfterBlackout, 3.5f, false);
}

void AHadiGameMode::RespawnAfterBlackout()
{
	if (!Hadi)
	{
		return;
	}
	FVector Spawn;
	float Yaw = 0.0f;
	if (!City->GetPoi(hadi::PoiType::PlayerSpawn, -1, Spawn, Yaw))
	{
		Spawn = FVector::ZeroVector;
	}
	SpendMoney(FMath::Min(100, GetMoney()));
	Hadi->ReviveAt(Spawn + FVector(0.0f, 0.0f, 95.0f), Yaw, Hadi->GetMaxHealth());
	Environment->AdvanceHours(4.0f);
}

// ---------------------------------------------------------------------------------- HUD
void AHadiGameMode::Notify(const FText& Message)
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->PushNotification(Message);
	}
}

void AHadiGameMode::ShowBigMessage(const FText& Title, const FText& Subtitle, float Duration)
{
	if (AHadiHUD* HUD = GetHadiHUD())
	{
		HUD->ShowBigMessage(Title, Subtitle, Duration);
	}
}

#undef LOCTEXT_NAMESPACE
