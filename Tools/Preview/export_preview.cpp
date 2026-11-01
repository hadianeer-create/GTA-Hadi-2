// HADI: VICE CITY - exports the simulation core's city + Hadi rig poses to preview_data.js so the
// browser previewer (Tools/Preview/index.html) can render exactly what the engine will spawn.
#include "Core/HadiCity.h"
#include "Core/HadiRig.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace hadi;

namespace
{
void WritePrim(FILE* f, int shape, int mat, Color c, Vec3 center, Vec3 size, const Basis& b, bool first)
{
	std::fprintf(f, "%s[%d,%d,%.3f,%.3f,%.3f,%.1f,%.1f,%.1f,%.2f,%.2f,%.2f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f]", first ? "" : ",", shape, mat, c.r, c.g,
		c.b, center.x, center.y, center.z, size.x, size.y, size.z, b.x.x, b.x.y, b.x.z, b.y.x, b.y.y, b.y.z, b.z.x, b.z.y, b.z.z);
}

struct PoseSpec
{
	const char* name;
	AnimInput input;
	int frames;
	int outfit;
};

void WriteRig(FILE* f, const PoseSpec& spec, bool firstPose)
{
	AnimState state;
	for (int i = 0; i < spec.frames; ++i)
	{
		UpdateAnimState(state, spec.input, 1.0f / 60.0f);
	}
	const Pose pose = ComputePose(state);
	JointWorld joints[kJointCount];
	SolveJoints(pose, joints);
	const Outfit& outfit = GetOutfit(spec.outfit);
	std::fprintf(f, "%s{\"name\":\"%s\",\"prims\":[", firstPose ? "" : ",", spec.name);
	bool first = true;
	for (const RigSegment& seg : GetRigSegments())
	{
		if (!IsSegmentVisible(seg.vis, outfit))
		{
			continue;
		}
		const SegmentTransform t = ApplyFace(seg, pose.face);
		const JointWorld& jw = joints[static_cast<int>(seg.joint)];
		const Vec3 center = jw.pos + Apply(jw.basis, t.center);
		const Basis b = Compose(jw.basis, BasisFromRot(t.rot));
		WritePrim(f, static_cast<int>(seg.shape), static_cast<int>(seg.mat), ResolveSlotColor(seg.slot, outfit), center, t.size, b, first);
		first = false;
	}
	std::fprintf(f, "]}");
}
} // namespace

int main(int argc, char** argv)
{
	const char* outPath = argc > 1 ? argv[1] : "preview_data.js";
	FILE* f = std::fopen(outPath, "w");
	if (!f)
	{
		std::perror("open");
		return 1;
	}
	const CityModel city = GenerateCity(CityParams{});
	std::fprintf(f, "window.HADI_DATA={\"seaLevel\":%.1f,\"prims\":[", city.seaLevel);
	bool first = true;
	for (const Prim& p : city.prims)
	{
		WritePrim(f, static_cast<int>(p.shape), static_cast<int>(p.mat), p.color, p.center, p.size, BasisFromRot(p.rot), first);
		first = false;
	}
	std::fprintf(f, "],\"pois\":[");
	first = true;
	for (const Poi& p : city.pois)
	{
		if (p.type == PoiType::StreetLight || p.type == PoiType::NeonLight)
		{
			continue;
		}
		std::fprintf(f, "%s[%d,%d,%.1f,%.1f,%.1f,%.1f]", first ? "" : ",", static_cast<int>(p.type), p.id, p.pos.x, p.pos.y, p.pos.z, p.yaw);
		first = false;
	}
	std::fprintf(f, "],\"map\":[");
	first = true;
	for (const MapRect& m : city.map)
	{
		std::fprintf(f, "%s[%d,%.1f,%.1f,%.1f,%.1f,%.3f,%.3f,%.3f]", first ? "" : ",", static_cast<int>(m.layer), m.center.x, m.center.y, m.half.x, m.half.y,
			m.color.r, m.color.g, m.color.b);
		first = false;
	}
	std::fprintf(f, "],\"route\":[");
	std::vector<Vec2> route;
	const Poi* car = city.FindPoi(PoiType::VehicleSpawn, poi_id::StarterCar);
	const Poi* marina = city.FindPoi(PoiType::MarinaMeet);
	if (car && marina && city.graph.BuildRoute(XY(car->pos), XY(marina->pos), route))
	{
		for (size_t i = 0; i < route.size(); ++i)
		{
			std::fprintf(f, "%s[%.1f,%.1f]", i == 0 ? "" : ",", route[i].x, route[i].y);
		}
	}
	std::fprintf(f, "],\"rig\":[");

	std::vector<PoseSpec> poses;
	auto add = [&poses](const char* name, int outfit, int frames, AnimInput in) { poses.push_back(PoseSpec{name, in, frames, outfit}); };
	AnimInput idle;
	add("Idle - Vice Casual", 0, 120, idle);
	AnimInput walk;
	walk.speed = 170.0f;
	add("Walk", 0, 47, walk);
	AnimInput run;
	run.speed = 420.0f;
	add("Run", 3, 31, run);
	AnimInput sprint;
	sprint.speed = 680.0f;
	add("Sprint", 3, 23, sprint);
	AnimInput jump;
	jump.grounded = false;
	jump.verticalSpeed = 350.0f;
	jump.speed = 300.0f;
	add("Jump", 2, 40, jump);
	AnimInput swim;
	swim.swimming = true;
	swim.speed = 220.0f;
	add("Swim (crawl)", 2, 100, swim);
	AnimInput drive;
	drive.driving = true;
	drive.steer = 0.5f;
	add("Driving", 1, 90, drive);
	AnimInput climb;
	climb.climbing = true;
	climb.climbAlpha = 0.6f;
	add("Climb / mantle", 5, 40, climb);
	AnimInput happy;
	happy.expression = Expression::Happy;
	add("Happy - Executive", 4, 90, happy);
	AnimInput angry;
	angry.expression = Expression::Angry;
	add("Angry - Night Out", 1, 90, angry);
	AnimInput surprised;
	surprised.expression = Expression::Surprised;
	add("Surprised - Marina Crew", 5, 90, surprised);
	for (size_t i = 0; i < poses.size(); ++i)
	{
		WriteRig(f, poses[i], i == 0);
	}
	std::fprintf(f, "]};\n");
	std::fclose(f);
	std::printf("wrote %s (%zu prims, %zu poses)\n", outPath, city.prims.size(), poses.size());
	return 0;
}
