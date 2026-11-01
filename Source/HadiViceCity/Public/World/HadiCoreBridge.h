// HADI: VICE CITY
// Conversions between the engine-agnostic simulation core (namespace hadi) and Unreal types.
#pragma once

#include "CoreMinimal.h"
#include "Core/HadiMath.h"

namespace HadiBridge
{
inline FVector ToFVector(const hadi::Vec3& V) { return FVector(V.x, V.y, V.z); }
inline FVector2D ToFVector2D(const hadi::Vec2& V) { return FVector2D(V.x, V.y); }
inline FRotator ToFRotator(const hadi::Rot& R) { return FRotator(R.pitch, R.yaw, R.roll); }
inline hadi::Vec2 ToHadi2D(const FVector& V) { return hadi::Vec2{static_cast<float>(V.X), static_cast<float>(V.Y)}; }

inline uint8 ToByte(float Unit) { return static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(Unit * 255.0f), 0, 255)); }

// Core colours are authored in sRGB; FLinearColor(FColor) performs the sRGB -> linear conversion.
inline FLinearColor ToLinear(const hadi::Color& C) { return FLinearColor(FColor(ToByte(C.r), ToByte(C.g), ToByte(C.b), 255)); }
inline uint32 PackColor(const hadi::Color& C) { return (uint32(ToByte(C.r)) << 16) | (uint32(ToByte(C.g)) << 8) | uint32(ToByte(C.b)); }
} // namespace HadiBridge
