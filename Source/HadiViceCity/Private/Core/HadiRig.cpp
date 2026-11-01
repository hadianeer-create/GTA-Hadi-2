// HADI: VICE CITY - Simulation Core
#include "Core/HadiRig.h"

#include <cmath>

namespace hadi
{
namespace rigdata
{
// Hadi's fixed features.
constexpr Color kSkin = Rgb(176, 120, 86);
constexpr Color kHair = Rgb(30, 24, 22);
constexpr Color kBeard = Rgb(64, 48, 40);
constexpr Color kLip = Rgb(146, 84, 72);
constexpr Color kEyeWhite = Rgb(236, 232, 226);
constexpr Color kIris = Rgb(62, 38, 22);
constexpr Color kSole = Rgb(64, 60, 58);
constexpr Color kLens = Rgb(18, 22, 30);
constexpr Color kFrame = Rgb(28, 28, 30);
constexpr Color kWatch = Rgb(206, 182, 120);

const RigJoint kJoints[kJointCount] = {
	{Joint::Root, Vec3{0.0f, 0.0f, 0.0f}, "Root"},
	{Joint::Root, Vec3{0.0f, 0.0f, 96.0f}, "Pelvis"},
	{Joint::Pelvis, Vec3{0.0f, 0.0f, 8.0f}, "Spine"},
	{Joint::Spine, Vec3{0.0f, 0.0f, 20.0f}, "Chest"},
	{Joint::Chest, Vec3{0.0f, 0.0f, 26.0f}, "Neck"},
	{Joint::Neck, Vec3{0.0f, 0.0f, 8.0f}, "Head"},
	{Joint::Chest, Vec3{0.0f, -20.5f, 19.0f}, "ShoulderL"},
	{Joint::ShoulderL, Vec3{0.0f, 0.0f, -28.0f}, "ElbowL"},
	{Joint::ElbowL, Vec3{0.0f, 0.0f, -25.0f}, "WristL"},
	{Joint::Chest, Vec3{0.0f, 20.5f, 19.0f}, "ShoulderR"},
	{Joint::ShoulderR, Vec3{0.0f, 0.0f, -28.0f}, "ElbowR"},
	{Joint::ElbowR, Vec3{0.0f, 0.0f, -25.0f}, "WristR"},
	{Joint::Pelvis, Vec3{0.0f, -10.0f, -5.0f}, "HipL"},
	{Joint::HipL, Vec3{0.0f, 0.0f, -43.0f}, "KneeL"},
	{Joint::KneeL, Vec3{0.0f, 0.0f, -41.0f}, "AnkleL"},
	{Joint::Pelvis, Vec3{0.0f, 10.0f, -5.0f}, "HipR"},
	{Joint::HipR, Vec3{0.0f, 0.0f, -43.0f}, "KneeR"},
	{Joint::KneeR, Vec3{0.0f, 0.0f, -41.0f}, "AnkleR"},
};

RigSegment Seg(Joint j, Shape shape, Vec3 c, Vec3 s, Slot slot, Visibility vis = Visibility::Always, FacePart face = FacePart::None, Rot r = Rot{})
{
	RigSegment seg;
	seg.joint = j;
	seg.shape = shape;
	seg.center = c;
	seg.size = s;
	seg.rot = r;
	seg.slot = slot;
	seg.vis = vis;
	seg.face = face;
	seg.mat = ResolveSlotMaterial(slot, Mat::Matte);
	return seg;
}

std::vector<RigSegment> BuildSegments()
{
	std::vector<RigSegment> s;
	// Torso.
	s.push_back(Seg(Joint::Pelvis, Shape::Cube, Vec3{0.0f, 0.0f, -2.0f}, Vec3{20.0f, 30.0f, 18.0f}, Slot::Pants));
	s.push_back(Seg(Joint::Pelvis, Shape::Cube, Vec3{0.0f, 0.0f, 7.0f}, Vec3{21.0f, 31.0f, 4.0f}, Slot::Sole));
	s.push_back(Seg(Joint::Spine, Shape::Cube, Vec3{0.0f, 0.0f, 10.0f}, Vec3{19.0f, 28.0f, 22.0f}, Slot::Top));
	s.push_back(Seg(Joint::Chest, Shape::Cube, Vec3{1.0f, 0.0f, 12.5f}, Vec3{23.0f, 37.0f, 27.0f}, Slot::Top));
	s.push_back(Seg(Joint::Chest, Shape::Sphere, Vec3{0.0f, -18.5f, 19.0f}, Vec3{14.0f, 14.0f, 14.0f}, Slot::Top));
	s.push_back(Seg(Joint::Chest, Shape::Sphere, Vec3{0.0f, 18.5f, 19.0f}, Vec3{14.0f, 14.0f, 14.0f}, Slot::Top));
	s.push_back(Seg(Joint::Chest, Shape::Cube, Vec3{12.6f, 0.0f, 11.0f}, Vec3{1.0f, 10.0f, 25.0f}, Slot::Shirt, Visibility::JacketOnly));
	s.push_back(Seg(Joint::Chest, Shape::Cube, Vec3{2.0f, 0.0f, 26.0f}, Vec3{17.0f, 18.0f, 3.0f}, Slot::ShirtAccent));
	s.push_back(Seg(Joint::Chest, Shape::Cube, Vec3{12.4f, -7.0f, 15.0f}, Vec3{0.8f, 6.0f, 6.0f}, Slot::ShirtAccent));
	// Neck & head.
	s.push_back(Seg(Joint::Neck, Shape::Cylinder, Vec3{0.0f, 0.0f, 4.0f}, Vec3{11.0f, 11.0f, 10.0f}, Slot::Skin));
	s.push_back(Seg(Joint::Head, Shape::Sphere, Vec3{1.0f, 0.0f, 11.0f}, Vec3{21.0f, 19.0f, 24.0f}, Slot::Skin));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{2.5f, 0.0f, 4.0f}, Vec3{16.0f, 15.0f, 10.0f}, Slot::Skin));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{3.5f, 0.0f, 1.2f}, Vec3{15.2f, 15.4f, 3.8f}, Slot::Beard));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{10.8f, 0.0f, 6.9f}, Vec3{1.0f, 4.6f, 0.8f}, Slot::Beard));
	s.push_back(Seg(Joint::Head, Shape::Sphere, Vec3{-0.5f, 0.0f, 16.5f}, Vec3{22.0f, 20.4f, 14.0f}, Slot::Hair, Visibility::NoCap));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{-6.0f, 0.0f, 12.0f}, Vec3{10.0f, 19.0f, 14.0f}, Slot::Hair));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{11.6f, 0.0f, 9.0f}, Vec3{4.0f, 3.0f, 5.0f}, Slot::Skin));
	s.push_back(Seg(Joint::Head, Shape::Sphere, Vec3{0.0f, -9.8f, 10.0f}, Vec3{4.0f, 2.0f, 6.0f}, Slot::Skin));
	s.push_back(Seg(Joint::Head, Shape::Sphere, Vec3{0.0f, 9.8f, 10.0f}, Vec3{4.0f, 2.0f, 6.0f}, Slot::Skin));
	// Eyes, brows, mouth (animated by FaceState).
	s.push_back(Seg(Joint::Head, Shape::Sphere, Vec3{10.6f, -3.6f, 12.0f}, Vec3{2.6f, 3.2f, 2.2f}, Slot::EyeWhite, Visibility::NoSunglasses, FacePart::EyeL));
	s.push_back(Seg(Joint::Head, Shape::Sphere, Vec3{10.6f, 3.6f, 12.0f}, Vec3{2.6f, 3.2f, 2.2f}, Slot::EyeWhite, Visibility::NoSunglasses, FacePart::EyeR));
	s.push_back(Seg(Joint::Head, Shape::Sphere, Vec3{11.5f, -3.6f, 12.0f}, Vec3{1.0f, 1.5f, 1.5f}, Slot::Iris, Visibility::NoSunglasses, FacePart::EyeL));
	s.push_back(Seg(Joint::Head, Shape::Sphere, Vec3{11.5f, 3.6f, 12.0f}, Vec3{1.0f, 1.5f, 1.5f}, Slot::Iris, Visibility::NoSunglasses, FacePart::EyeR));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{10.8f, -3.8f, 15.2f}, Vec3{1.5f, 5.0f, 1.2f}, Slot::Brow, Visibility::Always, FacePart::BrowL));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{10.8f, 3.8f, 15.2f}, Vec3{1.5f, 5.0f, 1.2f}, Slot::Brow, Visibility::Always, FacePart::BrowR));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{10.7f, 0.0f, 5.5f}, Vec3{1.2f, 3.2f, 0.9f}, Slot::Lip, Visibility::Always, FacePart::MouthCenter));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{10.5f, -2.3f, 5.6f}, Vec3{1.2f, 1.8f, 0.8f}, Slot::Lip, Visibility::Always, FacePart::MouthCornerL));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{10.5f, 2.3f, 5.6f}, Vec3{1.2f, 1.8f, 0.8f}, Slot::Lip, Visibility::Always, FacePart::MouthCornerR));
	// Accessories.
	s.push_back(Seg(Joint::Head, Shape::Sphere, Vec3{0.0f, 0.0f, 17.0f}, Vec3{23.0f, 21.0f, 13.0f}, Slot::Cap, Visibility::CapOnly));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{12.0f, 0.0f, 17.5f}, Vec3{11.0f, 16.0f, 1.5f}, Slot::Cap, Visibility::CapOnly, FacePart::None, Rot{-8.0f, 0.0f, 0.0f}));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{11.3f, -3.7f, 12.2f}, Vec3{0.8f, 5.0f, 2.8f}, Slot::Lens, Visibility::SunglassesOnly));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{11.3f, 3.7f, 12.2f}, Vec3{0.8f, 5.0f, 2.8f}, Slot::Lens, Visibility::SunglassesOnly));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{11.5f, 0.0f, 12.8f}, Vec3{0.6f, 2.6f, 0.6f}, Slot::Frame, Visibility::SunglassesOnly));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{4.5f, -9.9f, 12.8f}, Vec3{13.0f, 0.6f, 0.6f}, Slot::Frame, Visibility::SunglassesOnly));
	s.push_back(Seg(Joint::Head, Shape::Cube, Vec3{4.5f, 9.9f, 12.8f}, Vec3{13.0f, 0.6f, 0.6f}, Slot::Frame, Visibility::SunglassesOnly));
	// Arms.
	const Joint shoulders[2] = {Joint::ShoulderL, Joint::ShoulderR};
	const Joint elbows[2] = {Joint::ElbowL, Joint::ElbowR};
	const Joint wrists[2] = {Joint::WristL, Joint::WristR};
	for (int i = 0; i < 2; ++i)
	{
		s.push_back(Seg(shoulders[i], Shape::Cylinder, Vec3{0.0f, 0.0f, -8.0f}, Vec3{11.5f, 11.5f, 17.0f}, Slot::Top));
		s.push_back(Seg(shoulders[i], Shape::Cylinder, Vec3{0.0f, 0.0f, -18.0f}, Vec3{9.5f, 9.5f, 20.0f}, Slot::Forearm));
		s.push_back(Seg(elbows[i], Shape::Sphere, Vec3{0.0f, 0.0f, 0.0f}, Vec3{9.6f, 9.6f, 9.6f}, Slot::Forearm));
		s.push_back(Seg(elbows[i], Shape::Cylinder, Vec3{0.0f, 0.0f, -12.0f}, Vec3{8.8f, 8.8f, 24.0f}, Slot::Forearm));
		s.push_back(Seg(wrists[i], Shape::Cube, Vec3{0.5f, 0.0f, -5.0f}, Vec3{6.0f, 4.5f, 10.0f}, Slot::Skin));
		s.push_back(Seg(wrists[i], Shape::Cube, Vec3{3.5f, 0.0f, -2.5f}, Vec3{2.5f, 2.5f, 6.0f}, Slot::Skin, Visibility::Always, FacePart::None, Rot{-25.0f, 0.0f, 0.0f}));
	}
	s.push_back(Seg(Joint::ElbowL, Shape::Cylinder, Vec3{0.0f, 0.0f, -22.0f}, Vec3{9.8f, 9.8f, 2.6f}, Slot::Watch));
	// Legs.
	const Joint hips[2] = {Joint::HipL, Joint::HipR};
	const Joint knees[2] = {Joint::KneeL, Joint::KneeR};
	const Joint ankles[2] = {Joint::AnkleL, Joint::AnkleR};
	for (int i = 0; i < 2; ++i)
	{
		s.push_back(Seg(hips[i], Shape::Cylinder, Vec3{0.0f, 0.0f, -21.0f}, Vec3{15.0f, 15.0f, 44.0f}, Slot::Pants));
		s.push_back(Seg(knees[i], Shape::Sphere, Vec3{0.0f, 0.0f, 0.0f}, Vec3{13.0f, 13.0f, 13.0f}, Slot::Shin));
		s.push_back(Seg(knees[i], Shape::Cylinder, Vec3{0.0f, 0.0f, -20.0f}, Vec3{12.0f, 12.0f, 40.0f}, Slot::Shin));
		s.push_back(Seg(ankles[i], Shape::Cube, Vec3{5.0f, 0.0f, -3.5f}, Vec3{25.0f, 10.5f, 7.0f}, Slot::Shoes));
		s.push_back(Seg(ankles[i], Shape::Cube, Vec3{5.0f, 0.0f, -6.5f}, Vec3{26.0f, 11.0f, 1.2f}, Slot::Sole));
	}
	return s;
}

