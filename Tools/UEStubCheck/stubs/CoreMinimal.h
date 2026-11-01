// HADI: VICE CITY - Unreal API *stub* used only for offline syntax / consistency checking.
// It declares the subset of the engine API this project uses, with the same signatures as UE 5.x,
// so clang can catch typos, header/source mismatches, shadowing (an error in UE5) and float/double
// ambiguity without the engine. It is NOT a substitute for a real UnrealBuildTool compile.
#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>
#include <limits>

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
typedef wchar_t TCHAR;
typedef char ANSICHAR;

#define TEXT(x) L##x
#define UCLASS(...)
#define USTRUCT(...)
#define UENUM(...)
#define UPROPERTY(...)
#define UFUNCTION(...)
#define UINTERFACE(...)
#define UMETA(...)
#define GENERATED_BODY()
#define HADIVICECITY_API
#define UE_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define UE_KINDA_SMALL_NUMBER (1.e-4f)
#define UE_SMALL_NUMBER (1.e-8f)
#define UE_PI (3.1415926535897932f)
#define INDEX_NONE (-1)
#define IMPLEMENT_PRIMARY_GAME_MODULE(Impl, Module, Name) static int HadiStubModule_##Module = 0;
#define LOCTEXT(Key, Text) FText::FromString(FString(TEXT(Text)))
#define NSLOCTEXT(Ns, Key, Text) FText::FromString(FString(TEXT(Text)))

template <typename T>
using TFunction = std::function<T>;

template <typename T>
inline typename std::remove_reference<T>::type&& MoveTemp(T&& Value)
{
	return static_cast<typename std::remove_reference<T>::type&&>(Value);
}

template <typename T>
struct TNumericLimits
{
	static constexpr T Max() { return std::numeric_limits<T>::max(); }
	static constexpr T Lowest() { return std::numeric_limits<T>::lowest(); }
};

// ------------------------------------------------------------------------------- containers
template <typename T>
class TArray
{
public:
	TArray() = default;
	TArray(std::initializer_list<T> List) : Data(List) {}
	int32 Num() const { return static_cast<int32>(Data.size()); }
	bool IsValidIndex(int32 Index) const { return Index >= 0 && Index < Num(); }
	int32 Add(const T& Item) { Data.push_back(Item); return Num() - 1; }
	int32 Add(T&& Item) { Data.push_back(std::move(Item)); return Num() - 1; }
	int32 AddUnique(const T& Item) { for (int32 i = 0; i < Num(); ++i) { if (Data[i] == Item) { return i; } } return Add(Item); }
	template <typename... Args>
	int32 Emplace(Args&&... A) { Data.emplace_back(std::forward<Args>(A)...); return Num() - 1; }
	void Reset() { Data.clear(); }
	void Empty() { Data.clear(); }
	void Reserve(int32 N) { Data.reserve(static_cast<size_t>(N)); }
	void SetNum(int32 N) { Data.resize(static_cast<size_t>(N)); }
	template <typename U>
	int32 Remove(const U& Item) { const size_t Before = Data.size(); Data.erase(std::remove(Data.begin(), Data.end(), Item), Data.end()); return static_cast<int32>(Before - Data.size()); }
	void RemoveAt(int32 Index) { Data.erase(Data.begin() + Index); }
	template <typename U>
	bool Contains(const U& Item) const { for (const T& E : Data) { if (E == Item) { return true; } } return false; }
	template <typename U>
	int32 IndexOfByKey(const U& Key) const { for (int32 i = 0; i < Num(); ++i) { if (Data[static_cast<size_t>(i)] == Key) { return i; } } return INDEX_NONE; }
	T& Last() { return Data.back(); }
	const T& Last() const { return Data.back(); }
	T Pop() { T V = std::move(Data.back()); Data.pop_back(); return V; }
	template <typename P>
	void Sort(P Predicate) { std::sort(Data.begin(), Data.end(), Predicate); }
	T& operator[](int32 Index) { return Data[static_cast<size_t>(Index)]; }
	const T& operator[](int32 Index) const { return Data[static_cast<size_t>(Index)]; }
	T* GetData() { return Data.data(); }
	typename std::vector<T>::iterator begin() { return Data.begin(); }
	typename std::vector<T>::iterator end() { return Data.end(); }
	typename std::vector<T>::const_iterator begin() const { return Data.begin(); }
	typename std::vector<T>::const_iterator end() const { return Data.end(); }

private:
	std::vector<T> Data;
};

