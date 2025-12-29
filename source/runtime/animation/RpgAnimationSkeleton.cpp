#include "RpgAnimationTypes.h"



RpgAnimationSkeleton::RpgAnimationSkeleton(const RpgName& name) noexcept
{
	RPG_Log(RpgLogTemp, "Create skeleton (%s)", *name);

	Name = name;
	BoneNames.Reserve(16);
	BoneParentIndices.Reserve(16);
	BoneInverseBindPoseTransforms.Reserve(16);
}


RpgAnimationSkeleton::~RpgAnimationSkeleton() noexcept
{
}


void RpgAnimationSkeleton::SaveToAssetFile(const RpgString& filePath) noexcept
{
	RPG_NotImplementedYet();
}


void RpgAnimationSkeleton::LoadFromAssetFile(const RpgString& filePath) noexcept
{
	RPG_NotImplementedYet();
}




RpgSharedAnimationSkeleton RpgAnimationSkeleton::s_CreateShared(const RpgName& name) noexcept
{
	return RpgSharedAnimationSkeleton(new RpgAnimationSkeleton(name));
}