const Outfit kOutfits[] = {
	// id, name, price, shirt, accent, pants, shoes, jacket, cap, jacket?, longSleeves, shorts, cap?, sunglasses
	{"vice_casual", "Vice Casual", 0, Rgb(38, 156, 164), Rgb(240, 236, 226), Rgb(206, 186, 150), Rgb(236, 236, 230), Rgb(0, 0, 0), Rgb(0, 0, 0), false, false,
		false, false, true},
	{"night_out", "Night Out", 250, Rgb(26, 26, 30), Rgb(150, 30, 60), Rgb(30, 30, 34), Rgb(26, 22, 22), Rgb(122, 26, 52), Rgb(0, 0, 0), true, true, false,
		false, false},
	{"beach_day", "Beach Day", 120, Rgb(244, 120, 96), Rgb(250, 240, 210), Rgb(40, 60, 110), Rgb(176, 128, 80), Rgb(0, 0, 0), Rgb(240, 240, 236), false, false,
		true, true, true},
	{"street_runner", "Street Runner", 180, Rgb(236, 236, 240), Rgb(220, 40, 60), Rgb(30, 36, 70), Rgb(210, 40, 50), Rgb(34, 44, 90), Rgb(0, 0, 0), true, true,
		false, false, false},
	{"executive", "Executive", 400, Rgb(240, 240, 244), Rgb(40, 60, 120), Rgb(58, 60, 66), Rgb(20, 20, 22), Rgb(60, 62, 70), Rgb(0, 0, 0), true, true, false,
		false, true},
	{"marina_crew", "Marina Crew", 150, Rgb(244, 244, 248), Rgb(30, 40, 80), Rgb(214, 196, 160), Rgb(120, 80, 50), Rgb(0, 0, 0), Rgb(30, 40, 80), false, false,
		true, true, true},
};
constexpr int kOutfitCount = static_cast<int>(sizeof(kOutfits) / sizeof(kOutfits[0]));

