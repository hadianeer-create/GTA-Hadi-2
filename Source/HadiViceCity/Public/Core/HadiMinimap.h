// HADI: VICE CITY - Simulation Core
// World -> minimap projection (rotating, player-centred, "camera up" minimap).
#pragma once

#include "Core/HadiMath.h"

namespace hadi
{
struct MinimapView
{
	Vec2 center;            // world position at the centre of the minimap (cm)
	float yawDeg = 0.0f;    // camera yaw; the camera's forward direction points UP on screen
	float pixelsPerCm = 0.01f;
	Vec2 screenCenter;      // pixel position of the centre inside the minimap texture
};

// Screen space: +x right, +y down (Unreal canvas convention).
inline Vec2 WorldToMinimap(const MinimapView& v, Vec2 world)
{
	const float dx = world.x - v.center.x;
	const float dy = world.y - v.center.y;
	const float c = std::cos(v.yawDeg * kDegToRad);
	const float s = std::sin(v.yawDeg * kDegToRad);
	const float forward = dx * c + dy * s;
	const float right = -dx * s + dy * c;
	return Vec2{v.screenCenter.x + right * v.pixelsPerCm, v.screenCenter.y - forward * v.pixelsPerCm};
}

// Rotation (degrees, clockwise on screen) for a shape whose local X axis has world yaw `worldYaw`.
// Pair with a tile whose width = 2 * halfY * ppc and height = 2 * halfX * ppc.
inline float MinimapRotation(const MinimapView& v, float worldYaw)
{
	return WrapDegrees(worldYaw - v.yawDeg);
}

// Clamp a minimap point to a rectangle inset from the edges (for off-screen blips).
inline Vec2 ClampToRect(Vec2 p, Vec2 minCorner, Vec2 maxCorner, bool& outClamped)
{
	const Vec2 c = Vec2{Clamp(p.x, minCorner.x, maxCorner.x), Clamp(p.y, minCorner.y, maxCorner.y)};
	outClamped = c.x != p.x || c.y != p.y;
	return c;
}
} // namespace hadi
