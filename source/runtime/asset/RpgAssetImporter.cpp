#include "RpgAssetImporter.h"
#include "task/RpgAssetTask_ImportModel.h"
#include <compressonator.h>



RPG_LOG_DEFINE_CATEGORY(RpgLogAssetImporter, VERBOSITY_LOG)



RpgAssetImporter* g_AssetImporter = nullptr;

RpgAssetImporter::RpgAssetImporter() noexcept
{
	ImportingType = RpgAssetImportType::NONE;
	CMP_InitFramework();
}


RpgAssetImporter::~RpgAssetImporter() noexcept
{
}


void RpgAssetImporter::Reset() noexcept
{
	ImportingType = RpgAssetImportType::NONE;
}


void RpgAssetImporter::ImportTexture(RpgSharedTexture2D& out_Texture, const RpgAssetImportSetting_Texture& setting) noexcept
{
}


void RpgAssetImporter::ImportModel(RpgArray<RpgSharedModel>& out_Models, RpgSharedAnimationSkeleton& out_Skeleton, RpgArray<RpgSharedAnimationClip>& out_Animations, const RpgAssetImportSetting_Model& setting) noexcept
{
	RPG_IsMainThread();

	out_Models.Clear();
	out_Skeleton.Release();
	out_Animations.Clear();

	ImportingType = RpgAssetImportType::MODEL;

	RpgAssetTask_ImportModel task;
	task.Reset();
	task.SourceFilePath = setting.SourceFilePath;
	task.Scale = setting.Scale;
	task.bImportMaterialTexture = setting.bImportMaterialTexture;
	task.bImportSkeleton = setting.bImportSkeleton;
	task.bImportAnimation = setting.bImportAnimation;
	task.bGenerateTextureMipMaps = setting.bGenerateTextureMipMaps;
	task.bIgnoreTextureNormals = setting.bIgnoreTextureNormals;
	task.Execute();

	out_Models = task.GetImportedModels();

	if (setting.bImportSkeleton)
	{
		out_Skeleton = task.GetImportedSkeleton();
	}

	if (setting.bImportAnimation)
	{
		out_Animations = task.GetImportedAnimations();
	}
}