template <typename K, typename V>
struct TPair
{
	K Key;
	V Value;
};

template <typename K, typename V>
class TMap
{
public:
	V* Find(const K& Key) { for (auto& P : Data) { if (P.Key == Key) { return &P.Value; } } return nullptr; }
	const V* Find(const K& Key) const { for (const auto& P : Data) { if (P.Key == Key) { return &P.Value; } } return nullptr; }
	V& Add(const K& Key, const V& Value) { Data.push_back(TPair<K, V>{Key, Value}); return Data.back().Value; }
	V& FindOrAdd(const K& Key) { if (V* Found = Find(Key)) { return *Found; } Data.push_back(TPair<K, V>{Key, V()}); return Data.back().Value; }
	bool Contains(const K& Key) const { return Find(Key) != nullptr; }
	int32 Num() const { return static_cast<int32>(Data.size()); }
	typename std::vector<TPair<K, V>>::iterator begin() { return Data.begin(); }
	typename std::vector<TPair<K, V>>::iterator end() { return Data.end(); }

private:
	std::vector<TPair<K, V>> Data;
};

// ---------------------------------------------------------------------------------- strings
class FString
{
public:
	FString() = default;
	FString(const TCHAR* S) : Str(S ? S : L"") {}
	const TCHAR* operator*() const { return Str.c_str(); }
	bool IsEmpty() const { return Str.empty(); }
	int32 Len() const { return static_cast<int32>(Str.size()); }
	FString ToUpper() const { return *this; }
	int32 ParseIntoArray(TArray<FString>& Out, const TCHAR* Delimiter, bool bCullEmpty = true) const { (void)Delimiter; (void)bCullEmpty; Out.Reset(); return 0; }
	static FString FromInt(int32 Value) { return FString(std::to_wstring(Value).c_str()); }
	template <typename... Args>
	static FString Printf(const TCHAR* Format, Args&&... A)
	{
		static_assert(!(std::is_same<typename std::decay<Args>::type, FString>::value || ...), "Pass *FString (const TCHAR*) to Printf, not FString");
		(void)Format;
		(void)sizeof...(A);
		return FString();
	}
	friend FString operator+(const FString& A, const FString& B) { FString R; R.Str = A.Str + B.Str; return R; }
	friend FString operator+(const FString& A, const TCHAR* B) { FString R; R.Str = A.Str + B; return R; }
	bool operator==(const FString& Other) const { return Str == Other.Str; }

private:
	std::wstring Str;
};

class FName
{
public:
	FName() = default;
	FName(const TCHAR* S) : Str(S) {}
	bool IsNone() const { return Str.IsEmpty(); }
	bool operator==(const FName& Other) const { return Str == Other.Str; }
	FString ToString() const { return Str; }

private:
	FString Str;
};
static const FName NAME_None;

class FText
{
public:
	static FText FromString(const FString& S) { FText T; T.Str = S; return T; }
	static const FText& GetEmpty() { static FText Empty; return Empty; }
	static FText AsNumber(int32 Value) { return FromString(FString::FromInt(Value)); }
	static FText AsNumber(float Value) { (void)Value; return FText(); }
	template <typename... Args>
	static FText Format(const FText& Fmt, Args&&... A) { (void)sizeof...(A); return Fmt; }
	FString ToString() const { return Str; }
	bool IsEmpty() const { return Str.IsEmpty(); }

private:
	FString Str;
};

struct FAnsiToTchar
{
	explicit FAnsiToTchar(const char* S) { while (S && *S) { Buf.push_back(static_cast<wchar_t>(*S++)); } }
	const TCHAR* Get() const { return Buf.c_str(); }
	std::wstring Buf;
};
#define ANSI_TO_TCHAR(x) FAnsiToTchar(x).Get()