// Expression presets: browRaise, browTilt, mouthWidth, mouthOpen, mouthCurve, eyeOpen
const FaceState kExpressionFaces[kExpressionCount] = {
	{0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},     // Neutral
	{0.5f, -6.0f, 1.25f, 0.15f, 1.0f, 0.85f},  // Happy
	{-0.7f, 20.0f, 0.9f, 0.0f, -0.6f, 0.75f},  // Angry
	{1.4f, -6.0f, 0.7f, 1.0f, 0.0f, 1.3f},     // Surprised
	{0.3f, -16.0f, 0.95f, 0.0f, -1.0f, 0.8f},  // Sad
	{-0.3f, 9.0f, 0.95f, 0.0f, -0.1f, 0.9f},   // Focused
};

inline float PseudoRandom(float seed)
{
	const float v = std::sin(seed * 12.9898f) * 43758.5453f;
	return v - std::floor(v);
}

inline Rot LerpRot(const Rot& a, const Rot& b, float t)
{
	return Rot{a.pitch + WrapDegrees(b.pitch - a.pitch) * t, a.yaw + WrapDegrees(b.yaw - a.yaw) * t, a.roll + WrapDegrees(b.roll - a.roll) * t};
}

inline void BlendPose(Pose& base, const Pose& layer, float w)
{
	if (w <= 0.0001f)
	{
		return;
	}
	for (int i = 0; i < kJointCount; ++i)
	{
		base.joints[i] = LerpRot(base.joints[i], layer.joints[i], w);
	}
	base.pelvisOffset = base.pelvisOffset + (layer.pelvisOffset - base.pelvisOffset) * w;
}

