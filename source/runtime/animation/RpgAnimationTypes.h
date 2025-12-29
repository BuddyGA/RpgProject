#pragma once

#include "core/RpgMath.h"
#include "core/RpgString.h"
#include "core/RpgPointer.h"


// Maximum bone count in skeleton
#define RPG_SKELETON_MAX_BONE				254

// Invalid skeleton bone index
#define RPG_SKELETON_BONE_INDEX_INVALID		255

// Maximum number of vertex a bone can influences
#define RPG_SKELETON_BONE_MAX_VERTEX		8



RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogAnimation)


class RpgAnimationSkeleton;
class RpgAnimationPose;
class RpgAnimationClip;
class RpgAnimationWorldSubsystem;
class RpgAnimationTask_TickPose;



// ======================================================================================================================= //
// ANIMATION POSE
// ======================================================================================================================= //
class RpgAnimationPose
{
public:
	RpgAnimationPose() noexcept = default;


	RpgAnimationPose(const RpgAnimationPose& other) noexcept
		: BoneLocalTransforms(other.BoneLocalTransforms)
		, BonePoseTransforms(other.BonePoseTransforms)
		, BoneDirtyTransforms(other.BoneDirtyTransforms)
	{
	}


	RpgAnimationPose(RpgAnimationPose&& other) noexcept
		: BoneLocalTransforms(other.BoneLocalTransforms)
		, BonePoseTransforms(other.BonePoseTransforms)
		, BoneDirtyTransforms(other.BoneDirtyTransforms)
	{
	}


public:
	inline RpgAnimationPose& operator=(const RpgAnimationPose& rhs) noexcept
	{
		if (this != &rhs)
		{
			BoneLocalTransforms = rhs.BoneLocalTransforms;
			BonePoseTransforms = rhs.BonePoseTransforms;
			BoneDirtyTransforms = rhs.BoneDirtyTransforms;
		}

		return *this;
	}

	inline RpgAnimationPose& operator=(RpgAnimationPose&& rhs) noexcept
	{
		if (this != &rhs)
		{
			BoneLocalTransforms = std::move(rhs.BoneLocalTransforms);
			BonePoseTransforms = std::move(rhs.BonePoseTransforms);
			BoneDirtyTransforms = std::move(rhs.BoneDirtyTransforms);
		}

		return *this;
	}


public:
	void UpdateBonePoseTransforms(const RpgAnimationSkeleton* skeleton) noexcept;


	inline void Clear(bool bFreeMemory = false) noexcept
	{
		BoneLocalTransforms.Clear(bFreeMemory);
		BonePoseTransforms.Clear(bFreeMemory);
		BoneDirtyTransforms.Clear(bFreeMemory);
	}


	inline int AddBone(const RpgMatrixTransform& localTransform) noexcept
	{
		const int boneIndex = BoneLocalTransforms.GetCount();

		BoneLocalTransforms.AddValue(localTransform);
		BonePoseTransforms.AddValue(RpgMatrixTransform());
		BoneDirtyTransforms.AddValue(1);

		return boneIndex;
	}


	inline void SetBoneLocalTransform(int boneIndex, const RpgMatrixTransform& transform) noexcept
	{
		BoneLocalTransforms[boneIndex] = transform;
		BoneDirtyTransforms[boneIndex] = 1;
	}


	inline const RpgMatrixTransform& GetBoneLocalTransform(int boneIndex) const noexcept
	{
		return BoneLocalTransforms[boneIndex];
	}

	inline const RpgArray<RpgMatrixTransform>& GetBoneLocalTransforms() const noexcept
	{
		return BoneLocalTransforms;
	}


	inline const RpgMatrixTransform& GetBonePoseTransform(int boneIndex) const noexcept
	{
		return BonePoseTransforms[boneIndex];
	}

	inline const RpgArray<RpgMatrixTransform>& GetBonePoseTransforms() const noexcept
	{
		return BonePoseTransforms;
	}


private:
	RpgArray<RpgMatrixTransform> BoneLocalTransforms;
	RpgArray<RpgMatrixTransform> BonePoseTransforms;
	RpgArray<uint8_t> BoneDirtyTransforms;

};




// ======================================================================================================================= //
// ANIMATION SKELETON
// ======================================================================================================================= //
typedef RpgSharedPtr<class RpgAnimationSkeleton> RpgSharedAnimationSkeleton;

class RpgAnimationSkeleton
{
	RPG_NOCOPY(RpgAnimationSkeleton)

private:
	RpgAnimationSkeleton(const RpgName& name) noexcept;

public:
	~RpgAnimationSkeleton() noexcept;

	void SaveToAssetFile(const RpgString& filePath) noexcept;
	void LoadFromAssetFile(const RpgString& filePath) noexcept;


