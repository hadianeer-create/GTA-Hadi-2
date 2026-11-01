// HADI: VICE CITY - Simulation Core
// Sun / moon / lighting curves for the 24h cycle. Pure functions so they are unit-tested.
#pragma once

#include "Core/HadiMath.h"

namespace hadi
{
struct SkyState
{
	float sunElevation = 0.0f; // degrees above horizon (negative = night)
	float lightYaw = 0.0f;     // yaw of the directional light (UE rotator)
	float lightPitch = 0.0f;   // pitch of the directional light (negative = shining down)
	float sunIntensityLux = 0.0f;
	float sunTemperatureK = 6500.0f;
	float moonPitch = -40.0f;
	float moonYaw = 0.0f;
	float moonIntensityLux = 0.0f;
	float nightFactor = 0.0f;  // 0 day .. 1 full night (drives emissives and street lights)
	float skyLightIntensity = 1.0f;
};

constexpr float kSunriseHour = 6.25f;
constexpr float kSunsetHour = 19.75f;
constexpr float kMaxSunElevation = 68.0f;

inline float WrapHours(float h)
{
	float w = std::fmod(h, 24.0f);
	if (w < 0.0f)
	{
		w += 24.0f;
	}
	return w;
}

inline SkyState ComputeSky(float hours)
{
	SkyState s;
	const float h = WrapHours(hours);
	const float dayLen = kSunsetHour - kSunriseHour;
	float elevation = 0.0f;
	float dayT = 0.0f;
	if (h >= kSunriseHour && h <= kSunsetHour)
	{
		dayT = (h - kSunriseHour) / dayLen;
		elevation = kMaxSunElevation * std::sin(kPi * dayT);
	}
	else
	{
		const float nightLen = 24.0f - dayLen;
		const float since = h > kSunsetHour ? h - kSunsetHour : h + 24.0f - kSunsetHour;
		const float nightT = since / nightLen;
		elevation = -40.0f * std::sin(kPi * nightT);
		dayT = 1.0f + nightT; // keep the light rotating through the night
	}
	s.sunElevation = elevation;
	// East (+Y) at sunrise, south at noon, west (-Y) at sunset. Light points away from the sun.
	s.lightYaw = -90.0f + 180.0f * dayT;
	// Below the horizon the sun is parked at -1 degree (intensity is zero anyway) so shadows never
	// get cast upwards through the ground.
	s.lightPitch = -Max(elevation, 1.0f);
	s.sunIntensityLux = 10.0f * SmoothStep(-2.0f, 8.0f, elevation);
	s.sunTemperatureK = Lerp(3300.0f, 6300.0f, SmoothStep(0.0f, 30.0f, elevation));
	s.nightFactor = 1.0f - SmoothStep(-6.0f, 6.0f, elevation);
	const float moonElevation = Max(25.0f, -elevation + 15.0f);
	s.moonPitch = -moonElevation;
	s.moonYaw = s.lightYaw + 180.0f;
	s.moonIntensityLux = 0.25f * s.nightFactor;
	s.skyLightIntensity = Lerp(1.0f, 1.6f, s.nightFactor);
	return s;
}

// "08:45" style clock text helpers.
inline int ClockHour(float hours) { return static_cast<int>(WrapHours(hours)); }
inline int ClockMinute(float hours)
{
	const float h = WrapHours(hours);
	return static_cast<int>((h - std::floor(h)) * 60.0f);
}
} // namespace hadi
