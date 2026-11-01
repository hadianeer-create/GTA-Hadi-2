// HADI: VICE CITY - Simulation Core
// Procedural generator for "Coral Key", the first playable district of Vice City.
//
// Layout (metres, X = north, Y = east):
//   Lantana Bay (west)  |  Marina Row  |  Bayview Dr | Bayshore Estates | Hibiscus Ave | Palm Market |
//   Flamingo Ave | Neon Mile / Coral Key Center | Coral Ave | Coral Bay deco hotels | Seabreeze Blvd |
//   promenade | Coral Bay Beach | Atlantic Ocean (east)
// South Point towers sit at the southern tip, North Shore Park at the northern tip and the
// Vice Causeway leaves the island westwards towards the downtown skyline.
#include "Core/HadiCity.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace hadi
{
namespace citygen
{
constexpr float kM = 100.0f; // metres -> cm

// ---- Layout constants (metres unless suffixed Cm) ----
constexpr float kIslandX0 = -640.0f;
constexpr float kIslandX1 = 540.0f;
constexpr float kIslandY0 = -275.0f;
constexpr float kIslandY1 = 250.0f;
constexpr float kAvenueHalf = 8.0f;
constexpr float kStreetHalf = 6.0f;
constexpr float kAvenueX0 = -616.0f;
constexpr float kAvenueX1 = 486.0f;
constexpr float kStreetY0 = -258.0f;
constexpr float kStreetY1 = 238.0f;

constexpr float kRoadTopCm = 0.0f;
constexpr float kWalkTopCm = 15.0f;
constexpr float kGrassTopCm = 17.0f;
constexpr float kPathTopCm = 18.0f;
constexpr float kSandTopCm = -10.0f;
constexpr float kPierTopCm = 5.0f;
constexpr float kDeepCm = -1500.0f;

constexpr int kAvenueCount = 5;
constexpr float kAvenueY[kAvenueCount] = {-250.0f, -125.0f, 0.0f, 125.0f, 230.0f};
const char* const kAvenueNames[kAvenueCount] = {"Bayview Drive", "Hibiscus Avenue", "Flamingo Avenue", "Coral Avenue", "Seabreeze Boulevard"};

constexpr int kStreetCount = 10;
constexpr float kStreetX[kStreetCount] = {-610.0f, -480.0f, -360.0f, -240.0f, -120.0f, 0.0f, 120.0f, 240.0f, 360.0f, 480.0f};
const char* const kStreetNames[kStreetCount] = {"South Point Drive", "Sunrise Street", "Conch Street", "Mango Street", "Tarpon Street",
	"Pelican Street", "Marlin Street", "Heron Street", "Seagrape Street", "Starfish Street"};

constexpr float kCausewayHalf = 7.0f;
constexpr float kCausewayEndY = -1300.0f;

struct Rect2
{
	float x0 = 0.0f;
	float y0 = 0.0f;
	float x1 = 0.0f;
	float y1 = 0.0f;

	float Cx() const { return 0.5f * (x0 + x1); }
	float Cy() const { return 0.5f * (y0 + y1); }
	float Sx() const { return x1 - x0; }
	float Sy() const { return y1 - y0; }
	Rect2 Inset(float d) const { return Rect2{x0 + d, y0 + d, x1 - d, y1 - d}; }
	bool Contains(Vec2 p) const { return p.x >= x0 && p.x <= x1 && p.y >= y0 && p.y <= y1; }
};

inline Rect2 RectM(float x0, float y0, float x1, float y1) { return Rect2{x0 * kM, y0 * kM, x1 * kM, y1 * kM}; }
inline Vec3 V3(Vec2 p, float z) { return Vec3{p.x, p.y, z}; }

enum class Face : uint8_t
{
	PosX,
	NegX,
	PosY,
	NegY
};

struct Facade
{
	Vec2 mid;
	Vec2 along{1.0f, 0.0f};
	Vec2 out{0.0f, 1.0f};
	float width = 0.0f;
	bool alongX = true;
};

Facade MakeFacade(const Rect2& r, Face f)
{
	Facade fa;
	switch (f)
	{
	case Face::PosY:
		fa.mid = Vec2{r.Cx(), r.y1};
		fa.along = Vec2{1.0f, 0.0f};
		fa.out = Vec2{0.0f, 1.0f};
		fa.width = r.Sx();
		fa.alongX = true;
		break;
	case Face::NegY:
		fa.mid = Vec2{r.Cx(), r.y0};
		fa.along = Vec2{-1.0f, 0.0f};
		fa.out = Vec2{0.0f, -1.0f};
		fa.width = r.Sx();
		fa.alongX = true;
		break;
	case Face::PosX:
		fa.mid = Vec2{r.x1, r.Cy()};
		fa.along = Vec2{0.0f, -1.0f};
		fa.out = Vec2{1.0f, 0.0f};
		fa.width = r.Sy();
		fa.alongX = false;
		break;
	case Face::NegX:
		fa.mid = Vec2{r.x0, r.Cy()};
		fa.along = Vec2{0.0f, 1.0f};
		fa.out = Vec2{-1.0f, 0.0f};
		fa.width = r.Sy();
		fa.alongX = false;
		break;
	}
	return fa;
}

// Rotation that tilts a flat slab so it slopes DOWN towards the facade's outward direction.
Rot SlopeOutward(Face f, float deg)
{
	switch (f)
	{
	case Face::PosY: return Rot{0.0f, 0.0f, deg};
	case Face::NegY: return Rot{0.0f, 0.0f, -deg};
	case Face::PosX: return Rot{-deg, 0.0f, 0.0f};
	case Face::NegX: return Rot{deg, 0.0f, 0.0f};
	}
	return Rot{};
}

// Shrinks a lot into a building body with separate front / side / back setbacks (cm).
Rect2 InsetFacing(const Rect2& lot, Face front, float frontInset, float sideInset, float backInset)
{
	switch (front)
	{
	case Face::PosY: return Rect2{lot.x0 + sideInset, lot.y0 + backInset, lot.x1 - sideInset, lot.y1 - frontInset};
	case Face::NegY: return Rect2{lot.x0 + sideInset, lot.y0 + frontInset, lot.x1 - sideInset, lot.y1 - backInset};
	case Face::PosX: return Rect2{lot.x0 + backInset, lot.y0 + sideInset, lot.x1 - frontInset, lot.y1 - sideInset};
	case Face::NegX: return Rect2{lot.x0 + frontInset, lot.y0 + sideInset, lot.x1 - backInset, lot.y1 - sideInset};
	}
	return lot;
}

struct WindowSpec
{
	float margin = 150.0f;
	float width = 160.0f;
	float height = 170.0f;
	float spacing = 110.0f;
	float sill = 90.0f;
	float litChance = 0.4f;
	Color glass = pal::GlassDark;
	bool eyebrows = false;
	Color eyebrowColor = pal::White;
};

class Builder
{
public:
	Builder(CityModel& inModel, uint64_t seedValue) : model(inModel), rng(seedValue) {}

	void Run(const CityParams& params);

private:
	CityModel& model;
	Rng rng;
	int collectibleCount = 0;

	// ---------------------------------------------------------------- primitives
	void Add(Shape shape, Vec3 c, Vec3 s, Rot r, Mat m, Color col, bool collide, CullClass cull, bool shadow)
	{
		Prim p;
		p.shape = shape;
		p.center = c;
		p.size = s;
		p.rot = r;
		p.mat = m;
		p.color = col;
		p.collision = collide;
		p.cull = cull;
		p.castShadow = shadow;
		model.prims.push_back(p);
	}

	void Box(Vec3 c, Vec3 s, Mat m, Color col, bool collide = true, CullClass cull = CullClass::Far, Rot r = Rot{}, bool shadow = true)
	{
		Add(Shape::Cube, c, s, r, m, col, collide, cull, shadow);
	}

	void Cyl(Vec3 c, float diameter, float height, Mat m, Color col, bool collide = true, CullClass cull = CullClass::Medium, Rot r = Rot{}, bool shadow = true)
	{
		Add(Shape::Cylinder, c, Vec3{diameter, diameter, height}, r, m, col, collide, cull, shadow);
	}

	void Ball(Vec3 c, Vec3 s, Mat m, Color col, CullClass cull = CullClass::Medium)
	{
		Add(Shape::Sphere, c, s, Rot{}, m, col, false, cull, true);
	}

	void ConeAt(Vec3 c, float diameter, float height, Mat m, Color col, CullClass cull = CullClass::Medium, Rot r = Rot{})
	{
		Add(Shape::Cone, c, Vec3{diameter, diameter, height}, r, m, col, false, cull, true);
	}

	void Slab(const Rect2& r, float zBottom, float zTop, Mat m, Color col, bool collide = true, CullClass cull = CullClass::Far, bool shadow = true)
	{
		Box(Vec3{r.Cx(), r.Cy(), 0.5f * (zBottom + zTop)}, Vec3{r.Sx(), r.Sy(), zTop - zBottom}, m, col, collide, cull, Rot{}, shadow);
	}

	// Box placed in a yawed local frame (props, boats, towers).
	void LocalBox(Vec3 origin, float yaw, Vec3 offset, Vec3 size, Mat m, Color col, bool collide = true, CullClass cull = CullClass::Medium, float pitch = 0.0f, float roll = 0.0f, bool shadow = true)
	{
		const Basis b = BasisFromRot(Rot{0.0f, yaw, 0.0f});
		Box(origin + Apply(b, offset), size, m, col, collide, cull, Rot{pitch, yaw, roll}, shadow);
	}

	void LocalCyl(Vec3 origin, float yaw, Vec3 offset, float diameter, float height, Mat m, Color col, bool collide = true, CullClass cull = CullClass::Medium)
	{
		const Basis b = BasisFromRot(Rot{0.0f, yaw, 0.0f});
		Cyl(origin + Apply(b, offset), diameter, height, m, col, collide, cull);
	}

	void MapR(const Rect2& r, Color col, MapLayer layer)
	{
		MapRect mr;
		mr.center = Vec2{r.Cx(), r.Cy()};
		mr.half = Vec2{r.Sx() * 0.5f, r.Sy() * 0.5f};
		mr.color = col;
		mr.layer = layer;
		model.map.push_back(mr);
	}

	void AddPoi(PoiType type, Vec3 pos, float yaw, int id, const char* name, Color col = Color{})
	{
		Poi p;
		p.type = type;
		p.pos = pos;
		p.yaw = yaw;
		p.id = id;
		p.name = name;
		p.color = col;
		model.pois.push_back(p);
	}

	void FacadeBox(const Facade& fa, float alongOff, float outOff, float zCenter, float len, float thick, float height, Mat m, Color col,
		bool collide, CullClass cull, bool shadow = true, Rot r = Rot{})
	{
		const Vec2 p = fa.mid + fa.along * alongOff + fa.out * outOff;
		const Vec3 size = fa.alongX ? Vec3{len, thick, height} : Vec3{thick, len, height};
		Box(V3(p, zCenter), size, m, col, collide, cull, r, shadow);
	}

	// ---------------------------------------------------------------- palette helpers
	Color PickPastel()
	{
		static const Color options[] = {pal::Pink, pal::Mint, pal::Cream, pal::Sky, pal::Lavender, pal::Peach, pal::Lemon, pal::Coral, pal::Aqua, pal::White};
		return options[rng.RangeInt(0, 9)];
	}
	Color PickTrim()
	{
		static const Color options[] = {pal::TrimTeal, pal::TrimPink, pal::TrimNavy, pal::TrimGold, pal::White};
		return options[rng.RangeInt(0, 4)];
	}
	Color PickNeon()
	{
		static const Color options[] = {pal::NeonPink, pal::NeonCyan, pal::NeonPurple, pal::NeonOrange, pal::NeonGreen};
		return options[rng.RangeInt(0, 4)];
	}
	Color PickAwning()
	{
		static const Color options[] = {pal::TrimTeal, pal::TrimPink, pal::TrimNavy, pal::Coral, Rgb(40, 120, 70)};
		return options[rng.RangeInt(0, 4)];
	}

	// ---------------------------------------------------------------- building kit
	void Body(const Rect2& r, float baseZ, float height, Color wall)
	{
		Slab(r, baseZ - 20.0f, baseZ + height, Mat::Painted, wall, true, CullClass::Far);
		Footprint fp;
		fp.x0 = r.x0;
		fp.y0 = r.y0;
		fp.x1 = r.x1;
		fp.y1 = r.y1;
		fp.height = height;
		model.footprints.push_back(fp);
		MapR(r, height > 4000.0f ? mappal::BuildingTall : mappal::Building, MapLayer::Building);
	}

	void Cornice(const Rect2& r, float topZ, Color trim)
	{
		Slab(Rect2{r.x0 - 25.0f, r.y0 - 25.0f, r.x1 + 25.0f, r.y1 + 25.0f}, topZ - 35.0f, topZ, Mat::Painted, trim, false, CullClass::Far);
		// Parapet walls.
		const float t = 30.0f;
		const float h = 90.0f;
		Slab(Rect2{r.x0, r.y0, r.x1, r.y0 + t}, topZ, topZ + h, Mat::Painted, trim, true, CullClass::Medium);
		Slab(Rect2{r.x0, r.y1 - t, r.x1, r.y1}, topZ, topZ + h, Mat::Painted, trim, true, CullClass::Medium);
		Slab(Rect2{r.x0, r.y0 + t, r.x0 + t, r.y1 - t}, topZ, topZ + h, Mat::Painted, trim, true, CullClass::Medium);
		Slab(Rect2{r.x1 - t, r.y0 + t, r.x1, r.y1 - t}, topZ, topZ + h, Mat::Painted, trim, true, CullClass::Medium);
	}

	void RoofClutter(const Rect2& r, float topZ)
	{
		const int units = rng.RangeInt(1, 3);
		for (int i = 0; i < units; ++i)
		{
			const float x = rng.Range(r.x0 + 300.0f, r.x1 - 300.0f);
			const float y = rng.Range(r.y0 + 300.0f, r.y1 - 300.0f);
			Box(Vec3{x, y, topZ + 55.0f}, Vec3{160.0f, 110.0f, 110.0f}, Mat::Metal, pal::Metal, true, CullClass::Medium);
			Box(Vec3{x, y, topZ + 112.0f}, Vec3{80.0f, 80.0f, 6.0f}, Mat::Metal, pal::DarkMetal, false, CullClass::Near);
		}
		if (rng.Chance(0.35f))
		{
			const float x = rng.Range(r.x0 + 300.0f, r.x1 - 300.0f);
			const float y = rng.Range(r.y0 + 300.0f, r.y1 - 300.0f);
			Cyl(Vec3{x, y, topZ + 260.0f}, 220.0f, 240.0f, Mat::Matte, pal::Wood, false, CullClass::Medium);
			Cyl(Vec3{x, y, topZ + 70.0f}, 140.0f, 140.0f, Mat::Metal, pal::DarkMetal, false, CullClass::Medium);
		}
	}

	void WindowPane(const Facade& fa, float off, float zc, float w, float h, float litChance, Color glass)
	{
		if (rng.Chance(litChance))
		{
			const float pick = rng.Unit();
			const Color c = pick < 0.68f ? pal::WindowWarm : (pick < 0.92f ? pal::WindowCool : pal::WindowPurple);
			FacadeBox(fa, off, 2.0f, zc, w, 8.0f, h, Mat::WindowLit, c, false, CullClass::Medium, false);
		}
		else
		{
			FacadeBox(fa, off, 2.0f, zc, w, 8.0f, h, Mat::Glass, glass, false, CullClass::Medium, false);
		}
	}

	void PunchedWindows(const Facade& fa, float baseZ, int firstFloor, int lastFloor, float groundH, float floorH, const WindowSpec& ws)
	{
		const float usable = fa.width - 2.0f * ws.margin;
		const int count = static_cast<int>(std::floor((usable + ws.spacing) / (ws.width + ws.spacing)));
		if (count < 1)
		{
			return;
		}
		const float total = static_cast<float>(count) * ws.width + static_cast<float>(count - 1) * ws.spacing;
		const float start = -0.5f * total + 0.5f * ws.width;
		for (int f = firstFloor; f <= lastFloor; ++f)
		{
			const float floorZ = baseZ + (f == 0 ? 0.0f : groundH + static_cast<float>(f - 1) * floorH);
			const float zc = floorZ + ws.sill + 0.5f * ws.height;
			for (int i = 0; i < count; ++i)
			{
				const float off = start + static_cast<float>(i) * (ws.width + ws.spacing);
				WindowPane(fa, off, zc, ws.width, ws.height, ws.litChance, ws.glass);
			}
			if (ws.eyebrows)
			{
				FacadeBox(fa, 0.0f, 25.0f, floorZ + ws.sill + ws.height + 24.0f, usable + 60.0f, 50.0f, 10.0f, Mat::Painted, ws.eyebrowColor, false, CullClass::Medium);
			}
		}
	}

	void BandWindows(const Facade& fa, float baseZ, int firstFloor, int lastFloor, float groundH, float floorH, float margin, float bandH, float litChance, Color glass)
	{
		const float w = fa.width - 2.0f * margin;
		if (w < 100.0f)
		{
			return;
		}
		for (int f = firstFloor; f <= lastFloor; ++f)
		{
			const float floorZ = baseZ + (f == 0 ? 0.0f : groundH + static_cast<float>(f - 1) * floorH);
			WindowPane(fa, 0.0f, floorZ + 0.5f * floorH, w, bandH, litChance, glass);
		}
	}

	struct BuildResult
	{
		Vec3 door;      // ground-level point in front of the entrance
		float yaw = 0.0f; // direction the entrance faces
	};

	static float FaceYaw(Face f)
	{
		switch (f)
		{
		case Face::PosX: return 0.0f;
		case Face::NegX: return 180.0f;
		case Face::PosY: return 90.0f;
		case Face::NegY: return -90.0f;
		}
		return 0.0f;
	}

	BuildResult DecoHotel(const Rect2& lot, Face front, bool isSafehouse)
	{
		const int floors = isSafehouse ? 5 : rng.RangeInt(3, 6);
		const float groundH = 450.0f;
		const float floorH = 330.0f;
		const float height = groundH + static_cast<float>(floors - 1) * floorH;
		const float base = kWalkTopCm;
		const Rect2 body = InsetFacing(lot, front, 300.0f, 150.0f, 100.0f);
		const Color wall = isSafehouse ? pal::Mint : PickPastel();
		const Color trim = isSafehouse ? pal::TrimTeal : PickTrim();
		const Color neon = isSafehouse ? pal::NeonCyan : PickNeon();

		Body(body, base, height, wall);
		Cornice(body, base + height, trim);

		const Facade ff = MakeFacade(body, front);
		WindowSpec ws;
		ws.margin = 380.0f;
		ws.width = 150.0f;
		ws.height = 170.0f;
		ws.spacing = 125.0f;
		ws.litChance = 0.45f;
		ws.eyebrows = true;
		ws.eyebrowColor = trim;
		PunchedWindows(ff, base, 1, floors - 1, groundH, floorH, ws);

		ws.eyebrows = false;
		ws.margin = 260.0f;
		const Face sides[2] = {front == Face::PosY || front == Face::NegY ? Face::PosX : Face::PosY,
			front == Face::PosY || front == Face::NegY ? Face::NegX : Face::NegY};
		for (Face s : sides)
		{
			PunchedWindows(MakeFacade(body, s), base, 1, floors - 1, groundH, floorH, ws);
		}
		const Face back = front == Face::PosY ? Face::NegY : (front == Face::NegY ? Face::PosY : (front == Face::PosX ? Face::NegX : Face::PosX));
		ws.litChance = 0.3f;
		PunchedWindows(MakeFacade(body, back), base, 1, floors - 1, groundH, floorH, ws);

		// Ground floor: lobby glass either side of the door.
		const float w = ff.width;
		FacadeBox(ff, -0.24f * w, 3.0f, base + 190.0f, 0.3f * w, 8.0f, 300.0f, Mat::WindowLit, pal::WindowWarm, false, CullClass::Medium, false);
		FacadeBox(ff, 0.24f * w, 3.0f, base + 190.0f, 0.3f * w, 8.0f, 300.0f, Mat::WindowLit, pal::WindowWarm, false, CullClass::Medium, false);
		FacadeBox(ff, 0.0f, 5.0f, base + 140.0f, 220.0f, 10.0f, 280.0f, Mat::Glass, pal::GlassTeal, false, CullClass::Medium);

		// Porch canopy on columns.
		FacadeBox(ff, 0.0f, 150.0f, base + 400.0f, 0.6f * w, 300.0f, 25.0f, Mat::Painted, trim, true, CullClass::Medium);
		const float colOffsets[4] = {-0.27f, -0.09f, 0.09f, 0.27f};
		for (float k : colOffsets)
		{
			const Vec2 p = ff.mid + ff.along * (k * w) + ff.out * 270.0f;
			Cyl(V3(p, base + 194.0f), 35.0f, 388.0f, Mat::Painted, pal::White, true, CullClass::Medium);
		}

		// Streamline-moderne rounded front corners with wrap-around eyebrows.
		for (int side = -1; side <= 1; side += 2)
		{
			const Vec2 corner = ff.mid + ff.along * (0.5f * w * static_cast<float>(side));
			Cyl(V3(corner, base + 0.5f * (height - 40.0f)), 400.0f, height - 40.0f, Mat::Painted, wall, true, CullClass::Far);
			for (int f = 1; f < floors; ++f)
			{
				const float floorZ = base + groundH + static_cast<float>(f - 1) * floorH;
				Cyl(V3(corner, floorZ + ws.sill + ws.height + 24.0f), 450.0f, 10.0f, Mat::Painted, trim, false, CullClass::Medium);
			}
		}

		// Vertical fin with neon tubes - the classic deco sign tower.
		const float finOff = isSafehouse ? 0.22f * w : (rng.Chance(0.5f) ? 0.0f : 0.22f * w * (rng.Chance(0.5f) ? 1.0f : -1.0f));
		const float finH = height + 500.0f;
		FacadeBox(ff, finOff, 80.0f, base + 0.5f * finH, 70.0f, 160.0f, finH, Mat::Painted, trim, true, CullClass::Far);
		FacadeBox(ff, finOff - 33.0f, 163.0f, base + 0.5f * finH + 100.0f, 10.0f, 10.0f, finH - 200.0f, Mat::Neon, neon, false, CullClass::Medium, false);
		FacadeBox(ff, finOff + 33.0f, 163.0f, base + 0.5f * finH + 100.0f, 10.0f, 10.0f, finH - 200.0f, Mat::Neon, neon, false, CullClass::Medium, false);
		FacadeBox(ff, 0.0f, 6.0f, base + height - 70.0f, 0.7f * w, 10.0f, 14.0f, Mat::Neon, neon, false, CullClass::Medium, false);
		const Vec2 neonPos = ff.mid + ff.along * finOff + ff.out * 260.0f;
		AddPoi(PoiType::NeonLight, V3(neonPos, base + 0.55f * height), 0.0f, 0, "Deco neon", neon);

		// Stepped "ziggurat" crown above the entrance.
		FacadeBox(ff, 0.0f, -320.0f, base + height + 130.0f, 0.34f * w, 600.0f, 260.0f, Mat::Painted, wall, true, CullClass::Far);
		FacadeBox(ff, 0.0f, -320.0f, base + height + 300.0f, 0.18f * w, 420.0f, 100.0f, Mat::Painted, trim, true, CullClass::Far);
		RoofClutter(body.Inset(200.0f), base + height);

		BuildResult result;
		result.door = V3(ff.mid + ff.out * 220.0f, base);
		result.yaw = FaceYaw(front);
		return result;
	}

	BuildResult Shop(const Rect2& lot, Face front, int floorsMin, int floorsMax, bool isCafe, const Color* signOverride)
	{
		const int floors = rng.RangeInt(floorsMin, floorsMax);
		const float groundH = 460.0f;
		const float floorH = 340.0f;
		const float height = groundH + static_cast<float>(floors - 1) * floorH;
		const float base = kWalkTopCm;
		const Rect2 body = InsetFacing(lot, front, 50.0f, 100.0f, 100.0f);
		const Color wall = rng.Chance(0.3f) ? pal::Sand2 : PickPastel();
		const Color trim = PickTrim();

		Body(body, base, height, wall);
		Cornice(body, base + height, trim);

		const Facade ff = MakeFacade(body, front);
		const float w = ff.width;
		// Storefront glass (lit at night - shops stay open late in Vice City).
		FacadeBox(ff, 0.0f, 2.0f, base + 210.0f, w - 300.0f, 8.0f, 300.0f, Mat::WindowLit, pal::WindowWarm, false, CullClass::Medium, false);
		FacadeBox(ff, -0.25f * w, 6.0f, base + 135.0f, 180.0f, 10.0f, 270.0f, Mat::Matte, pal::DarkMetal, false, CullClass::Medium);

		// Striped awning.
		const Color awning = PickAwning();
		const int stripes = std::max(3, static_cast<int>((w - 200.0f) / 170.0f));
		const float stripeW = (w - 200.0f) / static_cast<float>(stripes);
		for (int i = 0; i < stripes; ++i)
		{
			const float off = -0.5f * (w - 200.0f) + (static_cast<float>(i) + 0.5f) * stripeW;
			FacadeBox(ff, off, 110.0f, base + 400.0f, stripeW, 220.0f, 8.0f, Mat::Matte, (i % 2) == 0 ? awning : pal::White, false, CullClass::Medium, true,
				SlopeOutward(front, 14.0f));
		}

		// Sign board.
		const Color sign = signOverride ? *signOverride : PickNeon();
		FacadeBox(ff, 0.0f, 15.0f, base + 520.0f, 0.55f * w, 30.0f, 100.0f, Mat::Neon, sign, false, CullClass::Medium, false);

		WindowSpec ws;
		ws.margin = 200.0f;
		ws.litChance = 0.4f;
		PunchedWindows(ff, base, 1, floors - 1, groundH, floorH, ws);
		const Face back = front == Face::PosY ? Face::NegY : Face::PosY;
		ws.litChance = 0.3f;
		PunchedWindows(MakeFacade(body, back), base, 1, floors - 1, groundH, floorH, ws);
		PunchedWindows(MakeFacade(body, Face::PosX), base, 1, floors - 1, groundH, floorH, ws);
		PunchedWindows(MakeFacade(body, Face::NegX), base, 1, floors - 1, groundH, floorH, ws);

		if (isCafe)
		{
			// Outdoor terrace: tables with parasols on the sidewalk.
			for (int i = -1; i <= 1; ++i)
			{
				const Vec2 p = ff.mid + ff.along * (static_cast<float>(i) * 420.0f + 120.0f) + ff.out * 330.0f;
				Cyl(V3(p, base + 74.0f), 80.0f, 6.0f, Mat::Painted, pal::White, false, CullClass::Near);
				Cyl(V3(p, base + 37.0f), 8.0f, 74.0f, Mat::Metal, pal::DarkMetal, true, CullClass::Near);
				Cyl(V3(p, base + 130.0f), 6.0f, 210.0f, Mat::Metal, pal::Metal, false, CullClass::Near);
				ConeAt(V3(p, base + 240.0f), 240.0f, 60.0f, Mat::Matte, (i == 0) ? pal::White : pal::TrimTeal, CullClass::Near);
				for (int c = -1; c <= 1; c += 2)
				{
					const Vec2 chair = p + ff.along * (static_cast<float>(c) * 65.0f);
					Box(V3(chair, base + 24.0f), Vec3{45.0f, 45.0f, 48.0f}, Mat::Painted, pal::Wood, true, CullClass::Near);
				}
			}
		}

		BuildResult result;
		result.door = V3(ff.mid + ff.along * (-0.25f * w) + ff.out * 250.0f, base);
		result.yaw = FaceYaw(front);
		return result;
	}

	void Nightclub(const Rect2& lot, Face front)
	{
		const int floors = rng.RangeInt(2, 4);
		const float groundH = 500.0f;
		const float floorH = 360.0f;
		const float height = groundH + static_cast<float>(floors - 1) * floorH;
		const float base = kWalkTopCm;
		const Rect2 body = InsetFacing(lot, front, 50.0f, 100.0f, 100.0f);
		static const Color walls[] = {pal::Charcoal, Rgb(64, 42, 84), Rgb(34, 44, 66), pal::Black, Rgb(90, 30, 60)};
		const Color wall = walls[rng.RangeInt(0, 4)];
		const Color neon = PickNeon();
		const Color neon2 = PickNeon();

		Body(body, base, height, wall);
		Cornice(body, base + height, pal::DarkMetal);
		const Facade ff = MakeFacade(body, front);
		const float w = ff.width;

		// Neon outline.
		FacadeBox(ff, -0.5f * w + 10.0f, 8.0f, base + 0.5f * height, 12.0f, 12.0f, height, Mat::Neon, neon, false, CullClass::Medium, false);
		FacadeBox(ff, 0.5f * w - 10.0f, 8.0f, base + 0.5f * height, 12.0f, 12.0f, height, Mat::Neon, neon, false, CullClass::Medium, false);
		FacadeBox(ff, 0.0f, 8.0f, base + height - 30.0f, w - 20.0f, 12.0f, 12.0f, Mat::Neon, neon, false, CullClass::Medium, false);
		FacadeBox(ff, 0.0f, 8.0f, base + groundH, w - 20.0f, 12.0f, 12.0f, Mat::Neon, neon2, false, CullClass::Medium, false);

		// Entrance with lit canopy.
		FacadeBox(ff, 0.0f, 6.0f, base + 150.0f, 260.0f, 12.0f, 300.0f, Mat::Matte, pal::Black, false, CullClass::Medium);
		FacadeBox(ff, 0.0f, 125.0f, base + 340.0f, 0.4f * w, 250.0f, 20.0f, Mat::Metal, pal::DarkMetal, true, CullClass::Medium);
		FacadeBox(ff, 0.0f, 125.0f, base + 329.0f, 0.36f * w, 200.0f, 3.0f, Mat::Light, pal::LampWarm, false, CullClass::Near, false);
		for (int side = -1; side <= 1; side += 2)
		{
			const Vec2 post = ff.mid + ff.along * (static_cast<float>(side) * 220.0f) + ff.out * 220.0f;
			Cyl(V3(post, base + 50.0f), 10.0f, 100.0f, Mat::Metal, pal::TrimGold, true, CullClass::Near);
			Ball(V3(post, base + 104.0f), Vec3{14.0f, 14.0f, 14.0f}, Mat::Metal, pal::TrimGold, CullClass::Near);
		}

		// Blade sign.
		FacadeBox(ff, 0.3f * w, 60.0f, base + groundH + 0.5f * (height - groundH), 100.0f, 40.0f, std::min(520.0f, height - groundH - 40.0f), Mat::Neon, neon2, false,
			CullClass::Medium, false);
		AddPoi(PoiType::NeonLight, V3(ff.mid + ff.out * 300.0f, base + 400.0f), 0.0f, 0, "Club neon", neon);

		const Face back = front == Face::PosY ? Face::NegY : Face::PosY;
		BandWindows(ff, base, 1, floors - 1, groundH, floorH, 150.0f, 160.0f, 0.65f, pal::GlassDark);
		BandWindows(MakeFacade(body, back), base, 1, floors - 1, groundH, floorH, 150.0f, 160.0f, 0.3f, pal::GlassDark);

		// Rooftop bar: railing and parasols.
		const float roofZ = base + height;
		for (int i = 0; i < 2; ++i)
		{
			const Vec2 p{rng.Range(body.x0 + 350.0f, body.x1 - 350.0f), rng.Range(body.y0 + 350.0f, body.y1 - 350.0f)};
			Cyl(V3(p, roofZ + 120.0f), 6.0f, 240.0f, Mat::Metal, pal::Metal, false, CullClass::Near);
			ConeAt(V3(p, roofZ + 250.0f), 300.0f, 70.0f, Mat::Matte, neon2, CullClass::Medium);
		}
	}

	void Office(const Rect2& lot, Face front)
	{
		const int floors = rng.RangeInt(6, 12);
		const float groundH = 520.0f;
		const float floorH = 360.0f;
		const float height = groundH + static_cast<float>(floors - 1) * floorH;
		const float base = kWalkTopCm;
		const Rect2 body = InsetFacing(lot, front, 300.0f, 150.0f, 150.0f);
		static const Color walls[] = {pal::Concrete, pal::Sand2, pal::White, pal::OffWhite, pal::Sky};
		const Color wall = walls[rng.RangeInt(0, 4)];
		const Color glass = rng.Chance(0.5f) ? pal::GlassBlue : pal::GlassTeal;

		Body(body, base, height, wall);
		Cornice(body, base + height, pal::OffWhite);
		const Face faces[4] = {Face::PosX, Face::NegX, Face::PosY, Face::NegY};
		for (Face f : faces)
		{
			BandWindows(MakeFacade(body, f), base, 1, floors - 1, groundH, floorH, 120.0f, 210.0f, 0.35f, glass);
		}
		const Facade ff = MakeFacade(body, front);
		FacadeBox(ff, 0.0f, 3.0f, base + 220.0f, ff.width - 200.0f, 8.0f, 340.0f, Mat::WindowLit, pal::WindowCool, false, CullClass::Medium, false);
		FacadeBox(ff, 0.0f, 150.0f, base + 430.0f, 0.5f * ff.width, 300.0f, 30.0f, Mat::Metal, pal::Metal, true, CullClass::Medium);
		// Mechanical penthouse + antenna.
		const Rect2 pent = body.Inset(std::min(body.Sx(), body.Sy()) * 0.3f);
		Slab(pent, base + height, base + height + 300.0f, Mat::Matte, pal::Concrete, true, CullClass::Far);
		Cyl(Vec3{pent.Cx(), pent.Cy(), base + height + 700.0f}, 20.0f, 800.0f, Mat::Metal, pal::Metal, false, CullClass::Far);
		RoofClutter(body.Inset(150.0f), base + height);
	}

	void Villa(const Rect2& lot, Face front)
	{
		// Local frame: 'a' runs along the street, 'd' is depth from the street edge into the lot.
		const bool posY = front == Face::PosY;
		const float streetEdge = posY ? lot.y1 : lot.y0;
		const float cx = lot.Cx();
		const float halfW = 0.5f * lot.Sx();
		auto at = [&](float a, float d) { return Vec2{cx + a, posY ? streetEdge - d : streetEdge + d}; };
		auto rectAD = [&](float a0, float d0, float a1, float d1) {
			const Vec2 p0 = at(a0, d0);
			const Vec2 p1 = at(a1, d1);
			return Rect2{std::min(p0.x, p1.x), std::min(p0.y, p1.y), std::max(p0.x, p1.x), std::max(p0.y, p1.y)};
		};
		const float base = kWalkTopCm;
		const float lotDepth = lot.Sy();

		// Lawn, driveway, hedges.
		Slab(lot.Inset(40.0f), base - 5.0f, kGrassTopCm, Mat::Foliage, pal::Grass, true, CullClass::Far);
		MapR(lot.Inset(40.0f), mappal::Park, MapLayer::Park);
		Slab(rectAD(halfW - 520.0f, 0.0f, halfW - 140.0f, 900.0f), base - 5.0f, kPathTopCm, Mat::Matte, pal::Concrete, true, CullClass::Medium);
		Slab(rectAD(-halfW + 60.0f, lotDepth - 120.0f, halfW - 60.0f, lotDepth - 40.0f), base, base + 140.0f, Mat::Foliage, pal::Hedge, true, CullClass::Medium);
		Slab(rectAD(-halfW + 40.0f, 60.0f, -halfW + 120.0f, lotDepth - 40.0f), base, base + 140.0f, Mat::Foliage, pal::Hedge, true, CullClass::Medium);
		Slab(rectAD(halfW - 120.0f, 960.0f, halfW - 40.0f, lotDepth - 40.0f), base, base + 140.0f, Mat::Foliage, pal::Hedge, true, CullClass::Medium);
		Slab(rectAD(-halfW + 120.0f, 40.0f, halfW - 560.0f, 70.0f), base, base + 90.0f, Mat::Painted, pal::White, true, CullClass::Medium);

		// House: ground volume + cantilevered upper volume.
		const Color wall = rng.Chance(0.6f) ? pal::White : PickPastel();
		const Color accent = rng.Chance(0.5f) ? pal::Wood : pal::Charcoal;
		const Rect2 lower = rectAD(-halfW + 300.0f, 900.0f, halfW - 300.0f, 2200.0f);
		Body(lower, base, 380.0f, wall);
		Slab(lower.Inset(-60.0f), base + 380.0f, base + 405.0f, Mat::Painted, pal::OffWhite, true, CullClass::Medium);
		const Rect2 upper = rectAD(-halfW + 450.0f, 800.0f, halfW - 750.0f, 1900.0f);
		Slab(upper, base + 405.0f, base + 740.0f, Mat::Painted, accent, true, CullClass::Far);
		Slab(upper.Inset(-50.0f), base + 740.0f, base + 765.0f, Mat::Painted, pal::OffWhite, true, CullClass::Medium);

		const Face back = posY ? Face::NegY : Face::PosY;
		const Facade lowerFront = MakeFacade(lower, front);
		FacadeBox(lowerFront, -0.15f * lowerFront.width, 3.0f, base + 180.0f, 0.5f * lowerFront.width, 8.0f, 280.0f, Mat::WindowLit, pal::WindowWarm, false,
			CullClass::Medium, false);
		const Vec2 garageCenter = at(halfW - 330.0f, 900.0f);
		Box(V3(garageCenter + MakeFacade(lower, front).out * 4.0f, base + 140.0f), lowerFront.alongX ? Vec3{320.0f, 10.0f, 260.0f} : Vec3{10.0f, 320.0f, 260.0f},
			Mat::Painted, pal::Concrete, false, CullClass::Medium);
		BandWindows(MakeFacade(upper, front), base, 1, 1, 405.0f, 335.0f, 80.0f, 220.0f, 0.55f, pal::GlassDark);
		BandWindows(MakeFacade(upper, back), base, 1, 1, 405.0f, 335.0f, 80.0f, 220.0f, 0.55f, pal::GlassDark);
		FacadeBox(MakeFacade(lower, back), 0.0f, 3.0f, base + 180.0f, 0.7f * lowerFront.width, 8.0f, 280.0f, Mat::WindowLit, pal::WindowWarm, false,
			CullClass::Medium, false);

		// Pool and deck in the back garden.
		const float poolHalf = std::min(450.0f, halfW - 400.0f);
		const Rect2 deck = rectAD(-poolHalf - 150.0f, 2350.0f, poolHalf + 150.0f, 3500.0f);
		Slab(deck, base, kPathTopCm + 1.0f, Mat::Matte, pal::OffWhite, true, CullClass::Medium);
		const Rect2 pool = rectAD(-poolHalf, 2500.0f, poolHalf, 3350.0f);
		Slab(pool, base, kPathTopCm + 1.5f, Mat::Water, pal::Pool, false, CullClass::Medium, false);
		MapR(pool, mappal::Pool, MapLayer::Pool);
		for (int i = -1; i <= 1; i += 2)
		{
			const Vec2 chair = at(static_cast<float>(i) * (poolHalf + 80.0f), 2420.0f);
			Box(V3(chair, kPathTopCm + 18.0f), Vec3{70.0f, 70.0f, 30.0f}, Mat::Painted, pal::White, true, CullClass::Near);
		}
		Palm(V3(at(-halfW + 260.0f, 3000.0f), kGrassTopCm), rng.Range(700.0f, 950.0f));
		Palm(V3(at(halfW - 260.0f, 400.0f), kGrassTopCm), rng.Range(650.0f, 900.0f));
	}

	void TowerBlock(const Rect2& block, int towerIndex)
	{
		const float base = kWalkTopCm;
		const Rect2 podium = Rect2{block.Cx() - 3000.0f, block.Cy() - 2300.0f, block.Cx() + 3000.0f, block.Cy() + 2300.0f};
		const float podiumH = 1300.0f;
		const Color podiumColor = towerIndex == 1 ? pal::Sand2 : pal::OffWhite;
		Body(podium, base, podiumH, podiumColor);
		Cornice(podium, base + podiumH, pal::White);
		const Face faces[4] = {Face::PosX, Face::NegX, Face::PosY, Face::NegY};
		for (Face f : faces)
		{
			const Facade fa = MakeFacade(podium, f);
			FacadeBox(fa, 0.0f, 3.0f, base + 230.0f, fa.width - 400.0f, 8.0f, 360.0f, Mat::WindowLit, pal::WindowWarm, false, CullClass::Medium, false);
			BandWindows(fa, base, 1, 2, 500.0f, 400.0f, 200.0f, 220.0f, 0.4f, pal::GlassBlue);
		}

		const int floors = 26 + towerIndex * 6 + rng.RangeInt(0, 4);
		const float floorH = 360.0f;
		const float towerH = static_cast<float>(floors) * floorH;
		const Rect2 tower = Rect2{block.Cx() - 1500.0f, block.Cy() - 1500.0f, block.Cx() + 1500.0f, block.Cy() + 1500.0f};
		const Color shaft = towerIndex == 0 ? pal::White : (towerIndex == 1 ? Rgb(210, 214, 222) : Rgb(232, 222, 210));
		const Color glass = towerIndex == 2 ? pal::GlassTeal : pal::GlassBlue;
		const float towerBase = base + podiumH;
		Body(tower, towerBase, towerH, shaft);
		for (Face f : faces)
		{
			BandWindows(MakeFacade(tower, f), towerBase, 1, floors, 0.0f, floorH, 150.0f, 250.0f, 0.38f, glass);
		}
		// Crown with neon ring and beacon.
		const float topZ = towerBase + towerH;
		const Rect2 crown = tower.Inset(350.0f);
		Slab(crown, topZ, topZ + 500.0f, Mat::Glass, glass, true, CullClass::Far);
		const Color ring = towerIndex == 0 ? pal::NeonCyan : (towerIndex == 1 ? pal::NeonPink : pal::NeonWhite);
		Slab(Rect2{tower.x0 - 10.0f, tower.y0 - 10.0f, tower.x1 + 10.0f, tower.y1 + 10.0f}, topZ - 40.0f, topZ, Mat::Neon, ring, false, CullClass::Far, false);
		Slab(Rect2{crown.x0 - 10.0f, crown.y0 - 10.0f, crown.x1 + 10.0f, crown.y1 + 10.0f}, topZ + 480.0f, topZ + 510.0f, Mat::Neon, ring, false, CullClass::Far, false);
		Cyl(Vec3{tower.Cx(), tower.Cy(), topZ + 1300.0f}, 40.0f, 1600.0f, Mat::Metal, pal::Metal, false, CullClass::Far);
		Ball(Vec3{tower.Cx(), tower.Cy(), topZ + 2120.0f}, Vec3{70.0f, 70.0f, 70.0f}, Mat::Light, pal::BeaconRed, CullClass::Never);

		// Plaza palms.
		const Vec2 corners[4] = {{block.x0 + 900.0f, block.y0 + 900.0f}, {block.x1 - 900.0f, block.y0 + 900.0f}, {block.x0 + 900.0f, block.y1 - 900.0f},
			{block.x1 - 900.0f, block.y1 - 900.0f}};
		for (const Vec2& c : corners)
		{
			Cyl(V3(c, base + 40.0f), 300.0f, 80.0f, Mat::Painted, pal::White, true, CullClass::Medium);
			Slab(Rect2{c.x - 140.0f, c.y - 140.0f, c.x + 140.0f, c.y + 140.0f}, base + 40.0f, base + 82.0f, Mat::Foliage, pal::GrassDark, false, CullClass::Near);
			Palm(V3(c, base + 82.0f), rng.Range(900.0f, 1250.0f));
		}
	}

	// ---------------------------------------------------------------- nature & props
	void Palm(Vec3 base, float height)
	{
		const float lean = rng.Range(2.0f, 13.0f);
		const float leanYaw = rng.Range(0.0f, 360.0f);
		const int segments = 6;
		const float segLen = height / static_cast<float>(segments);
		Vec3 pos = base;
		for (int i = 0; i < segments; ++i)
		{
			const float angle = lean * static_cast<float>(i + 1) / static_cast<float>(segments);
			const Rot r{-angle, leanYaw, 0.0f};
			const Vec3 dir = BasisFromRot(r).z;
			const float dia = Lerp(44.0f, 28.0f, static_cast<float>(i) / static_cast<float>(segments - 1));
			Cyl(pos + dir * (0.5f * segLen), dia, segLen * 1.06f, Mat::Matte, (i % 2) == 0 ? pal::PalmTrunk : pal::PalmTrunkDark, i < 2, CullClass::Far, r);
			pos = pos + dir * segLen;
		}
		Ball(pos, Vec3{60.0f, 60.0f, 50.0f}, Mat::Foliage, pal::PalmLeafDark, CullClass::Medium);
		const int fronds = 8;
		for (int k = 0; k < fronds; ++k)
		{
			const float yaw = static_cast<float>(k) * (360.0f / static_cast<float>(fronds)) + rng.Range(-14.0f, 14.0f);
			const float droop = rng.Range(-40.0f, -12.0f);
			const float len = rng.Range(320.0f, 420.0f);
			const Color leaf = (k % 2) == 0 ? pal::PalmLeaf : pal::PalmLeafDark;
			const Rot inner{droop, yaw, 0.0f};
			const Vec3 innerDir = BasisFromRot(inner).x;
			const float innerLen = len * 0.55f;
			Box(pos + innerDir * (0.5f * innerLen), Vec3{innerLen, 90.0f, 6.0f}, Mat::Foliage, leaf, false, CullClass::Medium, inner, true);
			const Vec3 tipStart = pos + innerDir * innerLen;
			const Rot outer{droop - 28.0f, yaw, 0.0f};
			const Vec3 outerDir = BasisFromRot(outer).x;
			const float outerLen = len * 0.5f;
			Box(tipStart + outerDir * (0.5f * outerLen), Vec3{outerLen, 70.0f, 5.0f}, Mat::Foliage, leaf, false, CullClass::Medium, outer, true);
		}
		for (int c = 0; c < 3; ++c)
		{
			const float a = static_cast<float>(c) * 120.0f * kDegToRad;
			Ball(pos + Vec3{std::cos(a) * 22.0f, std::sin(a) * 22.0f, -28.0f}, Vec3{22.0f, 22.0f, 24.0f}, Mat::Matte, pal::Coconut, CullClass::Near);
		}
	}

	void Lamp(Vec2 p, Vec2 toRoad, float baseZ)
	{
		Cyl(V3(p, baseZ + 400.0f), 18.0f, 800.0f, Mat::Metal, pal::DarkMetal, true, CullClass::Medium);
		Cyl(V3(p, baseZ + 40.0f), 34.0f, 80.0f, Mat::Metal, pal::DarkMetal, true, CullClass::Near);
		const bool alongX = Abs(toRoad.x) > 0.5f;
		Box(V3(p + toRoad * 90.0f, baseZ + 792.0f), alongX ? Vec3{180.0f, 12.0f, 10.0f} : Vec3{12.0f, 180.0f, 10.0f}, Mat::Metal, pal::DarkMetal, false, CullClass::Medium);
		Box(V3(p + toRoad * 175.0f, baseZ + 778.0f), alongX ? Vec3{64.0f, 30.0f, 14.0f} : Vec3{30.0f, 64.0f, 14.0f}, Mat::Light, pal::LampWarm, false, CullClass::Medium,
			Rot{}, false);
		AddPoi(PoiType::StreetLight, V3(p + toRoad * 175.0f, baseZ + 760.0f), 0.0f, 0, "Street light", pal::LampWarm);
	}

	void Bench(Vec2 p, float yaw, float baseZ)
	{
		const Vec3 o = V3(p, baseZ);
		LocalBox(o, yaw, Vec3{0.0f, 0.0f, 45.0f}, Vec3{50.0f, 180.0f, 8.0f}, Mat::Matte, pal::Wood, true, CullClass::Near);
		LocalBox(o, yaw, Vec3{-24.0f, 0.0f, 75.0f}, Vec3{6.0f, 180.0f, 40.0f}, Mat::Matte, pal::Wood, false, CullClass::Near);
		LocalBox(o, yaw, Vec3{0.0f, -75.0f, 21.0f}, Vec3{45.0f, 8.0f, 42.0f}, Mat::Metal, pal::DarkMetal, false, CullClass::Near);
		LocalBox(o, yaw, Vec3{0.0f, 75.0f, 21.0f}, Vec3{45.0f, 8.0f, 42.0f}, Mat::Metal, pal::DarkMetal, false, CullClass::Near);
	}

	void Umbrella(Vec2 p, Color canopy)
	{
		Cyl(V3(p, kSandTopCm + 120.0f), 6.0f, 240.0f, Mat::Metal, pal::Metal, false, CullClass::Near);
		ConeAt(V3(p, kSandTopCm + 245.0f), 260.0f, 70.0f, Mat::Matte, canopy, CullClass::Medium);
		const float yaw = 90.0f + rng.Range(-20.0f, 20.0f);
		for (int i = -1; i <= 1; i += 2)
		{
			const Basis b = BasisFromRot(Rot{0.0f, yaw, 0.0f});
			const Vec3 o = V3(p, kSandTopCm) + b.y * (static_cast<float>(i) * 90.0f) + b.x * -60.0f;
			LocalBox(o, yaw, Vec3{0.0f, 0.0f, 28.0f}, Vec3{190.0f, 64.0f, 10.0f}, Mat::Painted, pal::White, false, CullClass::Near);
			LocalBox(o, yaw, Vec3{-80.0f, 0.0f, 55.0f}, Vec3{60.0f, 64.0f, 8.0f}, Mat::Painted, pal::White, false, CullClass::Near, 40.0f);
			LocalBox(o, yaw, Vec3{0.0f, 0.0f, 34.0f}, Vec3{150.0f, 58.0f, 4.0f}, Mat::Matte, canopy, false, CullClass::Near);
		}
	}

	// Returns a walkable point on the observation deck.
	Vec3 LifeguardTower(Vec2 p, Color cabin)
	{
		const float yaw = 90.0f; // local +X faces the ocean (+Y)
		const Vec3 o = V3(p, kSandTopCm);
		const float deckZ = 230.0f;
		for (int ix = -1; ix <= 1; ix += 2)
		{
			for (int iy = -1; iy <= 1; iy += 2)
			{
				LocalCyl(o, yaw, Vec3{static_cast<float>(ix) * 125.0f, static_cast<float>(iy) * 125.0f, 0.5f * deckZ}, 20.0f, deckZ, Mat::Painted, pal::White, true,
					CullClass::Medium);
			}
		}
		LocalBox(o, yaw, Vec3{0.0f, 0.0f, deckZ - 10.0f}, Vec3{320.0f, 320.0f, 20.0f}, Mat::Matte, pal::Wood, true, CullClass::Medium);
		// Cabin on the left half of the deck, walkway on the right half.
		LocalBox(o, yaw, Vec3{-40.0f, -60.0f, deckZ + 100.0f}, Vec3{200.0f, 180.0f, 200.0f}, Mat::Painted, cabin, true, CullClass::Medium);
		LocalBox(o, yaw, Vec3{-40.0f, -60.0f, deckZ + 100.0f}, Vec3{206.0f, 186.0f, 30.0f}, Mat::Painted, pal::White, false, CullClass::Medium);
		LocalBox(o, yaw, Vec3{62.0f, -60.0f, deckZ + 140.0f}, Vec3{6.0f, 150.0f, 70.0f}, Mat::Glass, pal::GlassTeal, false, CullClass::Medium);
		LocalBox(o, yaw, Vec3{-40.0f, -60.0f, deckZ + 212.0f}, Vec3{250.0f, 230.0f, 14.0f}, Mat::Painted, pal::TrimPink, false, CullClass::Medium, -8.0f);
		// Ramp from the sand up to the walkway (walkable, ~20 degrees).
		const float rampLen = 640.0f;
		const float rampAngle = std::atan2(deckZ, rampLen) * kRadToDeg;
		LocalBox(o, yaw, Vec3{-160.0f - 0.5f * rampLen * std::cos(rampAngle * kDegToRad), 95.0f, 0.5f * deckZ - 4.0f}, Vec3{rampLen + 20.0f, 110.0f, 14.0f},
			Mat::Matte, pal::Wood, true, CullClass::Medium, rampAngle);
		// Front railing.
		LocalBox(o, yaw, Vec3{152.0f, 0.0f, deckZ + 50.0f}, Vec3{6.0f, 320.0f, 8.0f}, Mat::Painted, pal::White, false, CullClass::Near);
		const Basis b = BasisFromRot(Rot{0.0f, yaw, 0.0f});
		return o + Apply(b, Vec3{90.0f, 95.0f, deckZ});
	}

	void Yacht(Vec2 p, float yaw, float length, float beam)
	{
		const Vec3 o = V3(p, 0.0f);
		const float hullTop = 90.0f;
		const float hullBottom = -140.0f;
		const float hullH = hullTop - hullBottom;
		const float hullZ = 0.5f * (hullTop + hullBottom);
		LocalBox(o, yaw, Vec3{-0.08f * length, 0.0f, hullZ}, Vec3{length * 0.84f, beam, hullH}, Mat::Glossy, pal::White, true, CullClass::Medium);
		const float bowSide = beam / std::sqrt(2.0f);
		LocalBox(o, yaw, Vec3{0.34f * length, 0.0f, hullZ}, Vec3{bowSide, bowSide, hullH}, Mat::Glossy, pal::White, true, CullClass::Medium, 0.0f, 0.0f);
		// Rotate the bow block 45 degrees by adding to yaw.
		model.prims.back().rot.yaw = yaw + 45.0f;
		LocalBox(o, yaw, Vec3{-0.08f * length, 0.0f, -10.0f}, Vec3{length * 0.84f + 4.0f, beam + 4.0f, 30.0f}, Mat::Glossy, pal::TrimNavy, false, CullClass::Medium);
		LocalBox(o, yaw, Vec3{-0.08f * length, 0.0f, hullTop + 5.0f}, Vec3{length * 0.8f, beam - 20.0f, 10.0f}, Mat::Matte, pal::Wood, true, CullClass::Medium);
		LocalBox(o, yaw, Vec3{-0.12f * length, 0.0f, hullTop + 100.0f}, Vec3{length * 0.45f, beam * 0.75f, 180.0f}, Mat::Glossy, pal::White, true, CullClass::Medium);
		LocalBox(o, yaw, Vec3{-0.12f * length, 0.0f, hullTop + 120.0f}, Vec3{length * 0.45f + 4.0f, beam * 0.75f + 4.0f, 60.0f}, Mat::Glass, pal::GlassDark, false,
			CullClass::Medium);
		LocalBox(o, yaw, Vec3{-0.16f * length, 0.0f, hullTop + 230.0f}, Vec3{length * 0.25f, beam * 0.6f, 80.0f}, Mat::Glossy, pal::White, true, CullClass::Medium);
		LocalBox(o, yaw, Vec3{-0.16f * length, 0.0f, hullTop + 330.0f}, Vec3{40.0f, beam * 0.5f, 12.0f}, Mat::Metal, pal::Metal, false, CullClass::Near);
	}

	// ---------------------------------------------------------------- districts
	std::vector<std::pair<float, float>> SplitLots(float x0, float x1, float minW, float maxW, float gap)
	{
		std::vector<std::pair<float, float>> lots;
		float x = x0;
		while (x1 - x >= minW)
		{
			float w = rng.Range(minW, maxW);
			if (x1 - (x + w + gap) < minW)
			{
				w = x1 - x;
			}
			lots.emplace_back(x, x + w);
			x += w + gap;
		}
		return lots;
	}

	static int NearestLot(const std::vector<std::pair<float, float>>& lots, float targetX)
	{
		int best = 0;
		float bestDist = 1e30f;
		for (size_t i = 0; i < lots.size(); ++i)
		{
			const float d = Abs(0.5f * (lots[i].first + lots[i].second) - targetX);
			if (d < bestDist)
			{
				bestDist = d;
				best = static_cast<int>(i);
			}
		}
		return best;
	}

	void BuildTerrain()
	{
		// Sea floor and ocean surface.
		Box(Vec3{0.0f, 0.0f, kDeepCm - 500.0f}, Vec3{6000000.0f, 6000000.0f, 1000.0f}, Mat::Sand, pal::SeaFloor, true, CullClass::Never, Rot{}, false);
		Add(Shape::Cube, Vec3{0.0f, 0.0f, model.seaLevel - 1.0f}, Vec3{6000000.0f, 6000000.0f, 2.0f}, Rot{}, Mat::Water, pal::Ocean, false, CullClass::Never, false);

		// Island land mass (top hidden under roads / sidewalks; sides read as the seawall).
		const Rect2 island = RectM(kIslandX0, kIslandY0, kIslandX1, kIslandY1);
		Slab(island, kDeepCm, -2.0f, Mat::Matte, pal::Seawall, true, CullClass::Never);
		MapR(island, mappal::Road, MapLayer::Land);

		// Beach: dry flat sand, wet slope into the surf, then the drop-off.
		Slab(RectM(kIslandX0, kIslandY1, kIslandX1, 300.0f), kDeepCm, kSandTopCm, Mat::Sand, pal::Sand, true, CullClass::Never);
		SlopeSlab(300.0f * kM, kSandTopCm, 370.0f * kM, -260.0f, pal::SandWet);
		SlopeSlab(370.0f * kM, -260.0f, 560.0f * kM, kDeepCm, pal::SeaFloor);
		const float waterline = 300.0f + 70.0f * ((kSandTopCm - model.seaLevel) / (kSandTopCm - -260.0f));
		MapR(RectM(kIslandX0, kIslandY1, kIslandX1, waterline), mappal::Beach, MapLayer::Beach);
	}

	// Slab whose top surface runs from (y0, z0) to (y1, z1) across the full island length.
	void SlopeSlab(float y0, float z0, float y1, float z1, Color col)
	{
		const float thickness = 300.0f;
		const float dy = y1 - y0;
		const float dz = z1 - z0;
		const float len = std::sqrt(dy * dy + dz * dz);
		const float roll = std::atan2(-dz, dy) * kRadToDeg; // positive = descends towards +Y
		const Basis b = BasisFromRot(Rot{0.0f, 0.0f, roll});
		const Vec3 topMid{0.5f * (kIslandX0 + kIslandX1) * kM, 0.5f * (y0 + y1), 0.5f * (z0 + z1)};
		const Vec3 center = topMid - b.z * (0.5f * thickness);
		Box(center, Vec3{(kIslandX1 - kIslandX0) * kM, len + 40.0f, thickness}, Mat::Sand, col, true, CullClass::Never, Rot{0.0f, 0.0f, roll}, false);
	}

	void BuildRoads()
	{
		const int nodeBase = model.graph.NumNodes();
		// Avenues (north-south).
		for (int a = 0; a < kAvenueCount; ++a)
		{
			const float y = kAvenueY[a];
			const Rect2 road = RectM(kAvenueX0, y - kAvenueHalf, kAvenueX1, y + kAvenueHalf);
			Slab(road, -12.0f, kRoadTopCm, Mat::Asphalt, pal::Asphalt, true, CullClass::Never, false);
			RoadSeg seg;
			seg.a = Vec2{kStreetX[0] * kM, y * kM};
			seg.b = Vec2{kStreetX[kStreetCount - 1] * kM, y * kM};
			seg.width = 2.0f * kAvenueHalf * kM;
			seg.name = kAvenueNames[a];
			model.roads.push_back(seg);
			// Markings per block segment (skip intersections).
			for (int s = 0; s + 1 < kStreetCount; ++s)
			{
				const float xa = (kStreetX[s] + kStreetHalf + 1.0f) * kM;
				const float xb = (kStreetX[s + 1] - kStreetHalf - 1.0f) * kM;
				const float yc = y * kM;
				Box(Vec3{0.5f * (xa + xb), yc - 14.0f, 0.0f}, Vec3{xb - xa, 12.0f, 2.0f}, Mat::Matte, pal::LaneYellow, false, CullClass::Near, Rot{}, false);
				Box(Vec3{0.5f * (xa + xb), yc + 14.0f, 0.0f}, Vec3{xb - xa, 12.0f, 2.0f}, Mat::Matte, pal::LaneYellow, false, CullClass::Near, Rot{}, false);
				for (float x = xa + 150.0f; x + 300.0f < xb; x += 900.0f)
				{
					Box(Vec3{x + 150.0f, yc - 400.0f, 0.0f}, Vec3{300.0f, 12.0f, 2.0f}, Mat::Matte, pal::LaneWhite, false, CullClass::Near, Rot{}, false);
					Box(Vec3{x + 150.0f, yc + 400.0f, 0.0f}, Vec3{300.0f, 12.0f, 2.0f}, Mat::Matte, pal::LaneWhite, false, CullClass::Near, Rot{}, false);
				}
			}
		}
		// Streets (east-west).
		for (int s = 0; s < kStreetCount; ++s)
		{
			const float x = kStreetX[s];
			const Rect2 road = RectM(x - kStreetHalf, kStreetY0, x + kStreetHalf, kStreetY1);
			Slab(road, -12.0f, kRoadTopCm - 0.5f, Mat::Asphalt, pal::Asphalt, true, CullClass::Never, false);
			RoadSeg seg;
			seg.a = Vec2{x * kM, kAvenueY[0] * kM};
			seg.b = Vec2{x * kM, kAvenueY[kAvenueCount - 1] * kM};
			seg.width = 2.0f * kStreetHalf * kM;
			seg.name = kStreetNames[s];
			model.roads.push_back(seg);
			for (int a = 0; a + 1 < kAvenueCount; ++a)
			{
				const float ya = (kAvenueY[a] + kAvenueHalf + 1.0f) * kM;
				const float yb = (kAvenueY[a + 1] - kAvenueHalf - 1.0f) * kM;
				Box(Vec3{x * kM, 0.5f * (ya + yb), 0.0f}, Vec3{12.0f, yb - ya, 2.0f}, Mat::Matte, pal::LaneYellow, false, CullClass::Near, Rot{}, false);
			}
		}

		// Crosswalks at every intersection approach.
		for (int s = 0; s < kStreetCount; ++s)
		{
			for (int a = 0; a < kAvenueCount; ++a)
			{
				const float sx = kStreetX[s] * kM;
				const float ay = kAvenueY[a] * kM;
				// Across the avenue (north / south of the junction).
				for (int side = -1; side <= 1; side += 2)
				{
					const bool exists = side < 0 ? s > 0 : s < kStreetCount - 1;
					if (!exists)
					{
						continue;
					}
					const float cx = sx + static_cast<float>(side) * (kStreetHalf + 2.5f) * kM;
					for (float y = ay - kAvenueHalf * kM + 80.0f; y < ay + kAvenueHalf * kM - 40.0f; y += 110.0f)
					{
						Box(Vec3{cx, y, 0.0f}, Vec3{300.0f, 55.0f, 2.0f}, Mat::Matte, pal::LaneWhite, false, CullClass::Near, Rot{}, false);
					}
				}
				// Across the street (east / west of the junction).
				for (int side = -1; side <= 1; side += 2)
				{
					const bool exists = side < 0 ? a > 0 : a < kAvenueCount - 1;
					if (!exists)
					{
						continue;
					}
					const float cy = ay + static_cast<float>(side) * (kAvenueHalf + 2.5f) * kM;
					for (float x = sx - kStreetHalf * kM + 80.0f; x < sx + kStreetHalf * kM - 40.0f; x += 110.0f)
					{
						Box(Vec3{x, cy, 0.0f}, Vec3{55.0f, 300.0f, 2.0f}, Mat::Matte, pal::LaneWhite, false, CullClass::Near, Rot{}, false);
					}
				}
			}
		}

		// Road graph: grid of intersections.
		for (int s = 0; s < kStreetCount; ++s)
		{
			for (int a = 0; a < kAvenueCount; ++a)
			{
				model.graph.AddNode(Vec2{kStreetX[s] * kM, kAvenueY[a] * kM});
			}
		}
		auto nodeAt = [nodeBase](int s, int a) { return nodeBase + s * kAvenueCount + a; };
		for (int s = 0; s < kStreetCount; ++s)
		{
			for (int a = 0; a < kAvenueCount; ++a)
			{
				if (s + 1 < kStreetCount)
				{
					model.graph.AddEdge(nodeAt(s, a), nodeAt(s + 1, a));
				}
				if (a + 1 < kAvenueCount)
				{
					model.graph.AddEdge(nodeAt(s, a), nodeAt(s, a + 1));
				}
			}
		}
		// Causeway node.
		const int causeway = model.graph.AddNode(Vec2{0.0f, (kCausewayEndY + 10.0f) * kM});
		model.graph.AddEdge(nodeAt(5, 0), causeway);
	}

	void BuildCauseway()
	{
		const float y0 = kCausewayEndY * kM;
		const float y1 = kIslandY0 * kM;
		const Rect2 deck{-kCausewayHalf * kM, y0, kCausewayHalf * kM, y1 + 50.0f};
		Slab(deck, -90.0f, kRoadTopCm, Mat::Asphalt, pal::Asphalt, true, CullClass::Never, false);
		MapR(deck, mappal::Road, MapLayer::Road);
		RoadSeg seg;
		seg.a = Vec2{0.0f, kAvenueY[0] * kM};
		seg.b = Vec2{0.0f, y0};
		seg.width = 2.0f * kCausewayHalf * kM;
		seg.name = "Vice Causeway";
		model.roads.push_back(seg);
		// Jersey barriers and centre line.
		for (int side = -1; side <= 1; side += 2)
		{
			const float x = static_cast<float>(side) * (kCausewayHalf * kM - 25.0f);
			Box(Vec3{x, 0.5f * (y0 + y1), 40.0f}, Vec3{40.0f, y1 - y0, 80.0f}, Mat::Matte, pal::Concrete, true, CullClass::Far);
		}
		Box(Vec3{-14.0f, 0.5f * (y0 + y1), 0.0f}, Vec3{12.0f, y1 - y0, 2.0f}, Mat::Matte, pal::LaneYellow, false, CullClass::Near, Rot{}, false);
		Box(Vec3{14.0f, 0.5f * (y0 + y1), 0.0f}, Vec3{12.0f, y1 - y0, 2.0f}, Mat::Matte, pal::LaneYellow, false, CullClass::Near, Rot{}, false);
		// Piers.
		for (float y = y1 - 2000.0f; y > y0; y -= 4000.0f)
		{
			Cyl(Vec3{0.0f, y, 0.5f * (kDeepCm + -90.0f)}, 260.0f, -90.0f - kDeepCm, Mat::Matte, pal::Concrete, true, CullClass::Far);
		}
		// Lamps along the causeway.
		for (float y = y1 - 1500.0f; y > y0 + 500.0f; y -= 6000.0f)
		{
			Lamp(Vec2{(kCausewayHalf - 0.6f) * kM, y}, Vec2{-1.0f, 0.0f}, kRoadTopCm);
		}
		// "Road closed" barrier - the mainland opens in a later phase.
		for (int i = -3; i <= 3; ++i)
		{
			Box(Vec3{static_cast<float>(i) * 190.0f, y0 + 200.0f, 50.0f}, Vec3{160.0f, 60.0f, 100.0f}, Mat::Painted, (i % 2) == 0 ? pal::NeonRed : pal::White, true,
				CullClass::Medium);
		}
		Box(Vec3{0.0f, y0 + 210.0f, 210.0f}, Vec3{900.0f, 12.0f, 120.0f}, Mat::Neon, pal::NeonOrange, false, CullClass::Medium);
		Box(Vec3{0.0f, y0 + 50.0f, 150.0f}, Vec3{1400.0f, 100.0f, 300.0f}, Mat::Matte, pal::Concrete, true, CullClass::Medium);
	}

	void BuildBlocks()
	{
		for (int row = 0; row + 1 < kStreetCount; ++row)
		{
			for (int col = 0; col + 1 < kAvenueCount; ++col)
			{
				const Rect2 block = RectM(kStreetX[row] + kStreetHalf, kAvenueY[col] + kAvenueHalf, kStreetX[row + 1] - kStreetHalf, kAvenueY[col + 1] - kAvenueHalf);
				Slab(block, -12.0f, kWalkTopCm, Mat::Matte, pal::Sidewalk, true, CullClass::Far);
				MapR(block, mappal::Block, MapLayer::Block);
				BlockContents(row, col, block);
				BlockStreetFurniture(col, block);
			}
		}
	}

	void BlockStreetFurniture(int col, const Rect2& block)
	{
		// Lamps on both avenue-facing sidewalks.
		const float fractions[2] = {0.25f, 0.75f};
		for (float f : fractions)
		{
			const float x = block.x0 + f * block.Sx();
			Lamp(Vec2{x, block.y0 + 120.0f}, Vec2{0.0f, -1.0f}, kWalkTopCm);
			Lamp(Vec2{x, block.y1 - 120.0f}, Vec2{0.0f, 1.0f}, kWalkTopCm);
		}
		// Palm-lined boulevards: Flamingo Avenue (both sides) and Seabreeze Boulevard (west side).
		const float palmFractions[3] = {0.06f, 0.5f, 0.94f};
		for (float f : palmFractions)
		{
			const float x = block.x0 + f * block.Sx();
			if (col == 1 || col == 3)
			{
				Palm(Vec3{x, block.y1 - 230.0f, kWalkTopCm}, rng.Range(750.0f, 1050.0f));
			}
			if (col == 2)
			{
				Palm(Vec3{x, block.y0 + 230.0f, kWalkTopCm}, rng.Range(750.0f, 1050.0f));
			}
		}
	}

	void BlockContents(int row, int col, const Rect2& block)
	{
		if (row == 0)
		{
			if (col < 3)
			{
				TowerBlock(block, col);
			}
			else
			{
				Park(block, true, "South Point Park");
				AddCollectible(Vec3{block.Cx() + 1500.0f, block.Cy() - 1500.0f, kGrassTopCm});
			}
			return;
		}
		const Rect2 buildable = block.Inset(500.0f);
		switch (col)
		{
		case 0: VillaBlock(buildable); break;
		case 1:
			if (row == 4)
			{
				Park(block, true, "Flamingo Park");
				AddCollectible(Vec3{block.Cx(), block.Cy() + 1200.0f, kPathTopCm});
			}
			else
			{
				MarketBlock(buildable, row);
			}
			break;
		case 2:
			if (row == 3)
			{
				ParkingLot(block);
			}
			else if (row >= 5)
			{
				NeonBlock(buildable, row);
			}
			else
			{
				OfficeBlock(buildable);
			}
			break;
		default: BeachfrontBlock(buildable, row); break;
		}
	}

	void VillaBlock(const Rect2& b)
	{
		const float depth = 4200.0f;
		for (const auto& lot : SplitLots(b.x0, b.x1, 2600.0f, 3400.0f, 200.0f))
		{
			Villa(Rect2{lot.first, b.y1 - depth, lot.second, b.y1}, Face::PosY);
		}
		for (const auto& lot : SplitLots(b.x0, b.x1, 2600.0f, 3400.0f, 200.0f))
		{
			Villa(Rect2{lot.first, b.y0, lot.second, b.y0 + depth}, Face::NegY);
		}
	}

	void MarketBlock(const Rect2& b, int row)
	{
		const float depth = 4200.0f;
		const auto front = SplitLots(b.x0, b.x1, 1800.0f, 3000.0f, 500.0f);
		const int clothing = row == 5 ? NearestLot(front, 6000.0f) : -1;
		for (size_t i = 0; i < front.size(); ++i)
		{
			const Rect2 lot{front[i].first, b.y1 - depth, front[i].second, b.y1};
			if (static_cast<int>(i) == clothing)
			{
				const Color sign = pal::NeonPink;
				const BuildResult r = Shop(lot, Face::PosY, 3, 3, false, &sign);
				AddPoi(PoiType::ClothingStore, r.door, r.yaw, poi_id::TideAndThread, "Tide & Thread", sign);
			}
			else
			{
				Shop(lot, Face::PosY, 2, 5, false, nullptr);
			}
		}
		for (const auto& lot : SplitLots(b.x0, b.x1, 1800.0f, 3000.0f, 500.0f))
		{
			Shop(Rect2{lot.first, b.y0, lot.second, b.y0 + depth}, Face::NegY, 2, 4, false, nullptr);
		}
	}

	void NeonBlock(const Rect2& b, int row)
	{
		const float depth = 4000.0f;
		for (const auto& lot : SplitLots(b.x0, b.x1, 2000.0f, 2800.0f, 500.0f))
		{
			const Rect2 r{lot.first, b.y1 - depth, lot.second, b.y1};
			if (rng.Chance(0.55f))
			{
				Nightclub(r, Face::PosY);
			}
			else
			{
				Shop(r, Face::PosY, 2, 4, false, nullptr);
			}
		}
		for (const auto& lot : SplitLots(b.x0, b.x1, 2000.0f, 2800.0f, 500.0f))
		{
			const Rect2 r{lot.first, b.y0, lot.second, b.y0 + depth};
			if (rng.Chance(0.55f))
			{
				Nightclub(r, Face::NegY);
			}
			else
			{
				Shop(r, Face::NegY, 2, 4, false, nullptr);
			}
		}
		if (row == 7)
		{
			AddCollectible(Vec3{b.Cx(), b.y0 + depth + 0.5f * (b.Sy() - 2.0f * depth), kWalkTopCm});
		}
	}

	void OfficeBlock(const Rect2& b)
	{
		const float depth = 4200.0f;
		for (const auto& lot : SplitLots(b.x0, b.x1, 3000.0f, 4500.0f, 600.0f))
		{
			Office(Rect2{lot.first, b.y1 - depth, lot.second, b.y1}, Face::PosY);
		}
		for (const auto& lot : SplitLots(b.x0, b.x1, 3000.0f, 4500.0f, 600.0f))
		{
			Office(Rect2{lot.first, b.y0, lot.second, b.y0 + depth}, Face::NegY);
		}
	}

	void BeachfrontBlock(const Rect2& b, int row)
	{
		const float frontDepth = 3900.0f;
		const float backDepth = 3000.0f;
		const auto front = SplitLots(b.x0, b.x1, 2200.0f, 3200.0f, 500.0f);
		const int safehouse = row == 5 ? NearestLot(front, 6000.0f) : -1;
		const int cafe = row == 6 ? NearestLot(front, 17000.0f) : -1;
		for (size_t i = 0; i < front.size(); ++i)
		{
			const Rect2 lot{front[i].first, b.y1 - frontDepth, front[i].second, b.y1};
			if (static_cast<int>(i) == safehouse)
			{
				const BuildResult r = DecoHotel(lot, Face::PosY, true);
				AddPoi(PoiType::Safehouse, r.door, r.yaw, poi_id::HadiApartment, "The Seagrape Apartments", pal::NeonCyan);
				AddPoi(PoiType::PlayerSpawn, r.door + Vec3{250.0f, 200.0f, 0.0f}, 0.0f, 0, "Hadi's front door");
			}
			else if (static_cast<int>(i) == cafe)
			{
				const Color sign = pal::NeonOrange;
				const BuildResult r = Shop(lot, Face::PosY, 2, 3, true, &sign);
				AddPoi(PoiType::Restaurant, r.door, r.yaw, poi_id::CoralCafe, "Coral Cafe", sign);
			}
			else
			{
				DecoHotel(lot, Face::PosY, false);
			}
		}
		for (const auto& lot : SplitLots(b.x0, b.x1, 2000.0f, 3000.0f, 500.0f))
		{
			const Rect2 r{lot.first, b.y0, lot.second, b.y0 + backDepth};
			if (rng.Chance(0.5f))
			{
				Shop(r, Face::NegY, 2, 3, false, nullptr);
			}
			else
			{
				DecoHotel(r, Face::NegY, false);
			}
		}
	}

	void Park(const Rect2& block, bool fountain, const char* name)
	{
		(void)name;
		const Rect2 lawn = block.Inset(300.0f);
		Slab(lawn, 5.0f, kGrassTopCm, Mat::Foliage, pal::Grass, true, CullClass::Far);
		MapR(lawn, mappal::Park, MapLayer::Park);
		const float cx = block.Cx();
		const float cy = block.Cy();
		Slab(Rect2{lawn.x0, cy - 200.0f, lawn.x1, cy + 200.0f}, 6.0f, kPathTopCm, Mat::Matte, pal::Sidewalk, true, CullClass::Medium);
		Slab(Rect2{cx - 200.0f, lawn.y0, cx + 200.0f, lawn.y1}, 6.0f, kPathTopCm, Mat::Matte, pal::Sidewalk, true, CullClass::Medium);
		if (fountain)
		{
			Cyl(Vec3{cx, cy, kPathTopCm + 35.0f}, 1000.0f, 70.0f, Mat::Painted, pal::White, true, CullClass::Medium);
			Cyl(Vec3{cx, cy, kPathTopCm + 68.0f}, 920.0f, 4.0f, Mat::Water, pal::Pool, false, CullClass::Medium);
			Cyl(Vec3{cx, cy, kPathTopCm + 140.0f}, 80.0f, 240.0f, Mat::Painted, pal::White, true, CullClass::Medium);
			Cyl(Vec3{cx, cy, kPathTopCm + 255.0f}, 320.0f, 30.0f, Mat::Painted, pal::White, false, CullClass::Medium);
			Cyl(Vec3{cx, cy, kPathTopCm + 271.0f}, 290.0f, 4.0f, Mat::Water, pal::Pool, false, CullClass::Medium);
			Ball(Vec3{cx, cy, kPathTopCm + 300.0f}, Vec3{60.0f, 60.0f, 60.0f}, Mat::Painted, pal::TrimPink, CullClass::Medium);
		}
		for (int i = 0; i < 4; ++i)
		{
			const float a = static_cast<float>(i) * 90.0f;
			const Basis bb = BasisFromRot(Rot{0.0f, a, 0.0f});
			const Vec2 p = Vec2{cx, cy} + XY(bb.x) * 1600.0f + XY(bb.y) * 320.0f;
			Bench(p, a + 90.0f, kPathTopCm);
			Lamp(Vec2{cx, cy} + XY(bb.x) * 2600.0f + XY(bb.y) * -300.0f, XY(bb.y), kPathTopCm);
		}
		int planted = 0;
		for (int attempt = 0; attempt < 60 && planted < 12; ++attempt)
		{
			const Vec2 p{rng.Range(lawn.x0 + 400.0f, lawn.x1 - 400.0f), rng.Range(lawn.y0 + 400.0f, lawn.y1 - 400.0f)};
			if (Abs(p.x - cx) < 650.0f || Abs(p.y - cy) < 650.0f)
			{
				continue;
			}
			Palm(V3(p, kGrassTopCm), rng.Range(700.0f, 1150.0f));
			++planted;
		}
	}

	void ParkingLot(const Rect2& block)
	{
		const Rect2 lot = block.Inset(300.0f);
		Slab(lot, 5.0f, kGrassTopCm, Mat::Asphalt, pal::AsphaltWorn, true, CullClass::Far);
		const float stallW = 280.0f;
		const float stallD = 550.0f;
		for (int rowSide = 0; rowSide < 2; ++rowSide)
		{
			const float yc = rowSide == 0 ? lot.y0 + 300.0f + 0.5f * stallD : lot.y1 - 300.0f - 0.5f * stallD;
			for (float x = lot.x0 + 600.0f; x <= lot.x1 - 600.0f; x += stallW)
			{
				Box(Vec3{x, yc, kGrassTopCm}, Vec3{12.0f, stallD, 2.0f}, Mat::Matte, pal::LaneWhite, false, CullClass::Near, Rot{}, false);
			}
		}
		Box(Vec3{lot.x1 - 500.0f, lot.Cy(), kGrassTopCm + 140.0f}, Vec3{260.0f, 260.0f, 280.0f}, Mat::Painted, pal::Lemon, true, CullClass::Medium);
		Box(Vec3{lot.x1 - 500.0f, lot.Cy(), kGrassTopCm + 180.0f}, Vec3{264.0f, 264.0f, 80.0f}, Mat::WindowLit, pal::WindowWarm, false, CullClass::Medium, Rot{},
			false);
		Lamp(Vec2{lot.Cx() - 2500.0f, lot.Cy()}, Vec2{1.0f, 0.0f}, kGrassTopCm);
		Lamp(Vec2{lot.Cx() + 2500.0f, lot.Cy()}, Vec2{-1.0f, 0.0f}, kGrassTopCm);
		AddPoi(PoiType::VehicleSpawn, Vec3{lot.x0 + 600.0f + 4.5f * stallW, lot.y0 + 300.0f + 0.5f * stallD, kGrassTopCm}, 90.0f, 1, "Parking lot");
		AddPoi(PoiType::VehicleSpawn, Vec3{lot.x0 + 600.0f + 9.5f * stallW, lot.y0 + 300.0f + 0.5f * stallD, kGrassTopCm}, 90.0f, 2, "Parking lot");
		AddCollectible(Vec3{lot.x0 + 250.0f, lot.y1 - 250.0f, kGrassTopCm});
	}

	void LowWall(float x0, float x1, float y, float baseZ, const std::vector<float>& gapsX, float gapHalf)
	{
		float start = x0;
		std::vector<float> gaps = gapsX;
		std::sort(gaps.begin(), gaps.end());
		gaps.push_back(x1 + gapHalf);
		for (float g : gaps)
		{
			const float end = std::min(g - gapHalf, x1);
			if (end - start > 50.0f)
			{
				Box(Vec3{0.5f * (start + end), y, baseZ + 30.0f}, Vec3{end - start, 40.0f, 60.0f}, Mat::Painted, pal::OffWhite, true, CullClass::Medium);
				Box(Vec3{0.5f * (start + end), y, baseZ + 64.0f}, Vec3{end - start, 50.0f, 8.0f}, Mat::Painted, pal::TrimTeal, false, CullClass::Near);
			}
			start = g + gapHalf;
		}
	}

	void BuildPromenades()
	{
		// East promenade between Seabreeze Boulevard and the beach.
		const Rect2 east = RectM(kIslandX0, kStreetY1, kAvenueX1, kIslandY1);
		Slab(east, -12.0f, kWalkTopCm, Mat::Matte, pal::Sidewalk, true, CullClass::Far);
		MapR(east, mappal::Promenade, MapLayer::Promenade);
		std::vector<float> stairGaps;
		for (float x = -600.0f; x < 480.0f; x += 60.0f)
		{
			stairGaps.push_back(x * kM);
			Slab(Rect2{x * kM - 150.0f, kIslandY1 * kM, x * kM + 150.0f, kIslandY1 * kM + 150.0f}, kSandTopCm - 20.0f, 2.0f, Mat::Matte, pal::Curb, true,
				CullClass::Medium);
		}
		LowWall(kIslandX0 * kM, kAvenueX1 * kM, kIslandY1 * kM - 40.0f, kWalkTopCm, stairGaps, 160.0f);
		for (float x = -624.0f; x < 480.0f; x += 36.0f)
		{
			Lamp(Vec2{x * kM, kStreetY1 * kM + 120.0f}, Vec2{0.0f, -1.0f}, kWalkTopCm);
		}
		for (float x = -615.0f; x < 480.0f; x += 18.0f)
		{
			Palm(Vec3{x * kM, kIslandY1 * kM - 500.0f, kWalkTopCm}, rng.Range(800.0f, 1200.0f));
		}
		for (float x = -590.0f; x < 470.0f; x += 60.0f)
		{
			Bench(Vec2{x * kM + 900.0f, kIslandY1 * kM - 220.0f}, 90.0f, kWalkTopCm);
		}

		// Marina promenade on the bay side.
		const Rect2 west = RectM(kIslandX0, kIslandY0, kAvenueX1, kStreetY0);
		Slab(west, -12.0f, kWalkTopCm, Mat::Matte, pal::Sidewalk, true, CullClass::Far);
		MapR(west, mappal::Promenade, MapLayer::Promenade);
		std::vector<float> pierGaps;
		for (float px : kPierX)
		{
			pierGaps.push_back(px * kM);
		}
		LowWall(kIslandX0 * kM, kAvenueX1 * kM, kIslandY0 * kM + 30.0f, kWalkTopCm, pierGaps, 200.0f);
		for (float x = -624.0f; x < 480.0f; x += 36.0f)
		{
			Lamp(Vec2{x * kM, kStreetY0 * kM - 120.0f}, Vec2{0.0f, 1.0f}, kWalkTopCm);
		}
		for (float x = -612.0f; x < 480.0f; x += 24.0f)
		{
			Palm(Vec3{x * kM, kIslandY0 * kM + 450.0f, kWalkTopCm}, rng.Range(700.0f, 1000.0f));
		}
		for (float x = -560.0f; x < 470.0f; x += 72.0f)
		{
			Bench(Vec2{x * kM + 1200.0f, kIslandY0 * kM + 250.0f}, -90.0f, kWalkTopCm);
		}

		// South Point waterfront strip.
		const Rect2 south = RectM(kIslandX0, kStreetY0, kAvenueX0, kStreetY1);
		Slab(south, -12.0f, kWalkTopCm, Mat::Matte, pal::Sidewalk, true, CullClass::Far);
		MapR(south, mappal::Promenade, MapLayer::Promenade);
		Box(Vec3{kIslandX0 * kM + 30.0f, 0.5f * (kStreetY0 + kStreetY1) * kM, kWalkTopCm + 30.0f}, Vec3{40.0f, (kStreetY1 - kStreetY0) * kM, 60.0f}, Mat::Painted,
			pal::OffWhite, true, CullClass::Medium);
		for (float y = -240.0f; y < 230.0f; y += 30.0f)
		{
			Palm(Vec3{kIslandX0 * kM + 500.0f, y * kM, kWalkTopCm}, rng.Range(800.0f, 1100.0f));
		}

		// North Shore Park.
		const Rect2 north = RectM(kAvenueX1, kIslandY0, kIslandX1, kIslandY1);
		Slab(north, -12.0f, kWalkTopCm, Mat::Matte, pal::Sidewalk, true, CullClass::Far);
		const Rect2 northLawn = north.Inset(200.0f);
		Slab(northLawn, 5.0f, kGrassTopCm, Mat::Foliage, pal::Grass, true, CullClass::Far);
		MapR(northLawn, mappal::Park, MapLayer::Park);
		const float pathX = 0.5f * (kAvenueX1 + kIslandX1) * kM;
		Slab(Rect2{pathX - 200.0f, northLawn.y0, pathX + 200.0f, northLawn.y1}, 6.0f, kPathTopCm, Mat::Matte, pal::Sidewalk, true, CullClass::Medium);
		Box(Vec3{kIslandX1 * kM - 30.0f, 0.5f * (kIslandY0 + kIslandY1) * kM, kWalkTopCm + 30.0f}, Vec3{40.0f, (kIslandY1 - kIslandY0) * kM, 60.0f}, Mat::Painted,
			pal::OffWhite, true, CullClass::Medium);
		for (float y = -260.0f; y < 240.0f; y += 40.0f)
		{
			Lamp(Vec2{pathX - 350.0f, y * kM}, Vec2{1.0f, 0.0f}, kGrassTopCm);
		}
		for (int i = 0; i < 18; ++i)
		{
			const Vec2 p{rng.Range(northLawn.x0 + 300.0f, northLawn.x1 - 300.0f), rng.Range(northLawn.y0 + 300.0f, northLawn.y1 - 300.0f)};
			if (Abs(p.x - pathX) < 500.0f)
			{
				continue;
			}
			Palm(V3(p, kGrassTopCm), rng.Range(700.0f, 1100.0f));
		}
	}

	static constexpr float kPierX[5] = {-400.0f, -340.0f, -280.0f, -220.0f, -160.0f};

	void BuildMarina()
	{
		const float pierLen = 60.0f;
		for (float px : kPierX)
		{
			const Rect2 deck = RectM(px - 1.75f, kIslandY0 - pierLen, px + 1.75f, kIslandY0 + 0.3f);
			Slab(deck, kPierTopCm - 30.0f, kPierTopCm, Mat::Matte, pal::Wood, true, CullClass::Medium);
			MapR(deck, mappal::Pier, MapLayer::Pier);
			for (float y = kIslandY0 - 3.0f; y > kIslandY0 - pierLen; y -= 6.0f)
			{
				for (int side = -1; side <= 1; side += 2)
				{
					Cyl(Vec3{(px + static_cast<float>(side) * 1.6f) * kM, y * kM, 0.5f * (-400.0f + kPierTopCm + 50.0f)}, 30.0f, kPierTopCm + 50.0f + 400.0f,
						Mat::Matte, pal::WoodDark, true, CullClass::Medium);
				}
			}
			Lamp(Vec2{(px + 1.4f) * kM, (kIslandY0 - pierLen + 2.0f) * kM}, Vec2{-1.0f, 0.0f}, kPierTopCm);
		}
		// Moored yachts between the piers, bows pointing out into the bay.
		const float yachtX[4] = {-370.0f, -310.0f, -250.0f, -190.0f};
		for (float yx : yachtX)
		{
			const float length = rng.Range(1400.0f, 2300.0f);
			const float beam = rng.Range(450.0f, 600.0f);
			Yacht(Vec2{yx * kM, (kIslandY0 - 10.0f) * kM - 0.5f * length}, -90.0f, length, beam);
		}
	}

	void BuildBeach()
	{
		const float towersX[4] = {-420.0f, -170.0f, 80.0f, 330.0f};
		const Color cabins[4] = {pal::Pink, pal::Aqua, pal::Lemon, pal::Lavender};
		for (int i = 0; i < 4; ++i)
		{
			const Vec3 deck = LifeguardTower(Vec2{towersX[i] * kM, 274.0f * kM}, cabins[i]);
			if (i == 1)
			{
				AddCollectible(deck);
			}
		}
		// Volleyball court.
		const float vx = 205.0f * kM;
		const float vy = 280.0f * kM;
		for (int side = -1; side <= 1; side += 2)
		{
			Cyl(Vec3{vx + static_cast<float>(side) * 500.0f, vy, kSandTopCm + 125.0f}, 12.0f, 250.0f, Mat::Metal, pal::Metal, true, CullClass::Near);
		}
		Box(Vec3{vx, vy, kSandTopCm + 205.0f}, Vec3{1000.0f, 3.0f, 90.0f}, Mat::Matte, pal::White, false, CullClass::Near);
		// Umbrella clusters.
		static const Color canopies[] = {pal::Coral, pal::TrimTeal, pal::Lemon, pal::White, pal::TrimPink, pal::Sky};
		int placed = 0;
		for (int attempt = 0; attempt < 200 && placed < 42; ++attempt)
		{
			const Vec2 p{rng.Range(-625.0f, 525.0f) * kM, rng.Range(256.0f, 296.0f) * kM};
			bool blocked = false;
			for (float tx : towersX)
			{
				if (Abs(p.x - tx * kM) < 1400.0f)
				{
					blocked = true;
				}
			}
			if (Abs(p.x - vx) < 1200.0f)
			{
				blocked = true;
			}
			for (const Vec3& c : collectiblePositions)
			{
				if (Distance(p, XY(c)) < 600.0f)
				{
					blocked = true;
				}
			}
			if (blocked)
			{
				continue;
			}
			Umbrella(p, canopies[rng.RangeInt(0, 5)]);
			++placed;
		}
		for (float x = -630.0f; x < 530.0f; x += 32.0f)
		{
			Palm(Vec3{x * kM, 254.0f * kM, kSandTopCm}, rng.Range(700.0f, 1100.0f));
		}
	}

	void BuildSkyline()
	{
		const Rect2 mainland = RectM(-1500.0f, -2700.0f, 1500.0f, -1800.0f);
		Slab(mainland, kDeepCm, 0.0f, Mat::Matte, pal::Concrete, false, CullClass::Never, false);
		MapR(mainland, mappal::Road, MapLayer::Land);
		Rng sky(0x5EA5CA9Eull);
		for (int i = 0; i < 26; ++i)
		{
			const float w = sky.Range(2400.0f, 4800.0f);
			const float d = sky.Range(2400.0f, 4800.0f);
			const float h = sky.Range(6000.0f, 26000.0f) * (1.0f - Abs(static_cast<float>(i) - 13.0f) / 30.0f);
			const Vec2 c{sky.Range(-1300.0f, 1300.0f) * kM, sky.Range(-2550.0f, -1900.0f) * kM};
			const Rect2 r{c.x - 0.5f * w, c.y - 0.5f * d, c.x + 0.5f * w, c.y + 0.5f * d};
			const Color body = sky.Chance(0.6f) ? (sky.Chance(0.5f) ? pal::GlassBlue : pal::GlassTeal) : Rgb(200, 204, 212);
			Slab(r, 0.0f, h, sky.Chance(0.6f) ? Mat::Glass : Mat::Painted, body, false, CullClass::Never, false);
			MapR(r, mappal::BuildingTall, MapLayer::Building);
			// Lit floor bands on the faces that look at the island (north/east).
			for (float z = 1200.0f; z < h - 300.0f; z += 1080.0f)
			{
				const Color lit = sky.Chance(0.7f) ? pal::WindowWarm : pal::WindowCool;
				if (sky.Chance(0.55f))
				{
					Box(Vec3{c.x, r.y1 + 6.0f, z}, Vec3{w - 200.0f, 8.0f, 160.0f}, Mat::WindowLit, lit, false, CullClass::Never, Rot{}, false);
				}
				if (sky.Chance(0.45f))
				{
					Box(Vec3{r.x1 + 6.0f, c.y, z}, Vec3{8.0f, d - 200.0f, 160.0f}, Mat::WindowLit, lit, false, CullClass::Never, Rot{}, false);
				}
				if (sky.Chance(0.45f))
				{
					Box(Vec3{r.x0 - 6.0f, c.y, z}, Vec3{8.0f, d - 200.0f, 160.0f}, Mat::WindowLit, lit, false, CullClass::Never, Rot{}, false);
				}
			}
			if (h > 15000.0f)
			{
				Ball(Vec3{c.x, c.y, h + 300.0f}, Vec3{120.0f, 120.0f, 120.0f}, Mat::Light, pal::BeaconRed, CullClass::Never);
				Cyl(Vec3{c.x, c.y, h + 150.0f}, 40.0f, 300.0f, Mat::Metal, pal::Metal, false, CullClass::Never);
			}
		}
	}

	std::vector<Vec3> collectiblePositions;

	void AddCollectible(Vec3 p)
	{
		collectiblePositions.push_back(p);
	}

	void PlacePois()
	{
		// Vehicles: the prologue car outside the Coral Cafe plus parked cars around the island.
		const Poi* cafe = model.FindPoi(PoiType::Restaurant, poi_id::CoralCafe);
		const float cafeX = cafe ? cafe->pos.x : 17000.0f;
		AddPoi(PoiType::VehicleSpawn, Vec3{cafeX + 900.0f, 224.6f * kM, kRoadTopCm}, 180.0f, poi_id::StarterCar, "Vento GT (Hadi's ride)");
		AddPoi(PoiType::VehicleSpawn, Vec3{-300.0f * kM, -245.5f * kM, kRoadTopCm}, 0.0f, 3, "Bayview Drive");
		AddPoi(PoiType::VehicleSpawn, Vec3{110.0f * kM, -5.5f * kM, kRoadTopCm}, 180.0f, 4, "Flamingo Avenue");

		AddPoi(PoiType::MarinaMeet, Vec3{-250.0f * kM, -266.0f * kM, kWalkTopCm}, -90.0f, 0, "Marina Row - Pier 3");

		int id = 0;
		for (const Vec3& c : collectiblePositions)
		{
			AddPoi(PoiType::Collectible, c, 0.0f, id++, "Neon Flamingo", pal::NeonPink);
		}
	}

	// Neon Flamingo collectibles at fixed, hand-picked spots (others are placed by districts).
	void AddFixedCollectibles()
	{
		AddCollectible(Vec3{420.0f * kM, 286.0f * kM, kSandTopCm});
		AddCollectible(Vec3{-280.0f * kM, -332.0f * kM, kPierTopCm});
		AddCollectible(Vec3{513.0f * kM, -100.0f * kM, kPathTopCm});
		AddCollectible(Vec3{0.0f, (kCausewayEndY + 12.0f) * kM, kRoadTopCm});
		AddCollectible(Vec3{-120.0f * kM, -267.0f * kM, kWalkTopCm});
		AddCollectible(Vec3{-628.0f * kM, 10.0f * kM, kWalkTopCm});
		AddCollectible(Vec3{478.0f * kM, 244.0f * kM, kWalkTopCm});
	}

	void BuildDistricts()
	{
		auto add = [this](const char* name, float x0, float y0, float x1, float y1) {
			District d;
			d.name = name;
			d.min = Vec2{x0 * kM, y0 * kM};
			d.max = Vec2{x1 * kM, y1 * kM};
			model.districts.push_back(d);
		};
		add("Flamingo Park", -114.0f, -117.0f, -6.0f, -8.0f);
		add("Neon Mile Parking", -234.0f, 8.0f, -126.0f, 117.0f);
		add("South Point Park", -604.0f, 133.0f, -486.0f, 250.0f);
		add("South Point", -640.0f, -275.0f, -486.0f, 250.0f);
		add("North Shore Park", 486.0f, -275.0f, 540.0f, 250.0f);
		add("Vice Causeway", -20.0f, -1300.0f, 20.0f, -275.0f);
		add("Marina Row", -640.0f, -400.0f, 486.0f, -242.0f);
		add("Bayshore Estates", -640.0f, -242.0f, 486.0f, -125.0f);
		add("Palm Market", -640.0f, -125.0f, 486.0f, 0.0f);
		add("Neon Mile", 6.0f, 0.0f, 486.0f, 125.0f);
		add("Coral Key Center", -640.0f, 0.0f, 6.0f, 125.0f);
		add("Coral Bay", -640.0f, 125.0f, 486.0f, 250.0f);
		add("Coral Bay Beach", -700.0f, 250.0f, 600.0f, 700.0f);
		add("Downtown Vice City", -1600.0f, -2800.0f, 1600.0f, -1750.0f);
		add("Lantana Bay", -3000.0f, -1750.0f, 3000.0f, -275.0f);
	}

	void AssignCells()
	{
		model.cellSize = 20000.0f;
		model.cellOrigin = Vec2{-160000.0f, -280000.0f};
		model.cellsX = 16;
		model.cellsY = 18;
		for (Prim& p : model.prims)
		{
			const float extent = Max(p.size.x, p.size.y);
			if (p.cull == CullClass::Never || extent > 15000.0f)
			{
				p.cell = -1;
			}
			else
			{
				p.cell = model.CellIndexAt(XY(p.center));
			}
		}
	}

};

void Builder::Run(const CityParams& params)
{
	model.seaLevel = -60.0f;
	model.islandMin = Vec2{kIslandX0 * kM, kIslandY0 * kM};
	model.islandMax = Vec2{kIslandX1 * kM, kIslandY1 * kM};
	model.worldMin = Vec2{-1500.0f * kM, -2700.0f * kM};
	model.worldMax = Vec2{1500.0f * kM, 600.0f * kM};

	BuildTerrain();
	BuildRoads();
	BuildCauseway();
	BuildBlocks();
	BuildPromenades();
	BuildMarina();
	AddFixedCollectibles();
	BuildBeach();
	if (params.includeSkyline)
	{
		BuildSkyline();
	}
	PlacePois();
	BuildDistricts();
	AssignCells();
}
} // namespace citygen

// ------------------------------------------------------------------------------------------
int CityModel::CellIndexAt(Vec2 p) const
{
	const int ix = static_cast<int>(std::floor((p.x - cellOrigin.x) / cellSize));
	const int iy = static_cast<int>(std::floor((p.y - cellOrigin.y) / cellSize));
	if (ix < 0 || iy < 0 || ix >= cellsX || iy >= cellsY)
	{
		return -1;
	}
	return iy * cellsX + ix;
}

Vec2 CityModel::CellMin(int cell) const
{
	const int ix = cell % cellsX;
	const int iy = cell / cellsX;
	return Vec2{cellOrigin.x + static_cast<float>(ix) * cellSize, cellOrigin.y + static_cast<float>(iy) * cellSize};
}

Vec2 CityModel::CellMax(int cell) const
{
	return CellMin(cell) + Vec2{cellSize, cellSize};
}

const Poi* CityModel::FindPoi(PoiType type, int id) const
{
	for (const Poi& p : pois)
	{
		if (p.type == type && (id < 0 || p.id == id))
		{
			return &p;
		}
	}
	return nullptr;
}

int CityModel::CountPois(PoiType type) const
{
	int count = 0;
	for (const Poi& p : pois)
	{
		if (p.type == type)
		{
			++count;
		}
	}
	return count;
}

const char* CityModel::DistrictAt(Vec2 p) const
{
	for (const District& d : districts)
	{
		if (p.x >= d.min.x && p.x <= d.max.x && p.y >= d.min.y && p.y <= d.max.y)
		{
			return d.name;
		}
	}
	return "Vice City Waters";
}

const char* CityModel::StreetAt(Vec2 p, float maxDistance) const
{
	const char* best = "";
	float bestDist = maxDistance;
	for (const RoadSeg& r : roads)
	{
		const Vec2 ab = r.b - r.a;
		const float lenSq = Dot(ab, ab);
		const float t = lenSq > 0.0f ? Saturate(Dot(p - r.a, ab) / lenSq) : 0.0f;
		const float d = Distance(p, r.a + ab * t);
		if (d < bestDist)
		{
			bestDist = d;
			best = r.name;
		}
	}
	return best;
}

bool CityModel::InsideBuilding(Vec2 p, float margin) const
{
	for (const Footprint& f : footprints)
	{
		if (p.x >= f.x0 - margin && p.x <= f.x1 + margin && p.y >= f.y0 - margin && p.y <= f.y1 + margin)
		{
			return true;
		}
	}
	return false;
}

CityModel GenerateCity(const CityParams& params)
{
	CityModel model;
	citygen::Builder builder(model, params.seed);
	builder.Run(params);
	return model;
}
} // namespace hadi