inline Rot& J(Pose& p, Joint j) { return p.joints[static_cast<int>(j)]; }

Pose LocomotionPose(const AnimState& s)
{
	Pose p;
	const float m = s.speed;
	const float a = SmoothStep(5.0f, 160.0f, m);
	const float r = SmoothStep(170.0f, 420.0f, m);
	const float sp = SmoothStep(420.0f, 680.0f, m);
	const float legSwing = a * Lerp(Lerp(24.0f, 36.0f, r), 48.0f, sp);
	const float kneeSwing = a * Lerp(Lerp(38.0f, 82.0f, r), 102.0f, sp);
	const float kneeStance = a * Lerp(4.0f, 14.0f, r);
	const float armSwing = a * Lerp(Lerp(18.0f, 32.0f, r), 48.0f, sp);
	const float elbow = Lerp(8.0f, Lerp(Lerp(14.0f, 76.0f, r), 92.0f, sp), a);
	const float lean = a * Lerp(Lerp(3.0f, 9.0f, r), 16.0f, sp);
	const float theta = 2.0f * kPi * s.gaitPhase;
	const float idle = 1.0f - a;
	const float breath = std::sin(s.time * 1.7f);

	const Joint hips[2] = {Joint::HipL, Joint::HipR};
	const Joint knees[2] = {Joint::KneeL, Joint::KneeR};
	const Joint ankles[2] = {Joint::AnkleL, Joint::AnkleR};
	for (int i = 0; i < 2; ++i)
	{
		const float ph = theta + (i == 0 ? 0.0f : kPi);
		const float hip = legSwing * std::sin(ph);
		const float knee = -(kneeStance + kneeSwing * Max(0.0f, std::cos(ph)));
		J(p, hips[i]).pitch = hip;
		J(p, knees[i]).pitch = knee;
		J(p, ankles[i]).pitch = -(hip + knee) * 0.85f;
	}
	// Idle weight shift.
	J(p, Joint::HipL).roll = idle * 1.5f * std::sin(s.time * 0.45f);
	J(p, Joint::HipR).roll = idle * 1.5f * std::sin(s.time * 0.45f);

	const float rollOut = Lerp(6.0f, 12.0f, r);
	J(p, Joint::ShoulderL).pitch = -armSwing * std::sin(theta) + 3.0f * idle;
	J(p, Joint::ShoulderR).pitch = armSwing * std::sin(theta) + 3.0f * idle;
	J(p, Joint::ShoulderL).roll = rollOut + idle * 0.8f * breath;
	J(p, Joint::ShoulderR).roll = -rollOut - idle * 0.8f * breath;
	J(p, Joint::ElbowL).pitch = elbow + a * 6.0f * Max(0.0f, -std::sin(theta));
	J(p, Joint::ElbowR).pitch = elbow + a * 6.0f * Max(0.0f, std::sin(theta));

	const float twist = a * Lerp(6.0f, 10.0f, r) * std::sin(theta);
	J(p, Joint::Pelvis).yaw = twist;
	J(p, Joint::Pelvis).roll = s.leanRoll + idle * 1.2f * std::sin(s.time * 0.45f);
	J(p, Joint::Spine).pitch = -lean * 0.5f;
	J(p, Joint::Chest).pitch = -lean * 0.5f + idle * 1.2f * breath;
	J(p, Joint::Chest).yaw = -twist * 1.8f;
	J(p, Joint::Head).pitch = lean * 0.9f;
	return p;
}

