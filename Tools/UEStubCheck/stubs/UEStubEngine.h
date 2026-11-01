// HADI: VICE CITY - Unreal Engine module stub (see CoreMinimal.h for the purpose).
#pragma once

#include "CoreMinimal.h"

class AActor;
class AController;
class AHUD;
class APawn;
class APlayerCameraManager;
class APlayerController;
class UActorComponent;
class UCameraComponent;
class UCanvas;
class UFont;
class UGameInstance;
class UGameUserSettings;
class UInputComponent;
class ULocalPlayer;
class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UTexture;
class UWorld;

// ----------------------------------------------------------------------------- engine enums
enum ECollisionChannel
{
	ECC_WorldStatic,
	ECC_WorldDynamic,
	ECC_Pawn,
	ECC_Visibility,
	ECC_Camera
};
namespace ECollisionEnabled
{
enum Type
{
	NoCollision,
	QueryOnly,
	PhysicsOnly,
	QueryAndPhysics
};
}
namespace EComponentMobility
{
enum Type
{
	Static,
	Stationary,
	Movable
};
}
enum class ETeleportType : uint8
{
	None,
	TeleportPhysics,
	ResetPhysics
};
enum EMovementMode
{
	MOVE_None,
	MOVE_Walking,
	MOVE_NavWalking,
	MOVE_Falling,
	MOVE_Swimming,
	MOVE_Flying,
	MOVE_Custom
};
enum EViewTargetBlendFunction
{
	VTBlend_Linear,
	VTBlend_Cubic,
	VTBlend_EaseIn,
	VTBlend_EaseOut,
	VTBlend_EaseInOut
};
enum class ELightUnits : uint8
{
	Unitless,
	Candelas,
	Lumens
};
enum ESkyLightSourceType
{
	SLS_CapturedScene,
	SLS_SpecifiedCubemap
};
namespace EWindowMode
{
enum Type
{
	Fullscreen,
	WindowedFullscreen,
	Windowed
};
inline Type ConvertIntToWindowMode(int32 In) { return static_cast<Type>(In); }
}
enum class EMouseLockMode : uint8
{
	DoNotLock,
	LockOnCapture,
	LockAlways
};
namespace EQuitPreference
{
enum Type
{
	Quit,
	Background
};
}
namespace EEndPlayReason
{
enum Type
{
	Destroyed,
	LevelTransition,
	EndPlayInEditor,
	RemovedFromWorld,
	Quit
};
}
enum class EAutoPossessAI : uint8
{
	Disabled,
	PlacedInWorld,
	Spawned,
	PlacedInWorldOrSpawned
};
enum ETickingGroup
{
	TG_PrePhysics,
	TG_DuringPhysics,
	TG_PostPhysics
};
enum EBlendMode
{
	BLEND_Opaque,
	BLEND_Masked,
	BLEND_Translucent
};
enum ESimpleElementBlendMode
{
	SE_BLEND_Opaque,
	SE_BLEND_Masked,
	SE_BLEND_Translucent
};
enum EMaterialDomain
{
	MD_Surface
};
enum class ESpawnActorCollisionHandlingMethod : uint8
{
	Undefined,
	AlwaysSpawn,
	AdjustIfPossibleButAlwaysSpawn,
	AdjustIfPossibleButDontSpawnIfColliding,
	DontSpawnIfColliding
};
enum EConsoleVariableFlags
{
	ECVF_Default = 0,
	ECVF_SetByGameSetting = 0x04000000
};

struct FTickFunction
{
	uint8 bCanEverTick : 1;
	uint8 bTickEvenWhenPaused : 1;
	TEnumAsByte<ETickingGroup> TickGroup;
};
struct FActorTickFunction : FTickFunction
{
};
struct FActorComponentTickFunction : FTickFunction
{
};

struct FHitResult
{
	float Distance = 0.0f;
	FVector ImpactPoint;
	FVector ImpactNormal;
	FVector Location;
	uint8 bBlockingHit : 1;
	uint8 bStartPenetrating : 1;
};

