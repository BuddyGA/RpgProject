#pragma once

/*
#include "core/RpgFilePath.h"
#include "core/RpgPointer.h"
#include "core/RpgStream.h"


// Asset file extension
#define RPG_ASSET_FILE_EXT						".rpga"

// Magic number for asset file header
#define RPG_ASSET_FILE_MAGIX					0x41475052 // (RPGA)


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogAsset)



enum class RpgAssetFileType : uint16_t
{
	NONE = 0,

	MESH,
	TEXTURE,
	FONT,
	MATERIAL,
	AUDIO,
	LEVEL,

	MAX_COUNT
};

constexpr const char* RPG_ASSET_FILE_TYPE_NAMES[] =
{
	"None",
	"Mesh",
	"Texture",
	"Font",
	"Material",
	"Audio",
	"Level"
};

static_assert(sizeof(RPG_ASSET_FILE_TYPE_NAMES) / sizeof(const char*) == static_cast<uint16_t>(RpgAssetFileType::MAX_COUNT), "Not equals!");



struct RpgAssetFileHeader
{
	uint32_t Magix{ 0 };
	uint16_t Type{ 0 };
	uint16_t Version{ 0 };
	uint32_t DataSizeBytes{ 0 };
};
static_assert(std::is_trivially_copyable<RpgAssetFileHeader>::value, "RpgAssetFileHeader must be POD!");



struct RpgAssetInfo
{
	// Asset path (relative to asset directory)
	RpgString Path;

	// Asset type
	RpgAssetFileType Type;
};



namespace RpgAssetFileImage
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



class RpgAssetInterface
{
	RPG_NOCOPY(RpgAssetInterface)

public:
	RpgAssetInterface(const RpgName& in_Name) noexcept
		: Name(in_Name)
	{
	}

	virtual ~RpgAssetInterface() noexcept = default;

	virtual void StreamWrite(RpgStreamWriter& writer) const noexcept = 0;
	virtual void StreamRead(RpgStreamReader& reader) noexcept = 0;


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}


private:
	RpgName Name;

};


#define RPG_ASSET_FILE(type, version)				\
public:												\
static constexpr RpgAssetFileType FILE_TYPE = type;	\
static constexpr uint16_t FILE_VERSION = version;
*/
