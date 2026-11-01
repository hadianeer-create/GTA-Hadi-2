// HADI: VICE CITY - Simulation Core
// Engine-agnostic math primitives. This header must NOT include any Unreal headers so the
// core can be compiled and unit-tested outside the engine (see Tools/CoreTests).
//
// Conventions match Unreal Engine exactly:
//   * Units are centimetres.
//   * X = forward (north on the city map), Y = right (east), Z = up. Left-handed.
//   * Rotations are degrees in FRotator order (Pitch, Yaw, Roll) and BasisFromRot reproduces
//     FRotationMatrix, so a Rot built here can be passed straight into an FRotator.
#pragma once

#include <cmath>
#include <cstdint>

namespace hadi
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kDegToRad = kPi / 180.0f;
constexpr float kRadToDeg = 180.0f / kPi;

struct Vec2
{
	float x = 0.0f;
	float y = 0.0f;
};

inline Vec2 operator+(Vec2 a, Vec2 b) { return Vec2{a.x + b.x, a.y + b.y}; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return Vec2{a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(Vec2 a, float s) { return Vec2{a.x * s, a.y * s}; }
inline float Dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
inline float Length(Vec2 a) { return std::sqrt(Dot(a, a)); }
inline float Distance(Vec2 a, Vec2 b) { return Length(a - b); }

struct Vec3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

inline Vec3 operator+(Vec3 a, Vec3 b) { return Vec3{a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return Vec3{a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3 operator*(Vec3 a, float s) { return Vec3{a.x * s, a.y * s, a.z * s}; }
inline float Dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float Length(Vec3 a) { return std::sqrt(Dot(a, a)); }
inline Vec2 XY(Vec3 a) { return Vec2{a.x, a.y}; }

// Rotation in degrees, identical semantics to Unreal's FRotator.
struct Rot
{
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;
};

// World-space directions of a rotated object's local X, Y and Z axes.
struct Basis
{
	Vec3 x{1.0f, 0.0f, 0.0f};
	Vec3 y{0.0f, 1.0f, 0.0f};
	Vec3 z{0.0f, 0.0f, 1.0f};
};

// Same formula as Unreal's FRotationMatrix (rows are the rotated axes).
inline Basis BasisFromRot(const Rot& r)
{
	const float sp = std::sin(r.pitch * kDegToRad);
	const float cp = std::cos(r.pitch * kDegToRad);
	const float sy = std::sin(r.yaw * kDegToRad);
	const float cy = std::cos(r.yaw * kDegToRad);
	const float sr = std::sin(r.roll * kDegToRad);
	const float cr = std::cos(r.roll * kDegToRad);

	Basis b;
	b.x = Vec3{cp * cy, cp * sy, sp};
	b.y = Vec3{sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp};
	b.z = Vec3{-(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp};
	return b;
}

inline Vec3 Apply(const Basis& b, Vec3 v) { return b.x * v.x + b.y * v.y + b.z * v.z; }

// Express a child's local basis inside its parent's world basis (parent * child).
inline Basis Compose(const Basis& parent, const Basis& child)
{
	Basis out;
	out.x = Apply(parent, child.x);
	out.y = Apply(parent, child.y);
	out.z = Apply(parent, child.z);
	return out;
}

inline float Clamp(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline float Saturate(float v) { return Clamp(v, 0.0f, 1.0f); }
inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
inline float Max(float a, float b) { return a > b ? a : b; }
inline float Min(float a, float b) { return a < b ? a : b; }
inline float Abs(float a) { return a < 0.0f ? -a : a; }

inline float SmoothStep(float edge0, float edge1, float x)
{
	const float t = Saturate((x - edge0) / (edge1 - edge0));
	return t * t * (3.0f - 2.0f * t);
}

// Wraps an angle in degrees into [-180, 180).
inline float WrapDegrees(float a)
{
	float w = std::fmod(a + 180.0f, 360.0f);
	if (w < 0.0f)
	{
		w += 360.0f;
	}
	return w - 180.0f;
}

// Frame-rate independent exponential approach.
inline float ExpApproach(float current, float target, float rate, float dt)
{
	return target + (current - target) * std::exp(-rate * dt);
}

// sRGB colour (0..1). The engine layer converts to linear space.
struct Color
{
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
};

constexpr Color Rgb(int r, int g, int b) { return Color{r / 255.0f, g / 255.0f, b / 255.0f}; }

inline bool SameColor(const Color& a, const Color& b) { return a.r == b.r && a.g == b.g && a.b == b.b; }

// Deterministic xorshift64* generator. We deliberately avoid <random> distributions because
// their output differs between standard libraries (MSVC vs libstdc++), which would make the
// city in the engine differ from the city the unit tests validate.
class Rng
{
public:
	explicit Rng(uint64_t seedValue) : state(seedValue != 0 ? seedValue : 0x9E3779B97F4A7C15ull) { Next(); }

	uint32_t Next()
	{
		state ^= state >> 12;
		state ^= state << 25;
		state ^= state >> 27;
		return static_cast<uint32_t>((state * 0x2545F4914F6CDD1Dull) >> 32);
	}

	float Unit() { return static_cast<float>(Next() >> 8) * (1.0f / 16777216.0f); }
	float Range(float lo, float hi) { return lo + (hi - lo) * Unit(); }
	int RangeInt(int lo, int hiInclusive) { return lo + static_cast<int>(Next() % static_cast<uint32_t>(hiInclusive - lo + 1)); }
	bool Chance(float probability) { return Unit() < probability; }

private:
	uint64_t state;
};
} // namespace hadi
