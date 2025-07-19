#pragma once

#include "RpgFilePath.h"


// Magic number for asset file header
#define RPG_ASSET_FILE_MAGIX					0x41475052 // (RPGA)

// Model asset version
#define RPG_ASSET_FILE_VERSION_MODEL			1

// Texture asset version
#define RPG_ASSET_FILE_VERSION_TEXTURE			1

// Font asset version
#define RPG_ASSET_FILE_VERSION_FONT				1

// Material asset version
#define RPG_ASSET_FILE_VERSION_MATERIAL			1

// Animation skeleton asset version
#define RPG_ASSET_FILE_VERSION_ANIM_SKELETON	1

// Animation clip asset version
#define RPG_ASSET_FILE_VERSION_ANIM_CLIP		1

// Audio asset version
#define RPG_ASSET_FILE_VERSION_AUDIO			1




enum class RpgAssetFileType : uint16_t
{
	NONE = 0,

	MODEL,
	TEXTURE,
	FONT,
	MATERIAL,
	SKELETON,
	ANIM_CLIP,
	AUDIO,

	MAX_COUNT
};



struct RpgAssetFileHeader
{
	uint32_t Magix{ 0 };
	uint32_t SizeBytes{ 0 };
	uint16_t Type{ 0 };
	uint16_t Version{ 0 };
};




namespace RpgAssetFileImage
{
	enum EType : uint8_t
	{
		BMP = 0,
		PNG,
		DDS,
		TGA,
		JPG,
		// ...
		MAX_COUNT
	};


	constexpr const char* EXTENSIONS[MAX_COUNT] =
	{
		".bmp",
		".png",
		".dds",
		".tga",
		".jpg",
	};


	extern RpgAssetFileImage::EType GetSupportedFileType(const RpgFilePath& filePath) noexcept;


	inline bool IsFileSupported(const RpgFilePath& filePath) noexcept
	{
		return GetSupportedFileType(filePath) != MAX_COUNT;
	}

};



namespace RpgAssetFileModel
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


	extern RpgAssetFileModel::EType GetSupportedFileType(const RpgFilePath& filePath) noexcept;


	inline bool IsFileSupported(const RpgFilePath& filePath) noexcept
	{
		return GetSupportedFileType(filePath) != MAX_COUNT;
	}

};