struct FCollisionQueryParams
{
	FCollisionQueryParams(FName TraceTag, bool bTraceComplex = false, const AActor* IgnoreActor = nullptr) { (void)TraceTag; (void)bTraceComplex; (void)IgnoreActor; }
	void AddIgnoredActor(const AActor* Actor) { (void)Actor; }
};

struct FCollisionShape
{
	static FCollisionShape MakeCapsule(float Radius, float HalfHeight) { (void)Radius; (void)HalfHeight; return FCollisionShape(); }
	static FCollisionShape MakeSphere(float Radius) { (void)Radius; return FCollisionShape(); }
};

struct FAttachmentTransformRules
{
	static const FAttachmentTransformRules KeepRelativeTransform;
	static const FAttachmentTransformRules SnapToTargetNotIncludingScale;
};
struct FDetachmentTransformRules
{
	static const FDetachmentTransformRules KeepWorldTransform;
};

class UCollisionProfile
{
public:
	static const FName BlockAll_ProfileName;
	static const FName Vehicle_ProfileName;
	static const FName NoCollision_ProfileName;
};

struct FTimerHandle
{
};

class FTimerManager
{
public:
	template <class UserClass>
	void SetTimer(FTimerHandle& Handle, UserClass* Object, void (UserClass::*Method)(), float Rate, bool bLoop = false, float FirstDelay = -1.0f)
	{
		(void)Handle; (void)Object; (void)Method; (void)Rate; (void)bLoop; (void)FirstDelay;
	}
	void ClearTimer(FTimerHandle& Handle) { (void)Handle; }
};

struct FActorSpawnParameters
{
	AActor* Owner = nullptr;
	ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;
};

// --------------------------------------------------------------------------------- assets
class UStaticMesh : public UObject
{
};
class UMaterialInterface : public UObject
{
};
class UMaterial : public UMaterialInterface
{
public:
	static UMaterial* GetDefaultMaterial(EMaterialDomain Domain) { (void)Domain; return nullptr; }
};
class UMaterialInstanceDynamic : public UMaterialInterface
{
public:
	static UMaterialInstanceDynamic* Create(UMaterialInterface* Parent, UObject* Outer) { (void)Parent; (void)Outer; return nullptr; }
	void SetVectorParameterValue(FName Name, const FLinearColor& Value) { (void)Name; (void)Value; }
	void SetScalarParameterValue(FName Name, float Value) { (void)Name; (void)Value; }
};
class UTexture : public UObject
{
};
class UFont : public UObject
{
public:
	int32 LegacyFontSize = 9;
};

// ----------------------------------------------------------------------------- components
class UActorComponent : public UObject
{
public:
	FActorComponentTickFunction PrimaryComponentTick;
	void RegisterComponent() {}
	virtual void SetActive(bool bNewActive, bool bReset = false) { (void)bNewActive; (void)bReset; }
	void SetAutoActivate(bool b) { (void)b; }
	AActor* GetOwner() const { return nullptr; }
	virtual void BeginPlay() {}
};

class USceneComponent : public UActorComponent
{
public:
	static const FName SocketNameDummy;
	void SetupAttachment(USceneComponent* Parent, FName Socket = NAME_None) { (void)Parent; (void)Socket; }
	void SetRelativeLocation(const FVector& V) { (void)V; }
	void SetRelativeRotation(const FRotator& R) { (void)R; }
	void SetRelativeScale3D(const FVector& S) { (void)S; }
	void SetRelativeLocationAndRotation(const FVector& V, const FRotator& R) { (void)V; (void)R; }
	void SetWorldLocation(const FVector& V) { (void)V; }
	void SetWorldRotation(const FRotator& R) { (void)R; }
	void SetMobility(EComponentMobility::Type M) { (void)M; }
	void SetVisibility(bool bVisible, bool bPropagate = false) { (void)bVisible; (void)bPropagate; }
	bool IsVisible() const { return true; }
	const FTransform& GetComponentTransform() const { static FTransform T; return T; }
	FVector GetComponentLocation() const { return FVector(); }
};

