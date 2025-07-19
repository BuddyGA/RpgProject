#include "RpgAnimationTypes.h"



RpgAnimationClip::RpgAnimationClip(const RpgName& in_Name, float in_DurationSeconds) noexcept
{
	RPG_Check(in_DurationSeconds > 0.0f);

	RPG_Log(RpgLogTemp, "Create animation (%s)", *in_Name);

	Name = in_Name;
	DurationSeconds = in_DurationSeconds;
}


RpgAnimationClip::~RpgAnimationClip() noexcept
{
}


void RpgAnimationClip::SaveToAssetFile(const RpgString& filePath) noexcept
{
	RPG_NotImplementedYet();
}


void RpgAnimationClip::LoadFromAssetFile(const RpgString& filePath) noexcept
{
	RPG_NotImplementedYet();
}


void RpgAnimationClip::AddTrack(const RpgAnimationTrack& in_Track) noexcept
{
	bool bShouldAdd = true;

	for (int t = 0; t < Tracks.GetCount(); ++t)
	{
		if (Tracks[t].BoneName == in_Track.BoneName)
		{
			bShouldAdd = false;
			break;
		}
	}

	RPG_Check(bShouldAdd);

	if (bShouldAdd)
	{
		Tracks.AddValue(in_Track);
	}
}


bool RpgAnimationClip::CheckSkeletonCompatibility(const RpgAnimationSkeleton* skeleton) const noexcept
{
	if (!skeleton)
	{
		return false;
	}

	for (int t = 0; t < Tracks.GetCount(); ++t)
	{
		const int boneIndex = skeleton->GetBoneIndex(Tracks[t].BoneName);

		if (boneIndex == RPG_SKELETON_BONE_INDEX_INVALID)
		{
			return false;
		}
	}

	return true;
}


RpgSharedAnimationClip RpgAnimationClip::s_CreateShared(const RpgName& name, float durationSeconds) noexcept
{
	return RpgSharedAnimationClip(new RpgAnimationClip(name, durationSeconds));
}
