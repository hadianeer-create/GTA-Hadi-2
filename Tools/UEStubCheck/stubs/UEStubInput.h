// HADI: VICE CITY - Enhanced Input plugin stub (see CoreMinimal.h for the purpose).
#pragma once

#include "UEStubEngine.h"

struct FKey
{
	FKey() = default;
	explicit FKey(const TCHAR* Name) { (void)Name; }
};

struct EKeys
{
	static const FKey W, A, S, D, E, F, P, V, SpaceBar, LeftShift, LeftControl, Escape, Enter, BackSpace, Up, Down, Left, Right;
	static const FKey Mouse2D, MouseWheelAxis, LeftMouseButton;
	static const FKey Gamepad_Left2D, Gamepad_Right2D, Gamepad_LeftThumbstick, Gamepad_RightThumbstick;
	static const FKey Gamepad_FaceButton_Bottom, Gamepad_FaceButton_Right, Gamepad_FaceButton_Left, Gamepad_FaceButton_Top;
	static const FKey Gamepad_RightTriggerAxis, Gamepad_LeftTriggerAxis, Gamepad_Special_Left, Gamepad_Special_Right;
	static const FKey Gamepad_DPad_Up, Gamepad_DPad_Down, Gamepad_DPad_Left, Gamepad_DPad_Right;
};

enum class EInputActionValueType : uint8
{
	Boolean,
	Axis1D,
	Axis2D,
	Axis3D
};

enum class ETriggerEvent : uint8
{
	None,
	Triggered,
	Started,
	Ongoing,
	Canceled,
	Completed
};

enum class EInputAxisSwizzle : uint8
{
	YXZ,
	ZYX,
	XZY,
	YZX,
	ZXY
};

struct FInputActionValue
{
	template <class T> T Get() const { return T(); }
};

class UInputAction : public UObject
{
public:
	EInputActionValueType ValueType = EInputActionValueType::Boolean;
	bool bTriggerWhenPaused = false;
	bool bConsumeInput = true;
};

class UInputModifier : public UObject
{
};
class UInputModifierNegate : public UInputModifier
{
public:
	bool bX = true;
	bool bY = true;
	bool bZ = true;
};
class UInputModifierSwizzleAxis : public UInputModifier
{
public:
	EInputAxisSwizzle Order = EInputAxisSwizzle::YXZ;
};
class UInputModifierDeadZone : public UInputModifier
{
public:
	float LowerThreshold = 0.2f;
	float UpperThreshold = 1.0f;
};

struct FEnhancedActionKeyMapping
{
	TArray<TObjectPtr<UInputModifier>> Modifiers;
};

class UInputMappingContext : public UObject
{
public:
	FEnhancedActionKeyMapping& MapKey(const UInputAction* Action, FKey Key) { (void)Action; (void)Key; static FEnhancedActionKeyMapping M; return M; }
};

class UEnhancedInputComponent : public UInputComponent
{
public:
	template <class UserClass>
	void BindAction(const UInputAction* Action, ETriggerEvent Event, UserClass* Object, void (UserClass::*Func)(const FInputActionValue&))
	{
		(void)Action; (void)Event; (void)Object; (void)Func;
	}
};

class UEnhancedInputLocalPlayerSubsystem : public UObject
{
public:
	void AddMappingContext(const UInputMappingContext* Context, int32 Priority) { (void)Context; (void)Priority; }
};