class UPrimitiveComponent : public USceneComponent
{
public:
	uint8 bReceivesDecals : 1;
	void SetCollisionEnabled(ECollisionEnabled::Type T) { (void)T; }
	void SetCollisionProfileName(FName Name) { (void)Name; }
	void SetGenerateOverlapEvents(bool b) { (void)b; }
	void SetCanEverAffectNavigation(bool b) { (void)b; }
	void SetCastShadow(bool b) { (void)b; }
	void SetSimulatePhysics(bool b) { (void)b; }
	void SetLinearDamping(float D) { (void)D; }
	void SetAngularDamping(float D) { (void)D; }
	void SetMassOverrideInKg(FName Bone, float Mass, bool bOverride = true) { (void)Bone; (void)Mass; (void)bOverride; }
	void SetCenterOfMass(FVector Offset, FName Bone = NAME_None) { (void)Offset; (void)Bone; }
	float GetMass() const { return 1.0f; }
	void AddForce(FVector Force, FName Bone = NAME_None, bool bAccelChange = false) { (void)Force; (void)Bone; (void)bAccelChange; }
	void AddForceAtLocation(FVector Force, FVector Location, FName Bone = NAME_None) { (void)Force; (void)Location; (void)Bone; }
	FVector GetPhysicsLinearVelocity(FName Bone = NAME_None) { (void)Bone; return FVector(); }
	FVector GetPhysicsLinearVelocityAtPoint(FVector Point, FName Bone = NAME_None) { (void)Point; (void)Bone; return FVector(); }
	void SetPhysicsLinearVelocity(FVector V, bool bAdd = false, FName Bone = NAME_None) { (void)V; (void)bAdd; (void)Bone; }
	void SetPhysicsAngularVelocityInDegrees(FVector V, bool bAdd = false, FName Bone = NAME_None) { (void)V; (void)bAdd; (void)Bone; }
	void WakeAllRigidBodies() {}
	bool IsSimulatingPhysics(FName Bone = NAME_None) const { (void)Bone; return true; }
	virtual void SetMaterial(int32 Index, UMaterialInterface* Material) { (void)Index; (void)Material; }
};

class UStaticMeshComponent : public UPrimitiveComponent
{
public:
	virtual bool SetStaticMesh(UStaticMesh* Mesh) { (void)Mesh; return true; }
};

class UInstancedStaticMeshComponent : public UStaticMeshComponent
{
public:
	virtual TArray<int32> AddInstances(const TArray<FTransform>& Transforms, bool bShouldReturnIndices, bool bWorldSpace = false, bool bUpdateNavigation = true)
	{
		(void)Transforms; (void)bShouldReturnIndices; (void)bWorldSpace; (void)bUpdateNavigation;
		return TArray<int32>();
	}
	void SetCullDistances(int32 Start, int32 End) { (void)Start; (void)End; }
};

class UShapeComponent : public UPrimitiveComponent
{
};
class UBoxComponent : public UShapeComponent
{
public:
	void InitBoxExtent(const FVector& Extent) { (void)Extent; }
};
class UCapsuleComponent : public UShapeComponent
{
public:
	void InitCapsuleSize(float Radius, float HalfHeight) { (void)Radius; (void)HalfHeight; }
	float GetScaledCapsuleRadius() const { return 34.0f; }
	float GetScaledCapsuleHalfHeight() const { return 90.0f; }
};

class UCameraComponent : public USceneComponent
{
public:
	float FieldOfView = 90.0f;
	uint8 bUsePawnControlRotation : 1;
	uint8 bConstrainAspectRatio : 1;
	virtual void SetFieldOfView(float Fov) { FieldOfView = Fov; }
};

class USpringArmComponent : public USceneComponent
{
public:
	static const FName SocketName;
	float TargetArmLength = 300.0f;
	FVector SocketOffset;
	FVector TargetOffset;
	float ProbeSize = 12.0f;
	float CameraLagSpeed = 10.0f;
	uint8 bUsePawnControlRotation : 1;
	uint8 bEnableCameraLag : 1;
	uint8 bInheritPitch : 1;
	uint8 bInheritYaw : 1;
	uint8 bInheritRoll : 1;
};

