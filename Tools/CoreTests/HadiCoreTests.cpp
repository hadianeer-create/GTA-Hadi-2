// HADI: VICE CITY - Simulation core unit tests.
// Build & run: Tools/CoreTests/run_tests.sh   (needs any C++17 compiler, no Unreal Engine)
#include "Core/HadiCity.h"
#include "Core/HadiMinimap.h"
#include "Core/HadiRig.h"
#include "Core/HadiTimeOfDay.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <set>
#include <string>

using namespace hadi;

static int gFailures = 0;
static int gChecks = 0;

#define EXPECT(cond, ...)                                                                                                                                       \
	do                                                                                                                                                         \
	{                                                                                                                                                          \
		++gChecks;                                                                                                                                             \
		if (!(cond))                                                                                                                                           \
		{                                                                                                                                                      \
			++gFailures;                                                                                                                                       \
			std::printf("  FAIL %s:%d: %s -- ", __FILE__, __LINE__, #cond);                                                                                   \
			std::printf(__VA_ARGS__);                                                                                                                          \
			std::printf("\n");                                                                                                                                 \
		}                                                                                                                                                      \
	} while (0)

static bool Finite(float v) { return std::isfinite(v); }
static bool Near(float a, float b, float tol) { return std::fabs(a - b) <= tol; }

static void TestMath()
{
	std::printf("[math]\n");
	// Unreal FRotator conventions.
	Basis b = BasisFromRot(Rot{0.0f, 90.0f, 0.0f});
	EXPECT(Near(b.x.y, 1.0f, 1e-5f), "yaw 90 should turn X towards +Y, got %f", b.x.y);
	b = BasisFromRot(Rot{30.0f, 0.0f, 0.0f});
	EXPECT(b.x.z > 0.49f, "positive pitch should raise the nose");
	b = BasisFromRot(Rot{0.0f, 0.0f, 20.0f});
	EXPECT(b.z.y > 0.3f, "positive roll should tilt Z towards +Y");
	EXPECT(Near(WrapDegrees(190.0f), -170.0f, 1e-4f), "wrap");
	Rng a(42), c(42);
	for (int i = 0; i < 100; ++i)
	{
		EXPECT(a.Next() == c.Next(), "rng determinism");
	}
	Rng r(7);
	for (int i = 0; i < 1000; ++i)
	{
		const float u = r.Unit();
		EXPECT(u >= 0.0f && u < 1.0f, "unit range %f", u);
		const int k = r.RangeInt(2, 5);
		EXPECT(k >= 2 && k <= 5, "range int %d", k);
	}
}

static unsigned long long HashModel(const CityModel& m)
{
	unsigned long long h = 1469598103934665603ull;
	auto mix = [&h](float v) {
		unsigned int bits;
		std::memcpy(&bits, &v, sizeof(bits));
		h = (h ^ bits) * 1099511628211ull;
	};
	for (const Prim& p : m.prims)
	{
		mix(p.center.x);
		mix(p.center.y);
		mix(p.center.z);
		mix(p.size.x);
	}
	return h;
}

static void TestCity()
{
	std::printf("[city]\n");
	const CityModel city = GenerateCity(CityParams{});
	std::printf("  prims=%zu map=%zu pois=%zu roads=%zu buildings=%zu graphNodes=%d\n", city.prims.size(), city.map.size(), city.pois.size(),
		city.roads.size(), city.footprints.size(), city.graph.NumNodes());

	EXPECT(city.prims.size() > 5000 && city.prims.size() < 80000, "prim budget: %zu", city.prims.size());
	const CityModel again = GenerateCity(CityParams{});
	EXPECT(HashModel(city) == HashModel(again), "generation must be deterministic");

	// Every primitive is finite and sensible.
	int bad = 0;
	for (const Prim& p : city.prims)
	{
		if (!Finite(p.center.x) || !Finite(p.center.y) || !Finite(p.center.z) || !(p.size.x > 0.0f) || !(p.size.y > 0.0f) || !(p.size.z > 0.0f) ||
			!Finite(p.rot.pitch) || !Finite(p.rot.yaw) || !Finite(p.rot.roll))
		{
			++bad;
		}
	}
	EXPECT(bad == 0, "%d invalid prims", bad);

	// Streaming cells: every local prim lands in a valid cell.
	int outside = 0;
	for (const Prim& p : city.prims)
	{
		if (p.cell < -1 || p.cell >= city.cellsX * city.cellsY)
		{
			++outside;
		}
	}
	EXPECT(outside == 0, "%d prims with invalid cell", outside);

	// Buildings never sit on a road.
	int onRoad = 0;
	for (const Footprint& f : city.footprints)
	{
		for (const RoadSeg& r : city.roads)
		{
			const bool vertical = std::fabs(r.a.x - r.b.x) < 1.0f; // runs along Y
			const float half = r.width * 0.5f;
			float rx0, rx1, ry0, ry1;
			if (vertical)
			{
				rx0 = r.a.x - half;
				rx1 = r.a.x + half;
				ry0 = std::min(r.a.y, r.b.y);
				ry1 = std::max(r.a.y, r.b.y);
			}
			else
			{
				ry0 = r.a.y - half;
				ry1 = r.a.y + half;
				rx0 = std::min(r.a.x, r.b.x);
				rx1 = std::max(r.a.x, r.b.x);
			}
			const bool overlap = f.x0 < rx1 && f.x1 > rx0 && f.y0 < ry1 && f.y1 > ry0;
			if (overlap)
			{
				++onRoad;
			}
		}
	}
	EXPECT(onRoad == 0, "%d building/road overlaps", onRoad);

	// Buildings don't overlap each other (allow touching).
	int overlaps = 0;
	for (size_t i = 0; i < city.footprints.size(); ++i)
	{
		for (size_t j = i + 1; j < city.footprints.size(); ++j)
		{
			const Footprint& a = city.footprints[i];
			const Footprint& c = city.footprints[j];
			const bool overlap = a.x0 < c.x1 - 1.0f && a.x1 > c.x0 + 1.0f && a.y0 < c.y1 - 1.0f && a.y1 > c.y0 + 1.0f;
			// A tower stacked on its own podium is fully contained - that's intended.
			const bool contained = (c.x0 >= a.x0 && c.x1 <= a.x1 && c.y0 >= a.y0 && c.y1 <= a.y1) || (a.x0 >= c.x0 && a.x1 <= c.x1 && a.y0 >= c.y0 && a.y1 <= c.y1);
			if (overlap && !contained)
			{
				++overlaps;
			}
		}
	}
	EXPECT(overlaps == 0, "%d building overlaps", overlaps);

	// Gameplay anchors.
	EXPECT(city.CountPois(PoiType::PlayerSpawn) == 1, "one spawn");
	EXPECT(city.FindPoi(PoiType::Restaurant, poi_id::CoralCafe) != nullptr, "Coral Cafe exists");
	EXPECT(city.FindPoi(PoiType::ClothingStore, poi_id::TideAndThread) != nullptr, "Tide & Thread exists");
	EXPECT(city.FindPoi(PoiType::Safehouse, poi_id::HadiApartment) != nullptr, "safehouse exists");
	EXPECT(city.FindPoi(PoiType::VehicleSpawn, poi_id::StarterCar) != nullptr, "starter car exists");
	EXPECT(city.CountPois(PoiType::Collectible) == 12, "12 Neon Flamingos, got %d", city.CountPois(PoiType::Collectible));
	EXPECT(city.CountPois(PoiType::StreetLight) > 100, "street lights %d", city.CountPois(PoiType::StreetLight));

	std::set<int> ids;
	for (const Poi& p : city.pois)
	{
		const bool ground = p.type != PoiType::StreetLight && p.type != PoiType::NeonLight;
		if (!ground)
		{
			continue;
		}
		EXPECT(!city.InsideBuilding(XY(p.pos), 30.0f), "POI '%s' (type %d) at (%.0f, %.0f) is inside a building", p.name, static_cast<int>(p.type), p.pos.x,
			p.pos.y);
		EXPECT(p.pos.z > city.seaLevel, "POI '%s' below sea level", p.name);
		if (p.type == PoiType::Collectible)
		{
			EXPECT(ids.insert(p.id).second, "duplicate collectible id %d", p.id);
		}
	}

	// The prologue route is walkable distance: spawn -> cafe -> car -> marina.
	const Poi* spawn = city.FindPoi(PoiType::PlayerSpawn);
	const Poi* cafe = city.FindPoi(PoiType::Restaurant, poi_id::CoralCafe);
	const Poi* car = city.FindPoi(PoiType::VehicleSpawn, poi_id::StarterCar);
	if (spawn && cafe && car)
	{
		const float toCafe = Distance(XY(spawn->pos), XY(cafe->pos));
		const float toCar = Distance(XY(cafe->pos), XY(car->pos));
		EXPECT(toCafe > 3000.0f && toCafe < 25000.0f, "spawn->cafe %.0f cm", toCafe);
		EXPECT(toCar < 3000.0f, "cafe->car %.0f cm", toCar);
		std::printf("  spawn->cafe %.1f m, cafe->car %.1f m\n", toCafe / 100.0f, toCar / 100.0f);
	}

	// Names.
	EXPECT(std::string(city.DistrictAt(Vec2{5000.0f, 18000.0f})) == "Coral Bay", "district: %s", city.DistrictAt(Vec2{5000.0f, 18000.0f}));
	EXPECT(std::string(city.StreetAt(Vec2{2000.0f, 23000.0f})) == "Seabreeze Boulevard", "street: %s", city.StreetAt(Vec2{2000.0f, 23000.0f}));
	EXPECT(std::string(city.DistrictAt(Vec2{-6000.0f, -6000.0f})) == "Flamingo Park", "park district");

	// Road graph & GPS.
	EXPECT(city.graph.IsConnected(), "road network must be connected");
	std::vector<Vec2> route;
	const Poi* marina = city.FindPoi(PoiType::MarinaMeet);
	EXPECT(marina != nullptr, "marina meet point");
	if (car && marina)
	{
		EXPECT(city.graph.BuildRoute(XY(car->pos), XY(marina->pos), route), "route car->marina");
		float len = 0.0f;
		for (size_t i = 1; i < route.size(); ++i)
		{
			len += Distance(route[i - 1], route[i]);
		}
		const float straight = Distance(XY(car->pos), XY(marina->pos));
		EXPECT(len >= straight - 1.0f, "route shorter than straight line");
		EXPECT(len < straight * 1.8f, "route too long: %.0f vs %.0f", len, straight);
		std::printf("  GPS car->marina: %zu points, %.0f m (straight %.0f m)\n", route.size(), len / 100.0f, straight / 100.0f);
	}
	std::vector<int> path;
	EXPECT(city.graph.FindPath(0, city.graph.NumNodes() - 1, path), "path to causeway end");
}

static void TestRig()
{
	std::printf("[rig]\n");
	EXPECT(GetRigSegments().size() > 40, "segments %zu", GetRigSegments().size());
	EXPECT(OutfitCount() >= 6, "outfits");
	for (int i = 0; i < OutfitCount(); ++i)
	{
		EXPECT(GetOutfit(i).name[0] != 0, "outfit name");
	}
	EXPECT(GetOutfit(0).price == 0, "starter outfit is free");
	// Shorts show skin on the shins.
	const Outfit& beach = GetOutfit(2);
	EXPECT(beach.shorts && SameColor(ResolveSlotColor(Slot::Shin, beach), ResolveSlotColor(Slot::Skin, beach)), "shorts");

	// Standing height.
	AnimState s;
	AnimInput in;
	for (int i = 0; i < 60; ++i)
	{
		UpdateAnimState(s, in, 1.0f / 60.0f);
	}
	Pose p = ComputePose(s);
	JointWorld w[kJointCount];
	SolveJoints(p, w);
	const float headZ = w[static_cast<int>(Joint::Head)].pos.z;
	EXPECT(headZ > 150.0f && headZ < 170.0f, "head joint height %f", headZ);
	EXPECT(Near(LowestFootZ(p), 0.0f, 0.6f), "idle feet on ground: %f", LowestFootZ(p));

	// Feet stay planted through walk / run / sprint cycles.
	const float speeds[3] = {170.0f, 420.0f, 680.0f};
	for (float speed : speeds)
	{
		AnimState g;
		AnimInput gi;
		gi.speed = speed;
		float maxErr = 0.0f;
		float maxLift = 0.0f;
		for (int i = 0; i < 240; ++i)
		{
			UpdateAnimState(g, gi, 1.0f / 60.0f);
			const Pose gp = ComputePose(g);
			maxErr = Max(maxErr, Abs(LowestFootZ(gp)));
			JointWorld gw[kJointCount];
			SolveJoints(gp, gw);
			maxLift = Max(maxLift, Max(gw[static_cast<int>(Joint::AnkleL)].pos.z, gw[static_cast<int>(Joint::AnkleR)].pos.z));
			for (int j = 0; j < kJointCount; ++j)
			{
				EXPECT(Finite(gw[j].pos.x) && Finite(gw[j].pos.z), "finite joint");
			}
		}
		EXPECT(maxErr < 0.6f, "speed %.0f: stance foot error %f", speed, maxErr);
		EXPECT(maxLift > 12.0f, "speed %.0f: swing foot should lift (%.1f)", speed, maxLift);
	}

	// Driving: seated pelvis height.
	AnimState d;
	AnimInput di;
	di.driving = true;
	for (int i = 0; i < 120; ++i)
	{
		UpdateAnimState(d, di, 1.0f / 60.0f);
	}
	SolveJoints(ComputePose(d), w);
	const float pelvisZ = w[static_cast<int>(Joint::Pelvis)].pos.z;
	EXPECT(pelvisZ > 40.0f && pelvisZ < 65.0f, "seated pelvis %f", pelvisZ);

	// Expressions & blinking.
	AnimState e;
	AnimInput ei;
	ei.expression = Expression::Happy;
	for (int i = 0; i < 120; ++i)
	{
		UpdateAnimState(e, ei, 1.0f / 60.0f);
	}
	const FaceState happy = ComputePose(e).face;
	EXPECT(happy.mouthCurve > 0.8f, "happy smile %f", happy.mouthCurve);
	bool blinked = false;
	for (int i = 0; i < 600; ++i)
	{
		UpdateAnimState(e, ei, 1.0f / 60.0f);
		if (ComputePose(e).face.eyeOpen < 0.3f)
		{
			blinked = true;
		}
	}
	EXPECT(blinked, "Hadi should blink within 10 seconds");
}

static void TestTimeOfDay()
{
	std::printf("[time]\n");
	const SkyState noon = ComputeSky(13.0f);
	const SkyState night = ComputeSky(1.0f);
	const SkyState dawn = ComputeSky(kSunriseHour + 0.1f);
	EXPECT(noon.sunElevation > 60.0f, "noon elevation %f", noon.sunElevation);
	EXPECT(noon.nightFactor < 0.01f, "noon is day");
	EXPECT(night.nightFactor > 0.99f, "1am is night");
	EXPECT(night.sunIntensityLux < 0.01f, "no sun at night");
	EXPECT(dawn.lightYaw < -80.0f, "sunrise light comes from the east (yaw %f)", dawn.lightYaw);
	float prev = ComputeSky(0.0f).nightFactor;
	for (float h = 0.01f; h < 24.0f; h += 0.01f)
	{
		const SkyState s = ComputeSky(h);
		EXPECT(Abs(s.nightFactor - prev) < 0.05f, "night factor jump at %f", h);
		EXPECT(s.lightPitch <= -1.0f + 1e-4f, "sun never shines upwards (%f at %f)", s.lightPitch, h);
		prev = s.nightFactor;
	}
	EXPECT(ClockHour(25.5f) == 1 && ClockMinute(25.5f) == 30, "clock wrap");
}

static void TestMinimap()
{
	std::printf("[minimap]\n");
	MinimapView v;
	v.center = Vec2{1000.0f, 1000.0f};
	v.pixelsPerCm = 0.1f;
	v.screenCenter = Vec2{100.0f, 100.0f};
	v.yawDeg = 0.0f;
	Vec2 north = WorldToMinimap(v, Vec2{1100.0f, 1000.0f});
	EXPECT(Near(north.x, 100.0f, 1e-3f) && Near(north.y, 90.0f, 1e-3f), "north is up (%f, %f)", north.x, north.y);
	Vec2 east = WorldToMinimap(v, Vec2{1000.0f, 1100.0f});
	EXPECT(Near(east.x, 110.0f, 1e-3f) && Near(east.y, 100.0f, 1e-3f), "east is right");
	v.yawDeg = 90.0f; // camera faces east -> east is up
	east = WorldToMinimap(v, Vec2{1000.0f, 1100.0f});
	EXPECT(Near(east.x, 100.0f, 1e-3f) && Near(east.y, 90.0f, 1e-3f), "rotating minimap (%f, %f)", east.x, east.y);
	EXPECT(Near(MinimapRotation(v, 90.0f), 0.0f, 1e-4f), "rect aligned with camera");
}

int main()
{
	TestMath();
	TestCity();
	TestRig();
	TestTimeOfDay();
	TestMinimap();
	std::printf("\n%d checks, %d failures\n", gChecks, gFailures);
	return gFailures == 0 ? 0 : 1;
}
