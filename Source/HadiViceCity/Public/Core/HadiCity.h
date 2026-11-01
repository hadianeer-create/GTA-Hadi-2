// HADI: VICE CITY - Simulation Core
// Procedural layout of the first playable slice of Vice City: the island of "Coral Key".
//
// The generator is deterministic and engine agnostic. It outputs:
//   * Prims    - every visual/collision primitive (spawned as instanced meshes by the engine)
//   * MapRects - simplified 2D shapes for the minimap / pause map
//   * Pois     - gameplay anchors (spawn, shops, safehouse, collectibles, lights, vehicles)
//   * Roads    - centre lines for GPS + a RoadGraph for pathfinding
//   * Districts / footprints for location names and placement validation
#pragma once

#include "Core/HadiMath.h"
#include "Core/HadiRoadGraph.h"
#include "Core/HadiTypes.h"

#include <cstdint>
#include <vector>

namespace hadi
{
struct Prim
{
	Shape shape = Shape::Cube;
	Mat mat = Mat::Matte;
	Color color;
	Vec3 center;          // world centre (cm)
	Vec3 size{100, 100, 100}; // full extents along local axes (cm)
	Rot rot;
	bool collision = true;
	bool castShadow = true;
	CullClass cull = CullClass::Far;
	int cell = -1; // streaming cell index, -1 = always loaded (global)
};

enum class MapLayer : uint8_t
{
	Land,
	Beach,
	Promenade,
	Road,
	Block,
	Park,
	Pier,
	Pool,
	Building,
	Count
};

struct MapRect
{
	Vec2 center;
	Vec2 half; // half extents along local X / Y
	float yaw = 0.0f;
	Color color;
	MapLayer layer = MapLayer::Land;
};

enum class PoiType : uint8_t
{
	PlayerSpawn,
	Safehouse,
	Restaurant,
	ClothingStore,
	MarinaMeet,
	Collectible,
	VehicleSpawn,
	StreetLight,
	NeonLight,
	Count
};

struct Poi
{
	PoiType type = PoiType::PlayerSpawn;
	Vec3 pos;          // ground position (z = walkable surface height)
	float yaw = 0.0f;  // facing in degrees
	int id = 0;
	const char* name = "";
	Color color;
};

struct RoadSeg
{
	Vec2 a;
	Vec2 b;
	float width = 0.0f;
	const char* name = "";
};

struct District
{
	const char* name = "";
	Vec2 min;
	Vec2 max;
};

struct Footprint
{
	float x0 = 0, y0 = 0, x1 = 0, y1 = 0; // axis aligned, cm
	float height = 0;
};

struct CityModel
{
	std::vector<Prim> prims;
	std::vector<MapRect> map;
	std::vector<Poi> pois;
	std::vector<RoadSeg> roads;
	std::vector<District> districts;
	std::vector<Footprint> footprints;
	RoadGraph graph;

	float seaLevel = -60.0f;  // ocean surface Z (cm)
	Vec2 islandMin;           // walkable island bounds
	Vec2 islandMax;
	Vec2 worldMin;            // everything generated (incl. causeway + skyline)
	Vec2 worldMax;

	// Streaming grid.
	float cellSize = 20000.0f;
	Vec2 cellOrigin;
	int cellsX = 0;
	int cellsY = 0;

	int CellIndexAt(Vec2 p) const;
	Vec2 CellMin(int cell) const;
	Vec2 CellMax(int cell) const;

	const Poi* FindPoi(PoiType type, int id = -1) const;
	int CountPois(PoiType type) const;
	const char* DistrictAt(Vec2 p) const;
	// Nearest named road within maxDistance (cm), or "" when off-road.
	const char* StreetAt(Vec2 p, float maxDistance = 1500.0f) const;
	// True when p lies inside any building footprint (optionally grown by margin).
	bool InsideBuilding(Vec2 p, float margin = 0.0f) const;
};

struct CityParams
{
	uint64_t seed = 0xC0FFEE1986ull;
	bool includeSkyline = true;
};

CityModel GenerateCity(const CityParams& params);

// Ids used by gameplay code for specific landmarks.
namespace poi_id
{
constexpr int CoralCafe = 1;
constexpr int TideAndThread = 2;
constexpr int HadiApartment = 3;
constexpr int StarterCar = 0; // VehicleSpawn id 0 = the prologue car
} // namespace poi_id
} // namespace hadi