class UMovementComponent : public UActorComponent
{
public:
	FVector Velocity;
	virtual void StopMovementImmediately() {}
};

class UCharacterMovementComponent : public UMovementComponent
{
public:
	uint8 bOrientRotationToMovement : 1;
	FRotator RotationRate;
	float JumpZVelocity = 420.0f;
	float AirControl = 0.05f;
	float MaxWalkSpeed = 600.0f;
	float MinAnalogWalkSpeed = 0.0f;
	float BrakingDecelerationWalking = 2048.0f;
	float BrakingDecelerationFlying = 0.0f;
	float GroundFriction = 8.0f;
	float MaxStepHeight = 45.0f;
	float MaxFlySpeed = 600.0f;
	void SetWalkableFloorAngle(float Angle) { (void)Angle; }
	bool IsMovingOnGround() const { return true; }
	bool IsFalling() const { return false; }
	virtual void SetMovementMode(EMovementMode Mode, uint8 Custom = 0) { (void)Mode; (void)Custom; }
	virtual void DisableMovement() {}
};

class ULightComponentBase : public USceneComponent
{
public:
	void SetCastShadows(bool b) { (void)b; }
};
class ULightComponent : public ULightComponentBase
{
public:
	void SetIntensity(float I) { (void)I; }
	void SetLightColor(FLinearColor C, bool bSRGB = true) { (void)C; (void)bSRGB; }
	void SetUseTemperature(bool b) { (void)b; }
	void SetTemperature(float T) { (void)T; }
};
class ULocalLightComponent : public ULightComponent
{
public:
	void SetIntensityUnits(ELightUnits Units) { (void)Units; }
	void SetAttenuationRadius(float R) { (void)R; }
};
class UPointLightComponent : public ULocalLightComponent
{
public:
	void SetSourceRadius(float R) { (void)R; }
};
class USpotLightComponent : public UPointLightComponent
{
public:
	void SetInnerConeAngle(float A) { (void)A; }
	void SetOuterConeAngle(float A) { (void)A; }
};
class UDirectionalLightComponent : public ULightComponent
{
public:
	void SetLightSourceAngle(float A) { (void)A; }
	void SetAtmosphereSunLight(bool b) { (void)b; }
	void SetAtmosphereSunLightIndex(int32 I) { (void)I; }
};
class USkyLightComponent : public ULightComponentBase
{
public:
	bool bRealTimeCapture = false;
	TEnumAsByte<ESkyLightSourceType> SourceType;
	void SetIntensity(float I) { (void)I; }
	void RecaptureSky() {}
};
class USkyAtmosphereComponent : public USceneComponent
{
};
class UVolumetricCloudComponent : public USceneComponent
{
};
class UExponentialHeightFogComponent : public USceneComponent
{
public:
	void SetFogDensity(float V) { (void)V; }
	void SetFogHeightFalloff(float V) { (void)V; }
	void SetVolumetricFog(bool b) { (void)b; }
};

struct FPostProcessSettings
{
	uint8 bOverride_AutoExposureMinBrightness : 1;
	uint8 bOverride_AutoExposureMaxBrightness : 1;
	uint8 bOverride_AutoExposureSpeedUp : 1;
	uint8 bOverride_AutoExposureSpeedDown : 1;
	uint8 bOverride_BloomIntensity : 1;
	uint8 bOverride_VignetteIntensity : 1;
	uint8 bOverride_FilmGrainIntensity : 1;
	uint8 bOverride_ColorSaturation : 1;
	uint8 bOverride_MotionBlurAmount : 1;
	float AutoExposureMinBrightness = 0;
	float AutoExposureMaxBrightness = 0;
	float AutoExposureSpeedUp = 0;
	float AutoExposureSpeedDown = 0;
	float BloomIntensity = 0;
	float VignetteIntensity = 0;
	float FilmGrainIntensity = 0;
	FVector4 ColorSaturation;
	float MotionBlurAmount = 0;
};
class UPostProcessComponent : public USceneComponent
{
public:
	FPostProcessSettings Settings;
	float Priority = 0.0f;
	uint32 bUnbound : 1;
};

