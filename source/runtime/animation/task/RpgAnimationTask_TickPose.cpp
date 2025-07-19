#include "RpgAnimationTask_TickPose.h"
#include "core/world/RpgWorld.h"
#include "../world/RpgAnimationComponent.h"



RpgAnimationTask_TickPose::RpgAnimationTask_TickPose() noexcept
{
	World = nullptr;
	DeltaTime = 0.0f;
	GlobalPlayRate = 1.0f;
}


void RpgAnimationTask_TickPose::Reset() noexcept
{
	RpgThreadTask::Reset();

	World = nullptr;
	DeltaTime = 0.0f;
	GlobalPlayRate = 1.0f;
	AnimationComponents.Clear();
}


void RpgAnimationTask_TickPose::Execute() noexcept
{
	for (int i = 0; i < AnimationComponents.GetCount(); ++i)
	{
		RpgAnimationComponent_AnimSkeletonPose* comp = AnimationComponents[i];

		// Check if paused
		if (comp->bPauseAnim)
		{
			continue;
		}

		// Skeleton must valid
		if (!comp->Skeleton)
		{
			RPG_LogWarn(RpgLogAnimation, "Fail to update animation for game object (%s). Invalid skeleton!", *World->GameObject_GetName(comp->GameObject));
			continue;
		}

		// AnimClip must valid
		if (!comp->Clip)
		{
			RPG_LogWarn(RpgLogAnimation, "Fail to update animation for game object (%s). Invalid animation clip!", *World->GameObject_GetName(comp->GameObject));
			continue;
		}

		const RpgAnimationSkeleton* skeleton = comp->Skeleton.Get();
		const RpgAnimationClip* animClip = comp->Clip.Get();

		const float animDurationSeconds = animClip->GetDurationSeconds();
		const float animPlayRate = RpgMath::Clamp(comp->PlayRate * GlobalPlayRate, 0.1f, 100.0f);
		comp->AnimTimer += DeltaTime * animPlayRate;

		if (comp->bLoopAnim)
		{
			comp->AnimTimer = RpgMath::ModF(comp->AnimTimer, animDurationSeconds);
		}
		else
		{
			if (comp->AnimTimer >= animDurationSeconds)
			{
				comp->AnimTimer = animDurationSeconds;
				continue;
			}
		}


	#ifndef RPG_BUILD_SHIPPING
		if (!animClip->CheckSkeletonCompatibility(skeleton))
		{
			RPG_LogWarn(RpgLogAnimation, "Animation clip (%s) is not compatible with skeleton (%s)", *animClip->GetName(), *skeleton->GetName());
			continue;
		}
	#endif // !RPG_BUILD_SHIPPING


		const float sampleTime = comp->AnimTimer;
		const int boneCount = skeleton->GetBoneCount();


		// Update bone local transforms
		const RpgArray<RpgAnimationTrack>& animationTracks = animClip->GetTracks();

		for (int i = 0; i < animationTracks.GetCount(); ++i)
		{
			const RpgAnimationTrack& track = animationTracks[i];
			bool bMarkDirty = false;

			// Position
			RpgVector3 interpolatedPosition;
			const RpgArray<RpgAnimationTrack::FKeyPosition>& keyPositions = track.KeyPositions;

			for (int p = 0; p < keyPositions.GetCount(); ++p)
			{
				if (sampleTime >= keyPositions[p].Timestamp && sampleTime <= keyPositions[p + 1].Timestamp)
				{
					const RpgAnimationTrack::FKeyPosition key0 = keyPositions[p];
					const RpgAnimationTrack::FKeyPosition key1 = keyPositions[p + 1];
					const float timeDiff = key1.Timestamp - key0.Timestamp;
					const float t = (timeDiff > 0.0f) ? (sampleTime - key0.Timestamp) / timeDiff : 0.0f;
					interpolatedPosition = RpgVector3::Lerp(key0.Value, key1.Value, t);

					bMarkDirty = true;

					break;
				}
			}

			// Rotation
			RpgQuaternion interpolatedRotation;
			const RpgArray<RpgAnimationTrack::FKeyRotation>& keyRotations = track.KeyRotations;

			for (int r = 0; r < keyRotations.GetCount(); ++r)
			{
				if (sampleTime >= keyRotations[r].Timestamp && sampleTime <= keyRotations[r + 1].Timestamp)
				{
					const RpgAnimationTrack::FKeyRotation key0 = keyRotations[r];
					const RpgAnimationTrack::FKeyRotation key1 = keyRotations[r + 1];
					const float timeDiff = key1.Timestamp - key0.Timestamp;
					const float t = (timeDiff > 0.0f) ? (sampleTime - key0.Timestamp) / timeDiff : 0.0f;
					interpolatedRotation = RpgQuaternion::Slerp(key0.Value, key1.Value, t);

					bMarkDirty = true;

					break;
				}
			}

			if (bMarkDirty)
			{
				const int boneIndex = skeleton->GetBoneIndex(track.BoneName);
				RPG_Check(boneIndex != RPG_SKELETON_BONE_INDEX_INVALID);
				comp->FinalPose.SetBoneLocalTransform(boneIndex, RpgMatrixTransform(interpolatedPosition, interpolatedRotation));
			}
		}

		// Update bone pose transforms
		comp->FinalPose.UpdateBonePoseTransforms(skeleton);
	}
}
