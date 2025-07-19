#include "RpgPhysicsTypes.h"
#include "core/world/RpgWorld.h"
#include "thirdparty/libccd/ccd.h"
#include "world/RpgPhysicsComponent.h"


#define RPG_PHYSICS_COLLISION_GJK_MAX_ITERATIONS		(32)
#define RPG_PHYSICS_COLLISION_GJK_MAX_EPA_TOLERANCE		(0.001f)
#define RPG_PHYSICS_COLLISION_GJK_DISTANCE_TOLERANCE	(0.001f)



namespace RpgPhysicsGJK
{
	static void SupportSphere(const void* obj, const ccd_vec3_t* dir, ccd_vec3_t* vec) noexcept
	{
		const RpgBoundingSphere* sphere = reinterpret_cast<const RpgBoundingSphere*>(obj);
		const RpgVector3 direction(dir->v[0], dir->v[1], dir->v[2]);
		const RpgVector3 farthestPoint = sphere->GetCenter() + direction.GetNormalize() * sphere->GetRadius();

		ccdVec3Set(vec, farthestPoint.X, farthestPoint.Y, farthestPoint.Z);
	}


	static void SupportBox(const void* obj, const ccd_vec3_t* dir, ccd_vec3_t* vec) noexcept
	{
		const RpgBoundingBox* box = reinterpret_cast<const RpgBoundingBox*>(obj);
		const RpgBoundingBox::FCornerPoints corners = box->GetCornerPoints();
		const RpgVector3 direction(dir->v[0], dir->v[1], dir->v[2]);

		float maxDot = -FLT_MAX;
		int cornerIndex = -1;

		for (int i = 0; i < 8; ++i)
		{
			const float dot = RpgVector3::DotProduct(corners.Points[i], direction);
			if (dot > maxDot)
			{
				maxDot = dot;
				cornerIndex = i;
			}
		}

		RPG_Check(cornerIndex != -1);
		const RpgVector3 farthestPoint = corners.Points[cornerIndex];

		ccdVec3Set(vec, farthestPoint.X, farthestPoint.Y, farthestPoint.Z);
	}


	static void SupportCapsule(const void* obj, const ccd_vec3_t* dir, ccd_vec3_t* vec) noexcept
	{
		const RpgBoundingCapsule* capsule = reinterpret_cast<const RpgBoundingCapsule*>(obj);
		const RpgVector3 direction(dir->v[0], dir->v[1], dir->v[2]);

		// Capsule always sweep along y-axis
		const RpgVector3 CAPSULE_AXIS(0.0f, 1.0f, 0.0f);

		const float dotAxis = RpgVector3::DotProduct(direction, CAPSULE_AXIS);
		const RpgVector3 axisDir = (dotAxis >= 0.0f) ? CAPSULE_AXIS : CAPSULE_AXIS * -1.0f;
		const RpgVector3 capCenter = capsule->Center + axisDir * capsule->HalfHeight;
		const RpgVector3 farthestPoint = capCenter + direction.GetNormalize() * capsule->Radius;

		ccdVec3Set(vec, farthestPoint.X, farthestPoint.Y, farthestPoint.Z);
	}

};



namespace RpgPhysicsCollision
{
	const FResponseChannels DEFAULT_COLLISION_RESPONSE_CHANNELS_IgnoreAll =
	{
		RpgPhysicsCollision::RESPONSE_IGNORE,		// NONE
		RpgPhysicsCollision::RESPONSE_IGNORE,		// BLOCKER
		RpgPhysicsCollision::RESPONSE_IGNORE,		// TRIGGER
		RpgPhysicsCollision::RESPONSE_IGNORE,		// CHARACTER
		RpgPhysicsCollision::RESPONSE_IGNORE,		// INTERACTABLE
		RpgPhysicsCollision::RESPONSE_IGNORE,		// ABILITY
		RpgPhysicsCollision::RESPONSE_IGNORE,		// MOUSE_SELECT
	};


