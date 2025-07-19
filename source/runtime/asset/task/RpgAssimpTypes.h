#pragma once

#include "render/RpgModel.h"
#include "animation/RpgAnimationTypes.h"



// Forward declare assimp types
struct aiScene;
struct aiNode;
struct aiMesh;



namespace RpgAssimp
{
	struct FTextureEmbedded
	{
		RpgName Name;
		void* Data{ nullptr };
		int Width{ 0 };
		int Height{ 0 };
		char FormatHint[12];


		FTextureEmbedded() noexcept
			: Name("")
			, Data(nullptr)
			, Width(0)
			, Height(0)
			, FormatHint()
		{
		}


		FTextureEmbedded(const RpgName& in_Name, void* in_Data, int in_Width, int in_Height, const char* in_FormatHint) noexcept
			: FTextureEmbedded()
		{
			Name = in_Name;
			Data = in_Data;
			Width = in_Width;
			Height = in_Height;
			RpgPlatformString::CStringCopy(FormatHint, in_FormatHint);
		}

	};


	struct FMaterialPhong
	{
		RpgName Name;
		RpgVector4 ParamVectorBaseColor;
		RpgVector4 ParamVectorSpecularColor;
		float ParamScalarShininess{ 0.0f };
		float ParamScalarOpacity{ 0.0f };
		int TextureIndexBaseColor{ RPG_INDEX_INVALID };
		int TextureIndexNormal{ RPG_INDEX_INVALID };
		int TextureIndexSpecular{ RPG_INDEX_INVALID };
	};


	struct FSkeleton
	{
		RpgArray<RpgName> BoneNames;
		RpgArray<int> BoneParentIndices;
		RpgArray<RpgMatrixTransform> BoneLocalTransforms;
		RpgArray<RpgMatrixTransform> BoneInverseBindPoseTransforms;


		inline void Clear(bool bFreeMemory = false) noexcept
		{
			BoneNames.Clear(bFreeMemory);
			BoneParentIndices.Clear(bFreeMemory);
			BoneLocalTransforms.Clear(bFreeMemory);
			BoneInverseBindPoseTransforms.Clear(bFreeMemory);
		}

		[[nodiscard]] inline int GetBoneIndex(const RpgName& boneName) const noexcept
		{
			const int index = BoneNames.FindIndexByValue(boneName);
			return (index == RPG_INDEX_INVALID) ? RPG_SKELETON_BONE_INDEX_INVALID : index;
		}
	};


	struct FModel
	{
		RpgName Name;
		RpgArray<RpgSharedMesh> Meshes;
		RpgArrayInline<int, RPG_MODEL_MAX_MESH> Materials;
	};

};