// ---------------------------------------------------------------------------------- actors
class UGameInstance : public UObject
{
public:
	virtual void Init() {}
};

class AActor : public UObject
{
public:
	FActorTickFunction PrimaryActorTick;
	TObjectPtr<USceneComponent> RootComponent;
	USceneComponent* GetRootComponent() const { return RootComponent; }
	FVector GetActorLocation() const { return FVector(); }
	FRotator GetActorRotation() const { return FRotator(); }
	FVector GetActorForwardVector() const { return FVector(); }
	FVector GetActorRightVector() const { return FVector(); }
	virtual FVector GetVelocity() const { return FVector(); }
	bool SetActorLocation(const FVector& V, bool bSweep = false, FHitResult* OutHit = nullptr, ETeleportType Teleport = ETeleportType::None)
	{
		(void)V; (void)bSweep; (void)OutHit; (void)Teleport;
		return true;
	}
	bool SetActorLocationAndRotation(FVector V, FRotator R, bool bSweep = false, FHitResult* OutHit = nullptr, ETeleportType Teleport = ETeleportType::None)
	{
		(void)V; (void)R; (void)bSweep; (void)OutHit; (void)Teleport;
		return true;
	}
	bool SetActorRotation(FRotator R) { (void)R; return true; }
	void SetActorRelativeLocation(FVector V) { (void)V; }
	void SetActorRelativeRotation(FRotator R) { (void)R; }
	virtual void SetActorHiddenInGame(bool b) { (void)b; }
	void SetActorTickEnabled(bool b) { (void)b; }
	void SetActorEnableCollision(bool b) { (void)b; }
	bool AttachToComponent(USceneComponent* Parent, const FAttachmentTransformRules& Rules, FName Socket = NAME_None) { (void)Parent; (void)Rules; (void)Socket; return true; }
	void DetachFromActor(const FDetachmentTransformRules& Rules) { (void)Rules; }
	virtual bool Destroy(bool bNetForce = false, bool bShouldModifyLevel = true) { (void)bNetForce; (void)bShouldModifyLevel; return true; }
	void AddInstanceComponent(UActorComponent* Component) { (void)Component; }
	template <class T> T* GetGameInstance() const { return nullptr; }
	FTimerManager& GetWorldTimerManager() const { static FTimerManager M; return M; }
	virtual UWorld* GetWorld() const override { return nullptr; }
	virtual void Tick(float DeltaSeconds) { (void)DeltaSeconds; }

protected:
	virtual void BeginPlay() {}
	virtual void EndPlay(const EEndPlayReason::Type Reason) { (void)Reason; }
};

class APawn : public AActor
{
public:
	EAutoPossessAI AutoPossessAI = EAutoPossessAI::Disabled;
	uint32 bUseControllerRotationPitch : 1;
	uint32 bUseControllerRotationYaw : 1;
	uint32 bUseControllerRotationRoll : 1;
	TObjectPtr<AController> Controller;
	AController* GetController() const { return Controller; }
	virtual void AddMovementInput(FVector Direction, float Scale = 1.0f, bool bForce = false) { (void)Direction; (void)Scale; (void)bForce; }
};

class USkeletalMeshComponent : public UPrimitiveComponent
{
};

class ACharacter : public APawn
{
public:
	UCharacterMovementComponent* GetCharacterMovement() const { return nullptr; }
	UCapsuleComponent* GetCapsuleComponent() const { return nullptr; }
	USkeletalMeshComponent* GetMesh() const { return nullptr; }
	virtual void Jump() {}
	virtual void StopJumping() {}
	virtual void Landed(const FHitResult& Hit) { (void)Hit; }
};

class AController : public AActor
{
public:
	void Possess(APawn* Pawn) { (void)Pawn; }
	APawn* GetPawn() const { return nullptr; }
	virtual FRotator GetControlRotation() const { return FRotator(); }
	virtual void SetControlRotation(const FRotator& R) { (void)R; }
	bool IsLocalController() const { return true; }

protected:
	virtual void OnPossess(APawn* Pawn) { (void)Pawn; }
};

