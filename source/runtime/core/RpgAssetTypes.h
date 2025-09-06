#pragma once

#include "RpgFilePath.h"
#include "RpgPointer.h"
#include "RpgStream.h"


// Asset file extension
#define RPG_ASSET_FILE_EXT						".rpga"

// Magic number for asset file header
#define RPG_ASSET_FILE_MAGIX					0x41475052 // (RPGA)


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogAsset)


extern class RpgAssetSystem* g_AssetSystem;



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




class RpgAssetObject
{
	RPG_NOCOPY(RpgAssetObject)

public:
	RpgAssetObject(const RpgName& in_Name) noexcept
		: Name(in_Name)
	{
	}

	virtual ~RpgAssetObject() noexcept = default;

	virtual void StreamWrite(RpgStreamWriter& writer) const noexcept
	{
		writer.Write(Name);
	}

	virtual void StreamRead(RpgStreamReader& reader) noexcept
	{
		reader.Read(Name);
	}


	inline const RpgName& GetAssetName() const noexcept
	{
		return Name;
	}

	inline const RpgString& GetAssetPath() const noexcept
	{
		return Path;
	}


private:
	RpgName Name;
	RpgString Path;


	friend RpgAssetSystem;

};

typedef RpgSharedPtr<RpgAssetObject> RpgSharedAsset;



#define RPG_ASSET_FILE(type, version)				\
public:												\
static constexpr RpgAssetFileType FILE_TYPE = type;	\
static constexpr uint16_t FILE_VERSION = version;
