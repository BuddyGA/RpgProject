#pragma once

#include "core/RpgMath.h"
#include "core/dsa/RpgArray.h"
#include "core/world/RpgGameObject.h"


#define RPG_PHYSICS_COLLISION_MAX_CONVEX_VERTICES	32
#define RPG_PHYSICS_COLLISION_MAX_CONTACT_RESULT	8
#define RPG_PHYSICS_TRACE_MAX_HIT_RESULT			10


class RpgPhysicsComponent_Filter;
class RpgPhysicsComponent_Collision;
class RpgPhysicsWorldSubsystem;
class RpgPhysicsTask_UpdateBound;
class RpgPhysicsTask_UpdateShape;



namespace RpgPhysicsCollision
{
	enum EShape : uint8_t
	{
		SHAPE_NONE = 0,
		SHAPE_SPHERE,
		SHAPE_BOX,
		SHAPE_CAPSULE,
		SHAPE_MESH_CONVEX,
		SHAPE_MESH_TRIANGLE,
		SHAPE_MAX_COUNT
	};

	constexpr const char* SHAPE_NAMES[SHAPE_MAX_COUNT] =
	{
		"<None>",
		"Sphere",
		"Box",
		"Capsule",
		"Mesh Convex",
		"Mesh Triangle"
	};
	

	enum EChannel : uint8_t
	{
		CHANNEL_NONE = 0,
		CHANNEL_BLOCKER,
		CHANNEL_TRIGGER,
		CHANNEL_CHARACTER,
		CHANNEL_INTERACTABLE,
		CHANNEL_ABILITY,
		CHANNEL_MOUSE_SELECT,
		CHANNEL_MAX_COUNT
	};


	enum EResponse : uint8_t
	{
		RESPONSE_IGNORE = 0,
		RESPONSE_OVERLAP,
		RESPONSE_BLOCK
	};


	struct FPairTest
	{
		RpgPhysicsComponent_Collision* FirstCollision{ nullptr };
		RpgPhysicsComponent_Collision* SecondCollision{ nullptr };
	};


	struct FContactResult
	{
		RpgVector3 ContactPoint;
		RpgVector3 SeparationDirection;
		float PenetrationDepth{ 0.0f };
	};


	struct FContactManifold
	{
		RpgPhysicsComponent_Collision* FirstCollision{ nullptr };
		RpgPhysicsComponent_Collision* SecondCollision{ nullptr };
		RpgArrayInline<FContactResult, RPG_PHYSICS_COLLISION_MAX_CONTACT_RESULT> FirstContactResults;
		RpgArrayInline<FContactResult, RPG_PHYSICS_COLLISION_MAX_CONTACT_RESULT> SecondContactResults;
	};


	typedef RpgArrayInline<RpgPhysicsCollision::EResponse, RpgPhysicsCollision::CHANNEL_MAX_COUNT> FResponseChannels;

	// Default collision response channels to ignore all
	extern const FResponseChannels DEFAULT_COLLISION_RESPONSE_CHANNELS_IgnoreAll;

	// Default collision response channels for blocker (floor, wall, pillar, etc)
	extern const FResponseChannels DEFAULT_COLLISION_RESPONSE_CHANNELS_Blocker;

	// Default collision response channels for character
	extern const FResponseChannels DEFAULT_COLLISION_RESPONSE_CHANNELS_Character;


	
	namespace Filter
	{
		extern void GeneratePairs(RpgArray<FPairTest>& out_Pairs, RpgWorld* world) noexcept;
	};


	namespace Broadphase
	{
		extern void GeneratePairs(RpgArray<FPairTest>& out_Pairs, const RpgArray<FPairTest>& filterPairs) noexcept;
	};


	namespace Narrowphase
	{
		extern bool TestOverlapSphereSphere(RpgBoundingSphere first, RpgBoundingSphere second, FContactResult* optOut_Result = nullptr) noexcept;
		extern bool TestOverlapSphereBox(RpgBoundingSphere sphere, RpgBoundingBox box, FContactResult* optOut_Result = nullptr) noexcept;
		extern bool TestOverlapBoxBox(RpgBoundingBox first, RpgBoundingBox second, FContactResult* optOut_Result = nullptr) noexcept;
		extern bool TestOverlapBoxSphere(RpgBoundingBox box, RpgBoundingSphere sphere, FContactResult* optOut_Result = nullptr) noexcept;
	};

};




namespace RpgPhysicsTrace
{
	struct FOption
	{
		RpgArray<RpgGameObjectID> IgnoredGameObjects;
		RpgPhysicsCollision::EChannel Channel{ RpgPhysicsCollision::CHANNEL_NONE };
	};


	struct FResult
	{
		RpgVector3 HitLocation;
		RpgVector3 ContactLocation;
		RpgVector3 ContactNormal;
		RpgPhysicsComponent_Collision* Component{ nullptr };
	};

	typedef RpgArrayInline<FResult, RPG_PHYSICS_TRACE_MAX_HIT_RESULT> FResultArray;



	extern FResult LineOne(const RpgWorld* world, RpgVector3 start, RpgVector3 end, const FOption& option) noexcept;
	extern FResultArray LineMany(const RpgWorld* world, RpgVector3 start, RpgVector3 end, const FOption& option) noexcept;
		   
	extern FResult SphereOne(const RpgWorld* world, RpgVector3 center, float radius, const FOption& option) noexcept;
	extern FResultArray SphereMany(const RpgWorld* world, RpgVector3 center, float radius, const FOption& option) noexcept;

};