	const FResponseChannels DEFAULT_COLLISION_RESPONSE_CHANNELS_Blocker =
	{
		RpgPhysicsCollision::RESPONSE_IGNORE,	// NONE
		RpgPhysicsCollision::RESPONSE_IGNORE,	// BLOCKER
		RpgPhysicsCollision::RESPONSE_IGNORE,	// TRIGGER
		RpgPhysicsCollision::RESPONSE_BLOCK,	// CHARACTER
		RpgPhysicsCollision::RESPONSE_IGNORE,	// INTERACTABLE
		RpgPhysicsCollision::RESPONSE_BLOCK,	// ABILITY
		RpgPhysicsCollision::RESPONSE_BLOCK,	// MOUSE_SELECT
	};


	const FResponseChannels DEFAULT_COLLISION_RESPONSE_CHANNELS_Character =
	{
		RpgPhysicsCollision::RESPONSE_IGNORE,	// NONE
		RpgPhysicsCollision::RESPONSE_BLOCK,	// BLOCKER
		RpgPhysicsCollision::RESPONSE_OVERLAP,	// TRIGGER
		RpgPhysicsCollision::RESPONSE_BLOCK,	// CHARACTER
		RpgPhysicsCollision::RESPONSE_IGNORE,	// INTERACTABLE
		RpgPhysicsCollision::RESPONSE_OVERLAP,	// ABILITY
		RpgPhysicsCollision::RESPONSE_BLOCK,	// MOUSE_SELECT
	};




// =========================================================================================================================================================== //
// FILTER
// =========================================================================================================================================================== //
	void Filter::GeneratePairs(RpgArray<FPairTest>& out_Pairs, RpgWorld* world) noexcept
	{
		for (auto firstIt = world->Component_CreateIterator<RpgPhysicsComponent_Filter>(); firstIt; ++firstIt)
		{
			RpgPhysicsComponent_Filter& firstFilter = firstIt.GetValue();

			if (firstFilter.ObjectChannel == RpgPhysicsCollision::CHANNEL_NONE)
			{
				continue;
			}


			for (auto secondIt = firstIt; secondIt; ++secondIt)
			{
				if (firstIt == secondIt)
				{
					continue;
				}

				const RpgPhysicsComponent_Filter& secondFilter = secondIt.GetValue();

				if (secondFilter.ObjectChannel == RpgPhysicsCollision::CHANNEL_NONE)
				{
					continue;
				}

				if (firstFilter.ResponseChannels[secondFilter.ObjectChannel] == secondFilter.ResponseChannels[firstFilter.ObjectChannel])
				{
					RpgPhysicsComponent_Collision* firstCollision = world->GameObject_GetComponent<RpgPhysicsComponent_Collision>(firstFilter.GameObject);
					RPG_Check(firstCollision && firstCollision->GameObject == firstFilter.GameObject);

					RpgPhysicsComponent_Collision* secondCollision = world->GameObject_GetComponent<RpgPhysicsComponent_Collision>(secondFilter.GameObject);
					RPG_Check(secondCollision && secondCollision->GameObject == secondFilter.GameObject);

					out_Pairs.AddValue({ firstCollision, secondCollision });
				}
			}
		}
	}
	



// =========================================================================================================================================================== //
// BROADPHASE
// =========================================================================================================================================================== //
	void Broadphase::GeneratePairs(RpgArray<FPairTest>& out_Pairs, const RpgArray<FPairTest>& filterPairs) noexcept
	{

	}




// =========================================================================================================================================================== //
// NARROWPHASE
// =========================================================================================================================================================== //
	bool Narrowphase::TestOverlapSphereSphere(RpgBoundingSphere first, RpgBoundingSphere second, RpgPhysicsCollision::FContactResult* optOut_Result) noexcept
	{
		const RpgVector3 firstCenter = first.GetCenter();
		const float firstRadius = first.GetRadius();

		const RpgVector3 secondCenter = second.GetCenter();
		const float secondRadius = second.GetRadius();

		const RpgVector3 delta = secondCenter - firstCenter;
		const float magSqr = delta.GetMagnitudeSqr();
		const float radiusSqr = (firstRadius + secondRadius) * (firstRadius * secondRadius);
		const bool bOverlapped = magSqr <= radiusSqr;

		if (bOverlapped && optOut_Result)
		{
			const RpgVector3 deltaNormalized = delta.GetNormalize();
			optOut_Result->SeparationDirection = deltaNormalized;
			optOut_Result->ContactPoint = deltaNormalized * secondRadius;
			optOut_Result->PenetrationDepth = RpgMath::Sqrt(magSqr);
		}

		return bOverlapped;
	}