Pose AirPose(const AnimState& s)
{
	Pose p;
	const float rising = SmoothStep(-300.0f, 200.0f, s.verticalSpeed);
	J(p, Joint::HipL).pitch = Lerp(18.0f, 40.0f, rising);
	J(p, Joint::KneeL).pitch = Lerp(-35.0f, -72.0f, rising);
	J(p, Joint::HipR).pitch = Lerp(8.0f, -14.0f, rising);
	J(p, Joint::KneeR).pitch = Lerp(-28.0f, -26.0f, rising);
	J(p, Joint::AnkleL).pitch = 15.0f;
	J(p, Joint::AnkleR).pitch = 10.0f;
	J(p, Joint::ShoulderL).pitch = Lerp(30.0f, -20.0f, rising);
	J(p, Joint::ShoulderR).pitch = Lerp(30.0f, 35.0f, rising);
	J(p, Joint::ShoulderL).roll = Lerp(48.0f, 28.0f, rising);
	J(p, Joint::ShoulderR).roll = -Lerp(48.0f, 28.0f, rising);
	J(p, Joint::ElbowL).pitch = 35.0f;
	J(p, Joint::ElbowR).pitch = 40.0f;
	J(p, Joint::Spine).pitch = -4.0f;
	J(p, Joint::Head).pitch = 6.0f;
	J(p, Joint::Pelvis).roll = s.leanRoll;
	return p;
}

Pose SwimPose(const AnimState& s)
{
	// Treading water when still, front crawl when moving.
	Pose tread;
	const float tp = 2.0f * kPi * s.swimPhase;
	J(tread, Joint::Pelvis).pitch = -12.0f;
	J(tread, Joint::HipL).pitch = 20.0f * std::sin(tp * 1.5f);
	J(tread, Joint::HipR).pitch = -20.0f * std::sin(tp * 1.5f);
	J(tread, Joint::KneeL).pitch = -40.0f;
	J(tread, Joint::KneeR).pitch = -40.0f;
	J(tread, Joint::ShoulderL).pitch = 45.0f + 15.0f * std::sin(tp);
	J(tread, Joint::ShoulderR).pitch = 45.0f - 15.0f * std::sin(tp);
	J(tread, Joint::ShoulderL).roll = 38.0f;
	J(tread, Joint::ShoulderR).roll = -38.0f;
	J(tread, Joint::ElbowL).pitch = 60.0f;
	J(tread, Joint::ElbowR).pitch = 60.0f;
	J(tread, Joint::Head).pitch = 12.0f;
	tread.pelvisOffset = Vec3{0.0f, 0.0f, -20.0f};

	Pose crawl;
	J(crawl, Joint::Pelvis).pitch = -78.0f;
	J(crawl, Joint::Head).pitch = 50.0f;
	J(crawl, Joint::Neck).pitch = 10.0f;
	const float armL = 180.0f - 360.0f * s.swimPhase;
	J(crawl, Joint::ShoulderL).pitch = WrapDegrees(armL);
	J(crawl, Joint::ShoulderR).pitch = WrapDegrees(armL + 180.0f);
	J(crawl, Joint::ShoulderL).roll = 12.0f;
	J(crawl, Joint::ShoulderR).roll = -12.0f;
	J(crawl, Joint::ElbowL).pitch = 15.0f + 30.0f * Max(0.0f, std::sin(tp));
	J(crawl, Joint::ElbowR).pitch = 15.0f + 30.0f * Max(0.0f, -std::sin(tp));
	const float kick = 14.0f * std::sin(tp * 3.0f);
	J(crawl, Joint::HipL).pitch = kick;
	J(crawl, Joint::HipR).pitch = -kick;
	J(crawl, Joint::KneeL).pitch = -10.0f;
	J(crawl, Joint::KneeR).pitch = -10.0f;
	J(crawl, Joint::AnkleL).pitch = 40.0f;
	J(crawl, Joint::AnkleR).pitch = 40.0f;
	crawl.pelvisOffset = Vec3{0.0f, 0.0f, -28.0f};

	BlendPose(tread, crawl, SmoothStep(30.0f, 160.0f, s.speed));
	return tread;
}

