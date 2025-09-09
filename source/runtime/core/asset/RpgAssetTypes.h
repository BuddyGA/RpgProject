#pragma once

#include "../RpgFilePath.h"
#include "../RpgPointer.h"
#include "../RpgStream.h"
#include "../RpgSet.h"


// Asset file extension
#define RPG_ASSET_FILE_EXT						".rpga"

// Magic number for asset file header
#define RPG_ASSET_FILE_HEADER					0x41475052U // (RPGA)

// Magic number for external asset 
#define RPG_ASSET_FILE_ASSET_EXT				0x45475052U // (RPGE)

// Magic number for data asset
#define RPG_ASSET_FILE_ASSET_DATA				0x44475052U // (RPGD)

// Magic number for end of file
#define RPG_ASSET_FILE_EOF						0x41475052U // (RPGA)



RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogAsset)


class RpgAssetObject;
class RpgAssetSystem;
class RpgAssetTask_Loader;

typedef RpgSet<RpgString, 8> RpgAssetReferences;


extern RpgAssetSystem* g_AssetSystem;



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
	uint16_t AssetReferenceCount{ 0 };
	uint32_t AssetReferenceSizeBytes{ 0 };
	uint32_t AssetDataSizeBytes{ 0 };
	RpgName AssetClassName;
};
static_assert(std::is_trivially_copyable<RpgAssetFileHeader>::value, "RpgAssetFileHeader must be POD!");




struct RpgAssetInfo
{
	// Asset class name
	RpgName ClassName;

	// Asset path
	RpgString Path;

	// Asset type
	RpgAssetFileType Type{ RpgAssetFileType::NONE };


	inline bool IsValid() const noexcept
	{
		return !ClassName.IsEmpty() && !Path.IsEmpty() && Type != RpgAssetFileType::NONE;
	}

};




typedef RpgSharedPtr<RpgAssetObject> RpgSharedAsset;

class RpgAssetObject
{
	RPG_NOCOPY(RpgAssetObject)

public:
	RpgAssetObject(const RpgName& in_Name) noexcept
		: Name(in_Name)
	{
	}

	virtual ~RpgAssetObject() noexcept = default;


	virtual void AssetStreamWrite(RpgStreamWriter& writer) noexcept = 0;
	virtual void AssetStreamRead(RpgStreamReader& reader, uint16_t version) noexcept = 0;
	virtual bool IsAssetLoaded() noexcept = 0;
	virtual void GetExternalAssetReferences(RpgAssetReferences& out_AssetRefs) noexcept {}


	virtual RpgName GetAssetClassName() const noexcept
	{
		return "RpgAssetObject";
	}

	virtual RpgAssetFileType GetAssetFileType() const noexcept
	{
		return RpgAssetFileType::NONE;
	}

	virtual uint16_t GetAssetFileVersion() const noexcept
	{
		return 1;
	}

	inline const RpgName& GetAssetName() const noexcept
	{
		return Name;
	}

	inline const RpgString& GetAssetPath() const noexcept
	{
		return Path;
	}


protected:
	virtual RpgAssetObject* CreateAsset() const noexcept = 0;
	virtual void SetAssetLoading() noexcept = 0;


private:
	RpgName Name;
	RpgString Path;


	friend RpgAssetSystem;

};



namespace RpgAssetStream
{
	extern RpgFilePath Write(RpgAssetObject* asset, const char* dstFolder) noexcept;
	extern void Read(RpgAssetObject* asset) noexcept;

};




#define RPG_ASSET_CLASS(classType, fileType, fileVersion)													\
public:																										\
	static constexpr RpgAssetFileType FILE_TYPE = fileType;													\
	static constexpr uint16_t FILE_VERSION = fileVersion;													\
public:																										\
	static const classType* GetDefault() noexcept															\
	{																										\
		static classType defaultObject(#classType##"__default");											\
		return &defaultObject;																				\
	}																										\
public:																										\
	virtual RpgAssetObject* CreateAsset() const noexcept override { return new classType(#classType); }		\
	virtual RpgName GetAssetClassName() const noexcept override { return #classType; }						\
	virtual RpgAssetFileType GetAssetFileType() const noexcept override	{ return FILE_TYPE;	}				\
	virtual uint16_t GetAssetFileVersion() const noexcept override { return FILE_VERSION; }
