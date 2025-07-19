#include "RpgAnimationTypes.h"



void RpgAnimationPose::UpdateBonePoseTransforms(const RpgAnimationSkeleton* skeleton) noexcept
{
	const int boneCount = skeleton->GetBoneCount();

	RpgArrayInline<int, RPG_SKELETON_MAX_BONE> updateBoneIndices;
	for (int b = 0; b < boneCount; ++b)
	{
		if (BoneDirtyTransforms[b])
		{
			updateBoneIndices.AddValue(b);
		}
	}

	// Zeroed dirty transfrom values
	RpgPlatformMemory::MemZero(BoneDirtyTransforms.GetData(), BoneDirtyTransforms.GetMemorySizeBytes_Allocated());

	for (int i = 0; i < updateBoneIndices.GetCount(); ++i)
	{
		const int boneIndex = updateBoneIndices[i];
		const int boneParentIndex = skeleton->GetBoneParentIndex(boneIndex);
		RPG_Check(boneParentIndex == RPG_SKELETON_BONE_INDEX_INVALID || boneParentIndex < boneIndex);

		BonePoseTransforms[boneIndex] = (boneParentIndex != RPG_SKELETON_BONE_INDEX_INVALID) ? BoneLocalTransforms[boneIndex] * BonePoseTransforms[boneParentIndex] : BoneLocalTransforms[boneIndex];
	}
}