Pose DrivePose(const AnimState& s)
{
	// Low sports-car seat: legs almost straight towards the pedals, reclined back.
	Pose p;
	p.pelvisOffset = Vec3{0.0f, 0.0f, -44.0f};
	J(p, Joint::HipL).pitch = 85.0f;
	J(p, Joint::HipR).pitch = 85.0f;
	J(p, Joint::KneeL).pitch = -30.0f;
	J(p, Joint::KneeR).pitch = -26.0f;
	J(p, Joint::HipL).roll = 5.0f;
	J(p, Joint::HipR).roll = -5.0f;
	J(p, Joint::AnkleL).pitch = -20.0f;
	J(p, Joint::AnkleR).pitch = -22.0f;
	J(p, Joint::Spine).pitch = 12.0f;
	J(p, Joint::Chest).pitch = 8.0f;
	J(p, Joint::Chest).roll = s.steer * 3.0f;
	J(p, Joint::ShoulderL).pitch = 70.0f + s.steer * 12.0f;
	J(p, Joint::ShoulderR).pitch = 70.0f - s.steer * 12.0f;
	J(p, Joint::ShoulderL).roll = 12.0f;
	J(p, Joint::ShoulderR).roll = -12.0f;
	J(p, Joint::ElbowL).pitch = 30.0f;
	J(p, Joint::ElbowR).pitch = 30.0f;
	J(p, Joint::Head).pitch = -16.0f;
	return p;
}

Pose ClimbPose(const AnimState& s)
{
	Pose p;
	const float t = SmoothStep(0.35f, 1.0f, s.climbAlpha);
	J(p, Joint::ShoulderL).pitch = Lerp(165.0f, 20.0f, t);
	J(p, Joint::ShoulderR).pitch = Lerp(165.0f, 20.0f, t);
	J(p, Joint::ShoulderL).roll = 14.0f;
	J(p, Joint::ShoulderR).roll = -14.0f;
	J(p, Joint::ElbowL).pitch = Lerp(15.0f, 95.0f, t);
	J(p, Joint::ElbowR).pitch = Lerp(15.0f, 95.0f, t);
	J(p, Joint::HipL).pitch = Lerp(25.0f, 85.0f, t);
	J(p, Joint::KneeL).pitch = Lerp(-45.0f, -110.0f, t);
	J(p, Joint::HipR).pitch = Lerp(25.0f, -5.0f, t);
	J(p, Joint::KneeR).pitch = Lerp(-45.0f, -20.0f, t);
	J(p, Joint::Spine).pitch = Lerp(-5.0f, -18.0f, t);
	J(p, Joint::Head).pitch = Lerp(20.0f, 10.0f, t);
	return p;
}

Pose DeadPose(const AnimState& /*s*/)
{
	Pose p;
	p.pelvisOffset = Vec3{0.0f, 0.0f, -80.0f};
	J(p, Joint::Pelvis).pitch = 86.0f;
	J(p, Joint::ShoulderL).roll = 65.0f;
	J(p, Joint::ShoulderR).roll = -55.0f;
	J(p, Joint::ShoulderL).pitch = 10.0f;
	J(p, Joint::ElbowL).pitch = 20.0f;
	J(p, Joint::ElbowR).pitch = 35.0f;
	J(p, Joint::KneeL).pitch = -12.0f;
	J(p, Joint::HipR).roll = -8.0f;
	J(p, Joint::Head).roll = 20.0f;
	return p;
}
} // namespace rigdata

// ---------------------------------------------------------------------------------------------
const RigJoint& GetJoint(Joint j)
{
	return rigdata::kJoints[static_cast<int>(j)];
}

const std::vector<RigSegment>& GetRigSegments()
{
	static const std::vector<RigSegment> segments = rigdata::BuildSegments();
	return segments;
}

int OutfitCount()
{
	return rigdata::kOutfitCount;
}

const Outfit& GetOutfit(int index)
{
	if (index < 0 || index >= rigdata::kOutfitCount)
	{
		index = 0;
	}
	return rigdata::kOutfits[index];
}