// ------------------------------------------------------------------------------------- math
namespace EAxis
{
enum Type
{
	None,
	X,
	Y,
	Z
};
}

struct FRotator;

struct FVector
{
	double X = 0.0, Y = 0.0, Z = 0.0;
	FVector() = default;
	FVector(double InX, double InY, double InZ) : X(InX), Y(InY), Z(InZ) {}
	explicit FVector(double V) : X(V), Y(V), Z(V) {}
	static const FVector ZeroVector;
	static const FVector UpVector;
	static const FVector ForwardVector;
	FVector operator+(const FVector& V) const { return FVector(X + V.X, Y + V.Y, Z + V.Z); }
	FVector operator-(const FVector& V) const { return FVector(X - V.X, Y - V.Y, Z - V.Z); }
	FVector operator-() const { return FVector(-X, -Y, -Z); }
	FVector operator*(double S) const { return FVector(X * S, Y * S, Z * S); }
	FVector operator/(double S) const { return FVector(X / S, Y / S, Z / S); }
	FVector& operator+=(const FVector& V) { X += V.X; Y += V.Y; Z += V.Z; return *this; }
	FVector& operator*=(double S) { X *= S; Y *= S; Z *= S; return *this; }
	bool operator==(const FVector& V) const { return X == V.X && Y == V.Y && Z == V.Z; }
	double Size() const { return std::sqrt(X * X + Y * Y + Z * Z); }
	double Size2D() const { return std::sqrt(X * X + Y * Y); }
	double SizeSquared() const { return X * X + Y * Y + Z * Z; }
	FVector GetSafeNormal() const { return *this; }
	FVector GetSafeNormal2D() const { return *this; }
	bool Normalize() { return true; }
	FRotator Rotation() const;
	FVector RotateAngleAxis(double AngleDeg, const FVector& Axis) const { (void)AngleDeg; (void)Axis; return *this; }
	static double DotProduct(const FVector& A, const FVector& B) { return A.X * B.X + A.Y * B.Y + A.Z * B.Z; }
	static FVector CrossProduct(const FVector& A, const FVector& B) { (void)B; return A; }
	static FVector VectorPlaneProject(const FVector& V, const FVector& N) { (void)N; return V; }
	static double Dist(const FVector& A, const FVector& B) { return (A - B).Size(); }
	static double Dist2D(const FVector& A, const FVector& B) { return (A - B).Size2D(); }
	static double DistSquared(const FVector& A, const FVector& B) { return (A - B).SizeSquared(); }
};
inline FVector operator*(double S, const FVector& V) { return V * S; }

struct FVector2D
{
	double X = 0.0, Y = 0.0;
	FVector2D() = default;
	FVector2D(double InX, double InY) : X(InX), Y(InY) {}
	static const FVector2D ZeroVector;
	FVector2D operator+(const FVector2D& V) const { return FVector2D(X + V.X, Y + V.Y); }
	FVector2D operator-(const FVector2D& V) const { return FVector2D(X - V.X, Y - V.Y); }
	FVector2D operator*(double S) const { return FVector2D(X * S, Y * S); }
	double SizeSquared() const { return X * X + Y * Y; }
	bool Equals(const FVector2D& V, double Tolerance) const { return std::fabs(X - V.X) <= Tolerance && std::fabs(Y - V.Y) <= Tolerance; }
	static double Distance(const FVector2D& A, const FVector2D& B) { return std::sqrt((A - B).SizeSquared()); }
};

struct FVector4
{
	double X = 0, Y = 0, Z = 0, W = 0;
	FVector4() = default;
	FVector4(double InX, double InY, double InZ, double InW) : X(InX), Y(InY), Z(InZ), W(InW) {}
};

struct FIntPoint
{
	int32 X = 0, Y = 0;
	FIntPoint() = default;
	FIntPoint(int32 InX, int32 InY) : X(InX), Y(InY) {}
	bool operator==(const FIntPoint& O) const { return X == O.X && Y == O.Y; }
};

struct FRotator
{
	double Pitch = 0.0, Yaw = 0.0, Roll = 0.0;
	FRotator() = default;
	FRotator(double InPitch, double InYaw, double InRoll) : Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}
	static const FRotator ZeroRotator;
};
inline FRotator FVector::Rotation() const { return FRotator(); }