class APlayerCameraManager : public AActor
{
public:
	float ViewPitchMin = -89.9f;
	float ViewPitchMax = 89.9f;
	FVector GetCameraLocation() const { return FVector(); }
	FRotator GetCameraRotation() const { return FRotator(); }
};

class UInputComponent : public UActorComponent
{
};

struct FInputModeDataBase
{
	virtual ~FInputModeDataBase() = default;
};
struct FInputModeGameOnly : FInputModeDataBase
{
};
struct FInputModeGameAndUI : FInputModeDataBase
{
	FInputModeGameAndUI& SetHideCursorDuringCapture(bool b) { (void)b; return *this; }
	FInputModeGameAndUI& SetLockMouseToViewportBehavior(EMouseLockMode M) { (void)M; return *this; }
};

class ULocalPlayer : public UObject
{
public:
	template <class T> static T* GetSubsystem(const ULocalPlayer* Player) { (void)Player; return nullptr; }
};

class APlayerController : public AController
{
public:
	TObjectPtr<UInputComponent> InputComponent;
	TObjectPtr<APlayerCameraManager> PlayerCameraManager;
	uint32 bShowMouseCursor : 1;
	uint32 bEnableClickEvents : 1;
	virtual void SetupInputComponent() {}
	virtual void PlayerTick(float DeltaTime) { (void)DeltaTime; }
	virtual void AddYawInput(float Val) { (void)Val; }
	virtual void AddPitchInput(float Val) { (void)Val; }
	virtual void SetViewTarget(AActor* Target) { (void)Target; }
	void SetViewTargetWithBlend(AActor* Target, float BlendTime = 0, EViewTargetBlendFunction Func = VTBlend_Linear, float Exp = 0, bool bLock = false)
	{
		(void)Target; (void)BlendTime; (void)Func; (void)Exp; (void)bLock;
	}
	virtual bool SetPause(bool bPause) { (void)bPause; return true; }
	virtual void SetInputMode(const FInputModeDataBase& Mode) { (void)Mode; }
	bool GetMousePosition(float& X, float& Y) const { X = 0; Y = 0; return true; }
	template <class T> T* GetHUD() const { return nullptr; }
	ULocalPlayer* GetLocalPlayer() const { return nullptr; }
};

class ACameraActor : public AActor
{
public:
	UCameraComponent* GetCameraComponent() const { return nullptr; }
};

class AGameModeBase : public AActor
{
public:
	TSubclassOf<APlayerController> PlayerControllerClass;
	TSubclassOf<AHUD> HUDClass;
	TSubclassOf<APawn> DefaultPawnClass;
	virtual void StartPlay() {}
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) { (void)NewPlayer; }
};

// ------------------------------------------------------------------------------------ world
class UWorldSubsystem : public UObject
{
};

class UWorld : public UObject
{
public:
	template <class T> T* SpawnActor(UClass* Class, const FTransform& Transform, const FActorSpawnParameters& Params = FActorSpawnParameters())
	{
		(void)Class; (void)Transform; (void)Params;
		return nullptr;
	}
	template <class T> T* SpawnActor(UClass* Class, const FVector& Location, const FRotator& Rotation, const FActorSpawnParameters& Params = FActorSpawnParameters())
	{
		(void)Class; (void)Location; (void)Rotation; (void)Params;
		return nullptr;
	}
	template <class T> T* GetSubsystem() const { return nullptr; }
	template <class T> T* GetAuthGameMode() const { return nullptr; }
	APlayerController* GetFirstPlayerController() const { return nullptr; }
	float GetDeltaSeconds() const { return 0.016f; }
	bool LineTraceSingleByChannel(FHitResult& Out, const FVector& Start, const FVector& End, ECollisionChannel Channel, const FCollisionQueryParams& Params) const
	{
		(void)Out; (void)Start; (void)End; (void)Channel; (void)Params;
		return false;
	}
	bool OverlapBlockingTestByChannel(const FVector& Pos, const FQuat& Rot, ECollisionChannel Channel, const FCollisionShape& Shape, const FCollisionQueryParams& Params) const
	{
		(void)Pos; (void)Rot; (void)Channel; (void)Shape; (void)Params;
		return false;
	}
};

