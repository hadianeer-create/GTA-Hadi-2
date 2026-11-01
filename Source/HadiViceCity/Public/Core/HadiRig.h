// HADI: VICE CITY - Simulation Core
// Hadi's procedural body rig, wardrobe, facial expressions and animation solver.
//
// The rig is a hierarchy of joints with primitive "segments" (body parts) attached. The
// engine builds one pivot component per joint and one mesh component per segment, then
// every frame asks ComputePose() for joint rotations. Because the solver is pure math it is
// unit-tested (feet stay planted, no NaNs) and previewed in Tools/Preview.
//
// When a production skeletal mesh is authored (Blender -> FBX), the same AnimInput drives an
// Animation Blueprint instead; see Docs/ARCHITECTURE.md.
#pragma once

#include "Core/HadiMath.h"
#include "Core/HadiTypes.h"

#include <cstdint>
#include <vector>

namespace hadi
{
enum class Joint : uint8_t
{
	Root,
	Pelvis,
	Spine,
	Chest,
	Neck,
	Head,
	ShoulderL,
	ElbowL,
	WristL,
	ShoulderR,
	ElbowR,
	WristR,
	HipL,
	KneeL,
	AnkleL,
	HipR,
	KneeR,
	AnkleR,
	Count
};
constexpr int kJointCount = static_cast<int>(Joint::Count);

// What colour a segment takes from the current outfit.
enum class Slot : uint8_t
{
	Skin,
	Hair,
	Beard,
	Brow,
	EyeWhite,
	Iris,
	Lip,
	Shirt,       // always the shirt colour
	ShirtAccent, // collar / pocket trim
	Top,         // jacket if worn, otherwise shirt
	Forearm,     // jacket > long-sleeve shirt > bare skin
	Pants,
	Shin,        // bare skin when wearing shorts
	Shoes,
	Sole,
	Cap,
	Lens,
	Frame,
	Watch,
	Count
};

enum class Visibility : uint8_t
{
	Always,
	JacketOnly,
	CapOnly,
	NoCap,
	SunglassesOnly,
	NoSunglasses
};

enum class FacePart : uint8_t
{
	None,
	BrowL,
	BrowR,
	EyeL,
	EyeR,
	MouthCenter,
	MouthCornerL,
	MouthCornerR
};

struct RigJoint
{
	Joint parent = Joint::Root;
	Vec3 offset; // rest position relative to the parent joint (cm)
	const char* name = "";
};

struct RigSegment
{
	Joint joint = Joint::Root;
	Shape shape = Shape::Cube;
	Vec3 center; // relative to the joint
	Vec3 size;
	Rot rot;
	Slot slot = Slot::Skin;
	Visibility vis = Visibility::Always;
	FacePart face = FacePart::None;
	Mat mat = Mat::Matte;
};

const RigJoint& GetJoint(Joint j);
const std::vector<RigSegment>& GetRigSegments();

// ---- Wardrobe -------------------------------------------------------------------------------
struct Outfit
{
	const char* id = "";
	const char* name = "";
	int price = 0;
	Color shirt;
	Color shirtAccent;
	Color pants;
	Color shoes;
	Color jacket;
	Color cap;
	bool hasJacket = false;
	bool longSleeves = false;
	bool shorts = false;
	bool hasCap = false;
	bool sunglasses = false;
};

int OutfitCount();
const Outfit& GetOutfit(int index);
Color ResolveSlotColor(Slot slot, const Outfit& outfit);
bool IsSegmentVisible(Visibility vis, const Outfit& outfit);
Mat ResolveSlotMaterial(Slot slot, Mat fallback);

// ---- Animation ------------------------------------------------------------------------------
enum class Expression : uint8_t
{
	Neutral,
	Happy,
	Angry,
	Surprised,
	Sad,
	Focused,
	Count
};
constexpr int kExpressionCount = static_cast<int>(Expression::Count);

struct AnimInput
{
	float speed = 0.0f;         // horizontal speed, cm/s
	float verticalSpeed = 0.0f; // cm/s
	float turnRate = 0.0f;      // deg/s, positive = turning right
	bool grounded = true;
	bool swimming = false;
	bool driving = false;
	float steer = 0.0f;         // -1..1 while driving
	bool climbing = false;
	float climbAlpha = 0.0f;    // 0..1 progress of a mantle
	bool dead = false;
	float lookYaw = 0.0f;       // where the camera points relative to the body (deg)
	float lookPitch = 0.0f;
	Expression expression = Expression::Neutral;
	bool talking = false;
};

struct FaceState
{
	float browRaise = 0.0f;  // cm
	float browTilt = 0.0f;   // deg, positive = inner ends down (angry)
	float mouthWidth = 1.0f; // scale
	float mouthOpen = 0.0f;  // 0..1
	float mouthCurve = 0.0f; // -1 frown .. +1 smile
	float eyeOpen = 1.0f;    // 0 closed .. 1.3 wide
};

struct AnimState
{
	float time = 0.0f;
	float gaitPhase = 0.0f;  // 0..1 locomotion cycle
	float swimPhase = 0.0f;
	float speed = 0.0f;      // smoothed
	float verticalSpeed = 0.0f;
	float airWeight = 0.0f;
	float swimWeight = 0.0f;
	float driveWeight = 0.0f;
	float climbWeight = 0.0f;
	float deadWeight = 0.0f;
	float climbAlpha = 0.0f;
	float leanRoll = 0.0f;
	float steer = 0.0f;
	float lookYaw = 0.0f;
	float lookPitch = 0.0f;
	float blinkTimer = 2.0f;
	float blink = 0.0f;
	float talkPhase = 0.0f;
	bool talking = false;
	float expression[kExpressionCount] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
};

struct Pose
{
	Rot joints[kJointCount];
	Vec3 pelvisOffset;
	FaceState face;
};

void UpdateAnimState(AnimState& state, const AnimInput& input, float dt);
Pose ComputePose(const AnimState& state);

// Transform of a segment after facial animation is applied (centre/size/rotation in joint space).
struct SegmentTransform
{
	Vec3 center;
	Vec3 size;
	Rot rot;
};
SegmentTransform ApplyFace(const RigSegment& seg, const FaceState& face);

// Forward kinematics: world (rig-root space) position + basis of every joint.
struct JointWorld
{
	Vec3 pos;
	Basis basis;
};
void SolveJoints(const Pose& pose, JointWorld out[kJointCount]);
// Lowest point of either foot in rig-root space (used to plant the feet).
float LowestFootZ(const Pose& pose);
} // namespace hadi
