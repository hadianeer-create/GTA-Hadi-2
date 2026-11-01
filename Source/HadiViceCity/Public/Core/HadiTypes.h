// HADI: VICE CITY - Simulation Core
// Shared enums and the art-direction colour palette (engine agnostic).
#pragma once

#include "Core/HadiMath.h"

namespace hadi
{
// Primitive meshes. The engine maps these to /Engine/BasicShapes (100 cm, pivot at centre).
enum class Shape : uint8_t
{
	Cube,
	Cylinder,
	Sphere,
	Cone,
	Count
};

// Surface families. The engine maps each to a parent material + parameter set.
enum class Mat : uint8_t
{
	Matte,      // plaster, concrete, fabric
	Painted,    // painted walls, trims
	Glossy,     // car paint, polished stone
	Metal,      // poles, rails, rims
	Glass,      // dark reflective building glass (opaque)
	GlassClear, // see-through glass (vehicles, shop fronts)
	WindowLit,  // building window that glows at night
	Neon,       // neon tubes and signs (always faintly lit, bright at night)
	Light,      // lamp heads, headlights (lit at night)
	Water,      // ocean, pools, fountains
	Foliage,    // grass, hedges, palm fronds
	Sand,
	Asphalt,
	Count
};

// Distance-based culling buckets for instanced geometry.
enum class CullClass : uint8_t
{
	Never,  // terrain, ocean, skyline
	Far,    // building bodies (~2 km)
	Medium, // facade details, palms (~700 m)
	Near    // road paint, small props (~300 m)
};

inline float CullDistanceCm(CullClass c)
{
	switch (c)
	{
	case CullClass::Far: return 250000.0f;
	case CullClass::Medium: return 80000.0f;
	case CullClass::Near: return 32000.0f;
	default: return 0.0f;
	}
}

// Art-direction palette ("Vice pastel"). sRGB values.
namespace pal
{
constexpr Color Asphalt = Rgb(38, 39, 42);
constexpr Color AsphaltWorn = Rgb(52, 53, 56);
constexpr Color Sidewalk = Rgb(178, 172, 160);
constexpr Color Curb = Rgb(196, 192, 184);
constexpr Color Concrete = Rgb(150, 148, 142);
constexpr Color Seawall = Rgb(120, 118, 112);
constexpr Color Sand = Rgb(236, 214, 170);
constexpr Color SandWet = Rgb(196, 172, 128);
constexpr Color SeaFloor = Rgb(170, 150, 110);
constexpr Color Grass = Rgb(74, 128, 52);
constexpr Color GrassDark = Rgb(56, 104, 40);
constexpr Color Hedge = Rgb(40, 86, 34);
constexpr Color PalmTrunk = Rgb(128, 104, 78);
constexpr Color PalmTrunkDark = Rgb(104, 84, 62);
constexpr Color PalmLeaf = Rgb(58, 120, 40);
constexpr Color PalmLeafDark = Rgb(40, 92, 30);
constexpr Color Coconut = Rgb(92, 64, 34);
constexpr Color Ocean = Rgb(18, 92, 118);
constexpr Color Pool = Rgb(40, 168, 210);
constexpr Color Wood = Rgb(132, 92, 58);
constexpr Color WoodDark = Rgb(92, 62, 40);
constexpr Color White = Rgb(240, 238, 232);
constexpr Color OffWhite = Rgb(226, 220, 206);
constexpr Color LaneWhite = Rgb(222, 222, 214);
constexpr Color LaneYellow = Rgb(226, 170, 40);
constexpr Color GlassDark = Rgb(26, 40, 52);
constexpr Color GlassBlue = Rgb(52, 88, 118);
constexpr Color GlassTeal = Rgb(46, 104, 108);
constexpr Color GlassClear = Rgb(150, 190, 205);
constexpr Color Metal = Rgb(168, 172, 178);
constexpr Color DarkMetal = Rgb(44, 46, 50);
constexpr Color Black = Rgb(16, 16, 18);
constexpr Color Charcoal = Rgb(54, 54, 60);
constexpr Color Rubber = Rgb(22, 22, 24);
constexpr Color Terracotta = Rgb(190, 104, 74);

// Pastel facades.
constexpr Color Pink = Rgb(244, 168, 186);
constexpr Color Mint = Rgb(160, 226, 196);
constexpr Color Cream = Rgb(246, 230, 196);
constexpr Color Sky = Rgb(162, 206, 240);
constexpr Color Lavender = Rgb(194, 176, 234);
constexpr Color Peach = Rgb(250, 190, 150);
constexpr Color Lemon = Rgb(246, 232, 150);
constexpr Color Coral = Rgb(244, 132, 116);
constexpr Color Aqua = Rgb(120, 214, 214);
constexpr Color Sand2 = Rgb(222, 200, 168);

// Accent trims.
constexpr Color TrimTeal = Rgb(40, 150, 150);
constexpr Color TrimPink = Rgb(214, 86, 130);
constexpr Color TrimNavy = Rgb(44, 62, 112);
constexpr Color TrimGold = Rgb(214, 170, 80);

// Neon.
constexpr Color NeonPink = Rgb(255, 50, 170);
constexpr Color NeonCyan = Rgb(40, 230, 255);
constexpr Color NeonPurple = Rgb(160, 70, 255);
constexpr Color NeonOrange = Rgb(255, 130, 30);
constexpr Color NeonGreen = Rgb(90, 255, 120);
constexpr Color NeonRed = Rgb(255, 40, 40);
constexpr Color NeonWhite = Rgb(235, 245, 255);

// Light emitters.
constexpr Color LampWarm = Rgb(255, 214, 160);
constexpr Color WindowWarm = Rgb(255, 204, 140);
constexpr Color WindowCool = Rgb(190, 214, 255);
constexpr Color WindowPurple = Rgb(200, 150, 255);
constexpr Color BeaconRed = Rgb(255, 40, 30);
} // namespace pal

// Minimap palette.
namespace mappal
{
constexpr Color Water = Rgb(32, 72, 96);
constexpr Color Road = Rgb(58, 60, 66);
constexpr Color Block = Rgb(120, 122, 128);
constexpr Color Building = Rgb(176, 178, 186);
constexpr Color BuildingTall = Rgb(206, 208, 216);
constexpr Color Park = Rgb(70, 120, 66);
constexpr Color Beach = Rgb(200, 182, 136);
constexpr Color Pier = Rgb(130, 96, 64);
constexpr Color Pool = Rgb(70, 170, 210);
constexpr Color Promenade = Rgb(150, 146, 136);
} // namespace mappal
} // namespace hadi