template <class T>
class TActorIterator
{
public:
	explicit TActorIterator(UWorld* World) { (void)World; }
	explicit operator bool() const { return false; }
	TActorIterator& operator++() { return *this; }
	T* operator*() const { return nullptr; }
};

// ------------------------------------------------------------------------------ canvas / UI
class FTexture
{
};
extern FTexture* GWhiteTexture;

struct FCanvasItem
{
	virtual ~FCanvasItem() = default;
	virtual void SetColor(const FLinearColor& C) { (void)C; }
	ESimpleElementBlendMode BlendMode = SE_BLEND_Translucent;
};
struct FCanvasTileItem : FCanvasItem
{
	FCanvasTileItem(const FVector2D& Pos, const FVector2D& Size, const FLinearColor& Color) { (void)Pos; (void)Size; (void)Color; }
	FRotator Rotation;
	FVector2D PivotPoint;
};
struct FCanvasTextItem : FCanvasItem
{
	FCanvasTextItem(const FVector2D& Pos, const FText& Text, const UFont* Font, const FLinearColor& Color) { (void)Pos; (void)Text; (void)Font; (void)Color; }
	FVector2D Scale;
	void EnableShadow(const FLinearColor& Color, const FVector2D& Offset = FVector2D(1.0, 1.0)) { (void)Color; (void)Offset; }
};
struct FCanvasLineItem : FCanvasItem
{
	FCanvasLineItem(const FVector2D& Start, const FVector2D& End) { (void)Start; (void)End; }
	float LineThickness = 0.0f;
};
struct FCanvasTriangleItem : FCanvasItem
{
	FCanvasTriangleItem(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FTexture* Texture) { (void)A; (void)B; (void)C; (void)Texture; }
};

class UCanvas : public UObject
{
public:
	float ClipX = 1920.0f;
	float ClipY = 1080.0f;
	void DrawItem(FCanvasItem& Item) { (void)Item; }
	void TextSize(const UFont* Font, const FString& Text, float& XL, float& YL, float ScaleX = 1.0f, float ScaleY = 1.0f)
	{
		(void)Font; (void)Text; (void)ScaleX; (void)ScaleY;
		XL = 0; YL = 0;
	}
};

class AHUD : public AActor
{
public:
	TObjectPtr<UCanvas> Canvas;
	TObjectPtr<APlayerController> PlayerOwner;
	virtual void DrawHUD() {}
	void DrawTexture(UTexture* Texture, float X, float Y, float W, float H, float U, float V, float UW, float VH, FLinearColor Tint = FLinearColor::White,
		EBlendMode Blend = BLEND_Translucent, float Scale = 1.f, bool bScalePosition = false, float Rotation = 0.f, FVector2D RotPivot = FVector2D::ZeroVector)
	{
		(void)Texture; (void)X; (void)Y; (void)W; (void)H; (void)U; (void)V; (void)UW; (void)VH; (void)Tint; (void)Blend; (void)Scale; (void)bScalePosition; (void)Rotation; (void)RotPivot;
	}
};

class UTextureRenderTarget2D : public UTexture
{
public:
	FLinearColor ClearColor;
	void UpdateResource() {}
};

struct FOnCanvasRenderTargetUpdate
{
	template <class U>
	void __Internal_AddDynamic(U* Object, void (U::*Func)(UCanvas*, int32, int32), FName Name) { (void)Object; (void)Func; (void)Name; }
};
#define AddDynamic(UserObject, FuncName) __Internal_AddDynamic(UserObject, FuncName, FName(TEXT(#FuncName)))

