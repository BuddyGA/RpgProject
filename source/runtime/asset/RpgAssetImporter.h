#pragma once

#include "core/RpgFilePath.h"
#include "render/RpgModel.h"
#include "animation/RpgAnimationTypes.h"


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogAssetImporter)



enum class RpgAssetImportType : uint8_t
{
	NONE = 0,
	MODEL,
	TEXTURE
};



struct RpgAssetImportSetting_Texture
{
	RpgFilePath SourceFilePath;
	RpgTextureFormat::EType Format{ RpgTextureFormat::NONE };
	bool bGenerateMipMaps{ false };
};



struct RpgAssetImportSetting_Model
{
	RpgFilePath SourceFilePath;
	float Scale{ 1.0f };
	bool bImportMaterialTexture{ false };
	bool bImportSkeleton{ false };
	bool bImportAnimation{ false };
	bool bGenerateTextureMipMaps{ false };
	bool bIgnoreTextureNormals{ false };
};



extern class RpgAssetImporter* g_AssetImporter;

class RpgAssetImporter final
{
	RPG_NOCOPYMOVE(RpgAssetImporter)

public:
	RpgAssetImporter() noexcept;
	~RpgAssetImporter() noexcept;
	void Reset() noexcept;
	void ImportTexture(RpgSharedTexture2D& out_Texture, const RpgAssetImportSetting_Texture& setting) noexcept;
	void ImportModel(RpgArray<RpgSharedModel>& out_Models, RpgSharedAnimationSkeleton& out_Skeleton, RpgArray<RpgSharedAnimationClip>& out_Animations, const RpgAssetImportSetting_Model& setting) noexcept;


	inline RpgAssetImportType GetCurrentImportType() noexcept
	{
		return ImportingType;
	}


private:
	RpgAssetImportType ImportingType;

};
