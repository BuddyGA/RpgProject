#pragma once

#include "core/world/RpgComponent.h"
#include "../RpgAnimationTypes.h"



// ======================================================================================================================= //
// ANIMATION COMPONENT
// ======================================================================================================================= //
class RpgAnimationComponent_AnimSkeletonPose
{
	RPG_COMPONENT_TYPE("RpgComponent (Animation) - AnimSkeletonPose")

public:
	RpgSharedAnimationClip Clip;
	float PlayRate;
	bool bLoopAnim;
	bool bPauseAnim;


public:
	RpgAnimationComponent_AnimSkeletonPose() noexcept
	{
		PlayRate = 1.0f;
		bLoopAnim = false;
		bPauseAnim = false;
		AnimTimer = 0.0f;
	}


	inline void Destroy() noexcept
	{
		// Nothing to do
	}


	inline void SetSkeleton(const RpgSharedAnimationSkeleton& in_Skeleton) noexcept
	{
		if (Skeleton != in_Skeleton)
		{
			Skeleton = in_Skeleton;
			ResetPose();
		}
	}

	[[nodiscard]] inline const RpgSharedAnimationSkeleton& GetSkeleton() const noexcept
	{
		return Skeleton;
	}


	inline void ResetPose() noexcept
	{
		FinalPose.Clear(true);

		if (Skeleton)
		{
			FinalPose = Skeleton->GetBindPose();
		}
	}


	[[nodiscard]] inline const RpgAnimationPose& GetFinalPose() const noexcept
	{
		return FinalPose;
	}


private:
	RpgSharedAnimationSkeleton Skeleton;
	RpgAnimationPose FinalPose;
	float AnimTimer;


	friend RpgAnimationWorldSubsystem;
	friend RpgAnimationTask_TickPose;

};
