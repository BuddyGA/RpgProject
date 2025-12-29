#pragma once

#include "render/asset/RpgTexture.h"


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogEditor)



namespace RpgEditorImportTexture
{
	enum EType : uint8_t
	{
		BMP = 0,
		PNG,
		TGA,
		DDS,
		JPG,
		// ...
		MAX_COUNT
	};


	constexpr const char* EXTENSIONS[MAX_COUNT] =
	{
		".bmp",
		".png",
		".tga",
		".dds",
		".jpg",
	};


	extern RpgEditorImportTexture::EType GetSupportedFileType(const RpgFilePath& filePath) noexcept;


	inline bool IsFileSupported(const RpgFilePath& filePath) noexcept
	{
		return GetSupportedFileType(filePath) != MAX_COUNT;
	}

};



namespace RpgEditorImportModel
{
	enum EType : uint8_t
	{
		OBJ = 0,
		FBX,
		GLTF,
		GLB,
		// ...
		MAX_COUNT
	};


	constexpr const char* EXTENSIONS[MAX_COUNT] =
	{
		".obj",
		".fbx",
		".gltf",
		".glb"
	};


	extern RpgEditorImportModel::EType GetSupportedFileType(const RpgFilePath& filePath) noexcept;


	inline bool IsFileSupported(const RpgFilePath& filePath) noexcept
	{
		return GetSupportedFileType(filePath) != MAX_COUNT;
	}

};




struct RpgEditorImportSetting_Texture
{
	RpgFilePath SourceFilePath;
	RpgTextureFormat::EType Format{ RpgTextureFormat::NONE };
	bool bGenerateMipMaps{ false };
};



struct RpgEditorImportSetting_Model
{
	RpgFilePath SourceFilePath;
	float Scale{ 1.0f };
	bool bImportMaterialTexture{ false };
	bool bImportSkeleton{ false };
	bool bImportAnimation{ false };
	bool bGenerateTextureMipMaps{ false };
	bool bIgnoreTextureNormals{ false };
};



struct RpgEditorAssetFile
{
	RpgName Name;
	RpgStringID Path;
	RpgAssetFileType Type{ RpgAssetFileType::NONE };
};



struct RpgEditorAssetFolder
{
	RpgFilePath Path;
	RpgArray<RpgEditorAssetFolder> Subfolders;
	RpgArray<RpgEditorAssetFile> Files;
};