struct FQuat
{
	static const FQuat Identity;
};

struct FTransform
{
	FTransform() = default;
	FTransform(const FRotator& R, const FVector& T, const FVector& S) { (void)R; (void)T; (void)S; }
	static const FTransform Identity;
	FVector GetUnitAxis(EAxis::Type Axis) const { (void)Axis; return FVector(); }
	FVector TransformPosition(const FVector& V) const { return V; }
};

struct FRotationMatrix
{
	explicit FRotationMatrix(const FRotator& R) { (void)R; }
	FVector GetUnitAxis(EAxis::Type Axis) const { (void)Axis; return FVector(); }
};

struct FColor
{
	uint8 R = 0, G = 0, B = 0, A = 255;
	FColor() = default;
	FColor(uint8 InR, uint8 InG, uint8 InB, uint8 InA = 255) : R(InR), G(InG), B(InB), A(InA) {}
};

struct FLinearColor
{
	float R = 0, G = 0, B = 0, A = 1;
	FLinearColor() = default;
	FLinearColor(float InR, float InG, float InB, float InA = 1.0f) : R(InR), G(InG), B(InB), A(InA) {}
	explicit FLinearColor(const FColor& C) { R = C.R / 255.0f; G = C.G / 255.0f; B = C.B / 255.0f; A = C.A / 255.0f; }
	static const FLinearColor White;
	static const FLinearColor Black;
	FLinearColor operator*(float S) const { return FLinearColor(R * S, G * S, B * S, A * S); }
	FColor ToFColor(bool bSRGB) const { (void)bSRGB; return FColor(); }
};

struct FBox2D
{
	FVector2D Min, Max;
	FBox2D() = default;
	FBox2D(const FVector2D& InMin, const FVector2D& InMax) : Min(InMin), Max(InMax) {}
	bool IsInside(const FVector2D& P) const { return P.X > Min.X && P.X < Max.X && P.Y > Min.Y && P.Y < Max.Y; }
};

struct FDateTime
{
	static FDateTime Now() { return FDateTime(); }
	FString ToString(const TCHAR* Format) const { (void)Format; return FString(); }
};

// Strict on purpose: every argument must already have the same type (no float/double mixing).
struct FMath
{
	template <class T> static T Clamp(const T X, const T Lo, const T Hi) { return X < Lo ? Lo : (X > Hi ? Hi : X); }
	template <class T> static T Max(const T A, const T B) { return A > B ? A : B; }
	template <class T> static T Min(const T A, const T B) { return A < B ? A : B; }
	template <class T> static T Abs(const T A) { return A < 0 ? -A : A; }
	template <class T> static T Square(const T A) { return A * A; }
	template <class T, class U> static T Lerp(const T& A, const T& B, const U& Alpha) { return static_cast<T>(A + (B - A) * Alpha); }
	template <class T> static T SmoothStep(T A, T B, T X) { (void)A; (void)B; return X; }
	static float FInterpTo(float Current, float Target, float DeltaTime, float Speed) { (void)Target; (void)DeltaTime; (void)Speed; return Current; }
	static FVector VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float Speed) { (void)Target; (void)DeltaTime; (void)Speed; return Current; }
	static FRotator RInterpTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float Speed) { (void)Target; (void)DeltaTime; (void)Speed; return Current; }
	static float Sin(float V) { return std::sin(V); }
	static double Sin(double V) { return std::sin(V); }
	static float Cos(float V) { return std::cos(V); }
	static double Cos(double V) { return std::cos(V); }
	static float Sqrt(float V) { return std::sqrt(V); }
	static double Sqrt(double V) { return std::sqrt(V); }
	static float Fmod(float A, float B) { return std::fmod(A, B); }
	static int32 RoundToInt(float V) { return static_cast<int32>(std::lround(V)); }
	static int32 RoundToInt(double V) { return static_cast<int32>(std::lround(V)); }
	static int32 FloorToInt(float V) { return static_cast<int32>(std::floor(V)); }
	template <class T> static T DegreesToRadians(T V) { return V * static_cast<T>(UE_PI / 180.0f); }
	template <class T> static T RadiansToDegrees(T V) { return V * static_cast<T>(180.0f / UE_PI); }
	static bool IsNearlyEqual(float A, float B, float Tolerance = UE_SMALL_NUMBER) { return std::fabs(A - B) <= Tolerance; }
};