	bool Narrowphase::TestOverlapSphereBox(RpgBoundingSphere sphere, RpgBoundingBox box, RpgPhysicsCollision::FContactResult* optOut_Result) noexcept
	{
		ccd_t ccd;
		CCD_INIT(&ccd);
		ccd.support1 = RpgPhysicsGJK::SupportSphere;
		ccd.support2 = RpgPhysicsGJK::SupportBox;
		ccd.max_iterations = RPG_PHYSICS_COLLISION_GJK_MAX_ITERATIONS;
		ccd.epa_tolerance = RPG_PHYSICS_COLLISION_GJK_MAX_EPA_TOLERANCE;
		ccd.dist_tolerance = RPG_PHYSICS_COLLISION_GJK_DISTANCE_TOLERANCE;

		bool bOverlapped = false;

		if (optOut_Result)
		{
			ccd_real_t depth = 0.0f;
			ccd_vec3_t separationDirection;
			ccd_vec3_t contactPoint;

			const int ret = ccdGJKPenetration(&sphere, &box, &ccd, &depth, &separationDirection, &contactPoint);
			RPG_Check(ret != -2);

			if (ret == 0)
			{
				bOverlapped = true;
				optOut_Result->ContactPoint = RpgVector3(contactPoint.v[0], contactPoint.v[1], contactPoint.v[2]);
				optOut_Result->SeparationDirection = RpgVector3(separationDirection.v[0], separationDirection.v[1], separationDirection.v[2]);
				optOut_Result->SeparationDirection.Normalize();
				optOut_Result->PenetrationDepth = depth;
			}
		}
		else
		{
			bOverlapped = ccdGJKIntersect(&sphere, &box, &ccd);
		}

		return bOverlapped;
	}


	bool Narrowphase::TestOverlapBoxBox(RpgBoundingBox first, RpgBoundingBox second, RpgPhysicsCollision::FContactResult* optOut_Result) noexcept
	{
		ccd_t ccd;
		CCD_INIT(&ccd);
		ccd.support1 = RpgPhysicsGJK::SupportBox;
		ccd.support2 = RpgPhysicsGJK::SupportBox;
		ccd.max_iterations = RPG_PHYSICS_COLLISION_GJK_MAX_ITERATIONS;
		ccd.epa_tolerance = RPG_PHYSICS_COLLISION_GJK_MAX_EPA_TOLERANCE;
		ccd.dist_tolerance = RPG_PHYSICS_COLLISION_GJK_DISTANCE_TOLERANCE;

		return ccdGJKIntersect(&first, &second, &ccd);
	}


	bool Narrowphase::TestOverlapBoxSphere(RpgBoundingBox box, RpgBoundingSphere sphere, RpgPhysicsCollision::FContactResult* optOut_Result) noexcept
	{
		ccd_t ccd;
		CCD_INIT(&ccd);
		ccd.support1 = RpgPhysicsGJK::SupportBox;
		ccd.support2 = RpgPhysicsGJK::SupportSphere;
		ccd.max_iterations = RPG_PHYSICS_COLLISION_GJK_MAX_ITERATIONS;
		ccd.epa_tolerance = RPG_PHYSICS_COLLISION_GJK_MAX_EPA_TOLERANCE;
		ccd.dist_tolerance = RPG_PHYSICS_COLLISION_GJK_DISTANCE_TOLERANCE;

		return ccdGJKIntersect(&box, &sphere, &ccd);
	}

};