class UCanvasRenderTarget2D : public UTextureRenderTarget2D
{
public:
	FOnCanvasRenderTargetUpdate OnCanvasRenderTargetUpdate;
	static UCanvasRenderTarget2D* CreateCanvasRenderTarget2D(UObject* Context, TSubclassOf<UCanvasRenderTarget2D> Class, int32 W = 1024, int32 H = 1024)
	{
		(void)Context; (void)Class; (void)W; (void)H;
		return nullptr;
	}
	static UClass* StaticClass();
};

// --------------------------------------------------------------------------- game systems
class USaveGame : public UObject
{
};

class UGameUserSettings : public UObject
{
public:
	int32 GetOverallScalabilityLevel() const { return 0; }
	void SetOverallScalabilityLevel(int32 Level) { (void)Level; }
	void ApplyNonResolutionSettings() {}
	void ApplyResolutionSettings(bool bCheck) { (void)bCheck; }
	EWindowMode::Type GetFullscreenMode() const { return EWindowMode::Windowed; }
	void SetFullscreenMode(EWindowMode::Type Mode) { (void)Mode; }
	FIntPoint GetScreenResolution() const { return FIntPoint(); }
	void SetScreenResolution(FIntPoint R) { (void)R; }
	bool IsVSyncEnabled() const { return false; }
	void SetVSyncEnabled(bool b) { (void)b; }
	float GetFrameRateLimit() const { return 0.0f; }
	void SetFrameRateLimit(float L) { (void)L; }
	virtual void SaveSettings() {}
	void RunHardwareBenchmark(int32 WorkScale = 10, float CPU = 1.0f, float GPU = 1.0f) { (void)WorkScale; (void)CPU; (void)GPU; }
	void ApplyHardwareBenchmarkResults() {}
};

class UEngine : public UObject
{
public:
	UGameUserSettings* GetGameUserSettings() { return nullptr; }
	UFont* GetLargeFont() { return nullptr; }
	UFont* GetMediumFont() { return nullptr; }
	UFont* GetSmallFont() { return nullptr; }
};
extern UEngine* GEngine;

class UGameplayStatics : public UObject
{
public:
	static APawn* GetPlayerPawn(const UObject* Context, int32 Index) { (void)Context; (void)Index; return nullptr; }
	static bool DoesSaveGameExist(const FString& Slot, int32 User) { (void)Slot; (void)User; return false; }
	static USaveGame* LoadGameFromSlot(const FString& Slot, int32 User) { (void)Slot; (void)User; return nullptr; }
	static bool SaveGameToSlot(USaveGame* Save, const FString& Slot, int32 User) { (void)Save; (void)Slot; (void)User; return true; }
	static USaveGame* CreateSaveGameObject(TSubclassOf<USaveGame> Class) { (void)Class; return nullptr; }
	static FString GetCurrentLevelName(const UObject* Context, bool bRemovePrefix = true) { (void)Context; (void)bRemovePrefix; return FString(); }
	static void OpenLevel(const UObject* Context, FName Level, bool bAbsolute = true, FString Options = FString()) { (void)Context; (void)Level; (void)bAbsolute; (void)Options; }
};

class UKismetSystemLibrary : public UObject
{
public:
	static void QuitGame(const UObject* Context, APlayerController* Player, EQuitPreference::Type Pref, bool bIgnore) { (void)Context; (void)Player; (void)Pref; (void)bIgnore; }
	static bool GetSupportedFullscreenResolutions(TArray<FIntPoint>& Out) { (void)Out; return true; }
};

struct FApp
{
	static double GetDeltaTime() { return 0.016; }
};
struct FPlatformTime
{
	static double Seconds() { return 0.0; }
};
struct FPackageName
{
	static bool DoesPackageExist(const FString& Path) { (void)Path; return false; }
};

class IConsoleVariable
{
public:
	virtual ~IConsoleVariable() = default;
	virtual void Set(int32 Value, EConsoleVariableFlags SetBy = ECVF_Default) { (void)Value; (void)SetBy; }
};
class IConsoleManager
{
public:
	static IConsoleManager& Get() { static IConsoleManager M; return M; }
	IConsoleVariable* FindConsoleVariable(const TCHAR* Name) const { (void)Name; return nullptr; }
};

class FDefaultGameModuleImpl
{
};