Color ResolveSlotColor(Slot slot, const Outfit& o)
{
	switch (slot)
	{
	case Slot::Skin: return rigdata::kSkin;
	case Slot::Hair: return rigdata::kHair;
	case Slot::Beard: return rigdata::kBeard;
	case Slot::Brow: return rigdata::kHair;
	case Slot::EyeWhite: return rigdata::kEyeWhite;
	case Slot::Iris: return rigdata::kIris;
	case Slot::Lip: return rigdata::kLip;
	case Slot::Shirt: return o.shirt;
	case Slot::ShirtAccent: return o.shirtAccent;
	case Slot::Top: return o.hasJacket ? o.jacket : o.shirt;
	case Slot::Forearm: return o.hasJacket ? o.jacket : (o.longSleeves ? o.shirt : rigdata::kSkin);
	case Slot::Pants: return o.pants;
	case Slot::Shin: return o.shorts ? rigdata::kSkin : o.pants;
	case Slot::Shoes: return o.shoes;
	case Slot::Sole: return rigdata::kSole;
	case Slot::Cap: return o.cap;
	case Slot::Lens: return rigdata::kLens;
	case Slot::Frame: return rigdata::kFrame;
	case Slot::Watch: return rigdata::kWatch;
	default: return Color{};
	}
}

bool IsSegmentVisible(Visibility vis, const Outfit& o)
{
	switch (vis)
	{
	case Visibility::JacketOnly: return o.hasJacket;
	case Visibility::CapOnly: return o.hasCap;
	case Visibility::NoCap: return !o.hasCap;
	case Visibility::SunglassesOnly: return o.sunglasses;
	case Visibility::NoSunglasses: return !o.sunglasses;
	default: return true;
	}
}

Mat ResolveSlotMaterial(Slot slot, Mat fallback)
{
	switch (slot)
	{
	case Slot::Iris:
	case Slot::Lens: return Mat::Glossy;
	case Slot::Watch:
	case Slot::Frame: return Mat::Metal;
	case Slot::Shoes: return Mat::Painted;
	case Slot::EyeWhite: return Mat::Painted;
	default: return fallback;
	}
}

void UpdateAnimState(AnimState& s, const AnimInput& in, float dt)
{
	dt = Clamp(dt, 0.0f, 0.1f);
	s.time += dt;
	s.speed = ExpApproach(s.speed, in.speed, 10.0f, dt);
	s.verticalSpeed = ExpApproach(s.verticalSpeed, in.verticalSpeed, 12.0f, dt);

	const bool inAir = !in.grounded && !in.swimming && !in.driving && !in.climbing && !in.dead;
	s.airWeight = ExpApproach(s.airWeight, inAir ? 1.0f : 0.0f, 9.0f, dt);
	s.swimWeight = ExpApproach(s.swimWeight, in.swimming ? 1.0f : 0.0f, 5.0f, dt);
	s.driveWeight = ExpApproach(s.driveWeight, in.driving ? 1.0f : 0.0f, 14.0f, dt);
	s.climbWeight = ExpApproach(s.climbWeight, in.climbing ? 1.0f : 0.0f, 14.0f, dt);
	s.deadWeight = ExpApproach(s.deadWeight, in.dead ? 1.0f : 0.0f, 5.0f, dt);
	s.climbAlpha = in.climbAlpha;

	const float run = SmoothStep(170.0f, 420.0f, s.speed);
	const float sprint = SmoothStep(420.0f, 680.0f, s.speed);
	const float stride = Lerp(Lerp(140.0f, 225.0f, run), 300.0f, sprint);
	s.gaitPhase += s.speed * dt / stride;
	s.gaitPhase -= std::floor(s.gaitPhase);
	s.swimPhase += dt * Lerp(0.35f, 0.8f, Saturate(s.speed / 300.0f));
	s.swimPhase -= std::floor(s.swimPhase);

	const float targetLean = Clamp(in.turnRate * 0.035f * Saturate(s.speed / 400.0f), -14.0f, 14.0f);
	s.leanRoll = ExpApproach(s.leanRoll, targetLean, 6.0f, dt);
	s.steer = ExpApproach(s.steer, in.steer, 10.0f, dt);
	s.lookYaw = ExpApproach(s.lookYaw, Clamp(WrapDegrees(in.lookYaw), -75.0f, 75.0f), 8.0f, dt);
	s.lookPitch = ExpApproach(s.lookPitch, Clamp(in.lookPitch, -35.0f, 35.0f), 8.0f, dt);

	// Blinking every 2.5-5 s.
	s.blinkTimer -= dt;
	if (s.blinkTimer <= 0.0f)
	{
		const float t = -s.blinkTimer;
		s.blink = Saturate(1.0f - Abs(t / 0.07f - 1.0f));
		if (t > 0.14f)
		{
			s.blink = 0.0f;
			s.blinkTimer = 2.5f + 2.5f * rigdata::PseudoRandom(s.time);
		}
	}
	else
	{
		s.blink = 0.0f;
	}

	const int target = static_cast<int>(in.expression);
	for (int e = 0; e < kExpressionCount; ++e)
	{
		s.expression[e] = ExpApproach(s.expression[e], e == target ? 1.0f : 0.0f, 7.0f, dt);
	}
	s.talking = in.talking;
	if (in.talking)
	{
		s.talkPhase += dt * 13.0f;
	}
}