// ------------------------------------------------------------------------------ UObject core
class UClass;
class UWorld;
class UScriptStruct;

class UObject
{
public:
	virtual ~UObject() = default;
	virtual UWorld* GetWorld() const { return nullptr; }
	template <class T> bool IsA() const { return dynamic_cast<const T*>(this) != nullptr; }
	FString GetName() const { return FString(); }
	FString GetPathName() const { return FString(); }
	template <class T> T* CreateDefaultSubobject(FName Name, bool bTransient = false) { (void)Name; (void)bTransient; return new T(); }
	static UClass* StaticClass();
};

class UClass : public UObject
{
};

class UInterface : public UObject
{
public:
	static UClass* StaticClass();
};

template <class T>
class TObjectPtr
{
public:
	TObjectPtr() = default;
	TObjectPtr(std::nullptr_t) {}
	template <class U, typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	TObjectPtr(U* In) : Ptr(In) {}
	template <class U, typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	TObjectPtr(const TObjectPtr<U>& In) : Ptr(In.Get()) {}
	T* Get() const { return Ptr; }
	operator T*() const { return Ptr; }
	T* operator->() const { return Ptr; }
	explicit operator bool() const { return Ptr != nullptr; }

private:
	T* Ptr = nullptr;
};

template <class T>
class TWeakObjectPtr
{
public:
	TWeakObjectPtr() = default;
	TWeakObjectPtr(std::nullptr_t) {}
	template <class U, typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	TWeakObjectPtr(U* In) : Ptr(In) {}
	T* Get() const { return Ptr; }
	bool IsValid() const { return Ptr != nullptr; }
	T* operator->() const { return Ptr; }
	bool operator==(const TWeakObjectPtr& O) const { return Ptr == O.Ptr; }

private:
	T* Ptr = nullptr;
};

template <class T>
class TSubclassOf
{
public:
	TSubclassOf() = default;
	TSubclassOf(UClass* In) : Class(In) {}
	UClass* Get() const { return Class; }
	explicit operator bool() const { return Class != nullptr; }

private:
	UClass* Class = nullptr;
};

template <class T>
struct TEnumAsByte
{
	TEnumAsByte() = default;
	TEnumAsByte(T In) : Value(In) {}
	T Value{};
};

template <class To, class From>
To* Cast(From* Src) { return dynamic_cast<To*>(Src); }
template <class To, class From>
const To* Cast(const From* Src) { return dynamic_cast<const To*>(Src); }
template <class To, class From>
To* Cast(const TObjectPtr<From>& Src) { return dynamic_cast<To*>(Src.Get()); }

template <class T>
T* NewObject(UObject* Outer = nullptr, FName Name = NAME_None) { (void)Outer; (void)Name; return new T(); }
template <class T>
T* LoadObject(UObject* Outer, const TCHAR* Path) { (void)Outer; (void)Path; return nullptr; }
template <class T>
T* DuplicateObject(const T* Source, UObject* Outer) { (void)Source; (void)Outer; return nullptr; }

// ----------------------------------------------------------------------------------- logging
template <typename... Args>
inline void HadiStubLog(const TCHAR* Format, Args&&... A)
{
	static_assert(!(std::is_same<typename std::decay<Args>::type, FString>::value || ...), "Pass *FString to UE_LOG, not FString");
	(void)Format;
	(void)sizeof...(A);
}
#define DECLARE_LOG_CATEGORY_EXTERN(Name, Verbosity, Compile) extern struct FLogCategory##Name { } Name;
#define DEFINE_LOG_CATEGORY(Name) FLogCategory##Name Name;
#define UE_LOG(Category, Verbosity, Format, ...) ((void)&Category, HadiStubLog(Format, ##__VA_ARGS__))