	inline int AddBone(const RpgName& name, int parentIndex, const RpgMatrixTransform& boneLocalTransform, const RpgMatrixTransform& boneInverseBindPoseTransform) noexcept
	{
		const int index = BoneNames.GetCount();

		RPG_Check(index >= 0 && index < RPG_SKELETON_MAX_BONE);
		RPG_Check(parentIndex == RPG_SKELETON_BONE_INDEX_INVALID || (parentIndex >= 0 && parentIndex < RPG_SKELETON_MAX_BONE));
		RPG_Check(index != parentIndex);

		BoneNames.AddValue(name);
		BoneParentIndices.AddValue(parentIndex);
		BoneInverseBindPoseTransforms.AddValue(boneInverseBindPoseTransform);
		BindPose.AddBone(boneLocalTransform);

		return index;
	}


	inline void UpdateBindPoseTransforms() noexcept
	{
		BindPose.UpdateBonePoseTransforms(this);
	}

	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}

	inline int GetBoneCount() const noexcept
	{
		return BoneNames.GetCount();
	}

	inline int GetBoneIndex(const RpgName& name) const noexcept
	{
		const int index = BoneNames.FindIndexByValue(name);
		return (index == RPG_INDEX_INVALID) ? RPG_SKELETON_BONE_INDEX_INVALID : index;
	}

	inline const RpgName& GetBoneName(int boneIndex) const noexcept
	{
		RPG_CheckV(boneIndex >= 0 && boneIndex < BoneNames.GetCount(), "Invalid bone index (%i)", boneIndex);
		return BoneNames[boneIndex];
	}

	inline int GetBoneParentIndex(int boneIndex) const noexcept
	{
		RPG_CheckV(boneIndex >= 0 && boneIndex < BoneNames.GetCount(), "Invalid bone index (%i)", boneIndex);
		return BoneParentIndices[boneIndex];
	}

	inline const RpgArray<int>& GetBoneParentIndices() const noexcept
	{
		return BoneParentIndices;
	}


	inline const RpgMatrixTransform& GetBoneInverseBindPoseTransform(int boneIndex) const noexcept
	{
		RPG_CheckV(boneIndex >= 0 && boneIndex < BoneNames.GetCount(), "Invalid bone index (%i)", boneIndex);
		return BoneInverseBindPoseTransforms[boneIndex];
	}

	inline const RpgAnimationPose& GetBindPose() const noexcept
	{
		return BindPose;
	}


private:
	RpgName Name;
	RpgArray<RpgName> BoneNames;
	RpgArray<int> BoneParentIndices;
	RpgArray<RpgMatrixTransform> BoneInverseBindPoseTransforms;
	RpgAnimationPose BindPose;


public:
	[[nodiscard]] static RpgSharedAnimationSkeleton s_CreateShared(const RpgName& name) noexcept;

};




// ======================================================================================================================= //
// ANIMATION CLIP
// ======================================================================================================================= //
struct RpgAnimationTrack
{
	RpgName BoneName;


	struct FKeyPosition
	{
		RpgVector3 Value;
		float Timestamp{ 0.0f };
	};
	RpgArray<FKeyPosition> KeyPositions;


	struct FKeyRotation
	{
		RpgQuaternion Value;
		float Timestamp{ 0.0f };
	};
	RpgArray<FKeyRotation> KeyRotations;

};



typedef RpgSharedPtr<class RpgAnimationClip> RpgSharedAnimationClip;

class RpgAnimationClip
{
	RPG_NOCOPY(RpgAnimationClip)

private:
	RpgAnimationClip(const RpgName& in_Name, float in_DurationSeconds) noexcept;

public:
	~RpgAnimationClip() noexcept;

	void SaveToAssetFile(const RpgString& filePath) noexcept;
	void LoadFromAssetFile(const RpgString& filePath) noexcept;

	void AddTrack(const RpgAnimationTrack& in_Track) noexcept;

	// Returns TRUE if for each track/bone in anim clip contains bone name in skeleton
	bool CheckSkeletonCompatibility(const RpgAnimationSkeleton* skeleton) const noexcept;


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}

	inline float GetDurationSeconds() const noexcept
	{
		return DurationSeconds;
	}

	inline const RpgArray<RpgAnimationTrack>& GetTracks() const noexcept
	{
		return Tracks;
	}


private:
	// Clip name
	RpgName Name;

	// Animation duration in seconds
	float DurationSeconds;

	// Track for each bone
	RpgArray<RpgAnimationTrack> Tracks;


public:
	[[nodiscard]] static RpgSharedAnimationClip s_CreateShared(const RpgName& name, float durationSeconds) noexcept;

};