Pose ComputePose(const AnimState& s)
{
	Pose pose = rigdata::LocomotionPose(s);
	// Plant the feet on the ground for the locomotion layer.
	pose.pelvisOffset = Vec3{0.0f, 0.0f, -LowestFootZ(pose)};

	rigdata::BlendPose(pose, rigdata::AirPose(s), s.airWeight);
	rigdata::BlendPose(pose, rigdata::SwimPose(s), s.swimWeight);
	rigdata::BlendPose(pose, rigdata::ClimbPose(s), s.climbWeight);
	rigdata::BlendPose(pose, rigdata::DrivePose(s), s.driveWeight);
	rigdata::BlendPose(pose, rigdata::DeadPose(s), s.deadWeight);

	// Head look follows the camera (reduced while swimming / driving / dead).
	const float lookScale = (1.0f - s.swimWeight * 0.7f) * (1.0f - s.deadWeight) * (1.0f - s.climbWeight);
	rigdata::J(pose, Joint::Neck).yaw += s.lookYaw * 0.35f * lookScale;
	rigdata::J(pose, Joint::Head).yaw += s.lookYaw * 0.45f * lookScale;
	rigdata::J(pose, Joint::Head).pitch += s.lookPitch * 0.5f * lookScale;

	// Face.
	FaceState face{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	float total = 0.0f;
	for (int e = 0; e < kExpressionCount; ++e)
	{
		total += s.expression[e];
	}
	if (total < 0.0001f)
	{
		face = rigdata::kExpressionFaces[0];
	}
	else
	{
		for (int e = 0; e < kExpressionCount; ++e)
		{
			const float w = s.expression[e] / total;
			const FaceState& f = rigdata::kExpressionFaces[e];
			face.browRaise += f.browRaise * w;
			face.browTilt += f.browTilt * w;
			face.mouthWidth += f.mouthWidth * w;
			face.mouthOpen += f.mouthOpen * w;
			face.mouthCurve += f.mouthCurve * w;
			face.eyeOpen += f.eyeOpen * w;
		}
	}
	if (s.talking)
	{
		face.mouthOpen = Saturate(face.mouthOpen + 0.5f * Abs(std::sin(s.talkPhase)));
	}
	face.eyeOpen *= (1.0f - s.blink) * (1.0f - s.deadWeight);
	pose.face = face;
	return pose;
}

SegmentTransform ApplyFace(const RigSegment& seg, const FaceState& face)
{
	SegmentTransform t{seg.center, seg.size, seg.rot};
	switch (seg.face)
	{
	case FacePart::BrowL:
		t.center.z += face.browRaise;
		t.rot.roll += face.browTilt;
		break;
	case FacePart::BrowR:
		t.center.z += face.browRaise;
		t.rot.roll -= face.browTilt;
		break;
	case FacePart::EyeL:
	case FacePart::EyeR: t.size.z *= Max(0.08f, face.eyeOpen); break;
	case FacePart::MouthCenter:
		t.size.y *= face.mouthWidth;
		t.size.z *= 1.0f + 3.0f * face.mouthOpen;
		t.center.z -= face.mouthOpen * 0.6f;
		break;
	case FacePart::MouthCornerL:
		t.center.y *= face.mouthWidth;
		t.center.z += face.mouthCurve * 0.7f - face.mouthOpen * 0.4f;
		t.rot.roll += face.mouthCurve * 25.0f;
		break;
	case FacePart::MouthCornerR:
		t.center.y *= face.mouthWidth;
		t.center.z += face.mouthCurve * 0.7f - face.mouthOpen * 0.4f;
		t.rot.roll -= face.mouthCurve * 25.0f;
		break;
	default: break;
	}
	return t;
}

void SolveJoints(const Pose& pose, JointWorld out[kJointCount])
{
	out[0].pos = Vec3{};
	out[0].basis = BasisFromRot(pose.joints[0]);
	for (int i = 1; i < kJointCount; ++i)
	{
		const RigJoint& j = rigdata::kJoints[i];
		const JointWorld& parent = out[static_cast<int>(j.parent)];
		Vec3 offset = j.offset;
		if (i == static_cast<int>(Joint::Pelvis))
		{
			offset = offset + pose.pelvisOffset;
		}
		out[i].pos = parent.pos + Apply(parent.basis, offset);
		out[i].basis = Compose(parent.basis, BasisFromRot(pose.joints[i]));
	}
}

float LowestFootZ(const Pose& pose)
{
	JointWorld w[kJointCount];
	SolveJoints(pose, w);
	float lowest = 1e9f;
	const Joint ankles[2] = {Joint::AnkleL, Joint::AnkleR};
	const Vec3 soleCorners[2] = {Vec3{-8.0f, 0.0f, -7.1f}, Vec3{18.0f, 0.0f, -7.1f}};
	for (Joint a : ankles)
	{
		const JointWorld& jw = w[static_cast<int>(a)];
		for (const Vec3& c : soleCorners)
		{
			lowest = Min(lowest, (jw.pos + Apply(jw.basis, c)).z);
		}
	}
	return lowest;
}
} // namespace hadi
