#pragma once

#include "asset/RpgAssetTypes.h"
#include "RpgRenderTypes.h"


// Minimum texture dimension
#define RPG_TEXTURE_MIN_DIM		64

// Maxiimum texture dimension
#define RPG_TEXTURE_MAX_DIM		4096

// Maximum texture mip count
#define RPG_TEXTURE_MAX_MIP		13



class RpgTexture2D;
typedef RpgSharedPtr<RpgTexture2D> RpgSharedTexture2D;

class RpgTextureRenderTarget;
typedef RpgSharedPtr<RpgTextureRenderTarget> RpgSharedTextureRenderTarget;

class RpgTextureDepthStencil;
typedef RpgSharedPtr<RpgTextureDepthStencil> RpgSharedTextureDepthStencil;

class RpgTextureDepthCube;
typedef RpgSharedPtr<class RpgTextureDepthCube> RpgSharedTextureDepthCube;




class RpgTexture2D : public RpgAssetInterface
{
	RPG_ASSET_FILE(RpgAssetFileType::TEXTURE, 1)

public:
	struct FMipData
	{
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Subresource{};
		int RowCount{ 0 };
		uint32_t RowBytes{ 0 };
		uint32_t SizeBytes{ 0 };
		uint16_t Width{ 0 };
		uint16_t Height{ 0 };
	};


public:
	RpgTexture2D() noexcept;
	RpgTexture2D(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height, uint8_t mipCount) noexcept;
	~RpgTexture2D() noexcept;

	virtual uint32_t CalculateAssetDataSizeBytes() const noexcept override;
	virtual void StreamWrite(RpgStreamWriter& writer) const noexcept override;
	virtual void StreamRead(RpgStreamReader& reader) noexcept override;


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}

	inline RpgTextureFormat::EType GetFormat() const noexcept
	{
		return Format;
	}

	inline RpgPointInt GetDimension() const noexcept
	{
		return RpgPointInt(static_cast<int>(Width), static_cast<int>(Height));
	}

	inline uint8_t GetMipCount() const noexcept
	{
		return MipCount;
	}

	inline size_t GetPixelSizeBytes() const noexcept
	{
		return PixelSizeBytes;
	}


	inline void Resize(uint16_t newWidth, uint16_t newHeight) noexcept
	{
		RPG_Check(newWidth >= RPG_TEXTURE_MIN_DIM && newWidth <= RPG_TEXTURE_MAX_DIM);
		RPG_Check(newHeight >= RPG_TEXTURE_MIN_DIM && newHeight <= RPG_TEXTURE_MAX_DIM);
		RPG_Check((Flags & (FLAG_RenderTarget | FLAG_DepthStencil)));

		Width = newWidth;
		Height = newHeight;
	}


	inline const uint8_t* MipReadLock(uint8_t mipLevel, FMipData& out_MipData) const noexcept
	{
		RPG_Check(mipLevel >= 0 && mipLevel < MipCount);
		RPG_Check(!(Flags & (FLAG_RenderTarget | FLAG_DepthStencil)));

		AcquireSRWLockShared(&MipLocks[mipLevel]);
		out_MipData = MipDatas[mipLevel];

		return PixelData + out_MipData.Subresource.Offset;
	}

	inline void MipReadUnlock(uint8_t mipLevel) noexcept
	{
		RPG_Check(mipLevel >= 0 && mipLevel < MipCount);
		RPG_Check(!(Flags & (FLAG_RenderTarget | FLAG_DepthStencil)));

		ReleaseSRWLockShared(&MipLocks[mipLevel]);
	}


	inline uint8_t* MipWriteLock(uint8_t mipLevel, FMipData& out_MipData) noexcept
	{
		RPG_Check(mipLevel >= 0 && mipLevel < MipCount);
		RPG_Check(!(Flags & (FLAG_RenderTarget | FLAG_DepthStencil)));

		AcquireSRWLockExclusive(&MipLocks[mipLevel]);
		out_MipData = MipDatas[mipLevel];
		Flags |= FLAG_Runtime_Dirty;

		return PixelData + out_MipData.Subresource.Offset;
	}

	inline void MipWriteUnlock(uint8_t mipLevel) noexcept
	{
		RPG_Check(mipLevel >= 0 && mipLevel < MipCount);
		RPG_Check(!(Flags & (FLAG_RenderTarget | FLAG_DepthStencil)));

		ReleaseSRWLockExclusive(&MipLocks[mipLevel]);
	}


	inline bool IsDirty() const noexcept
	{
		return (Flags & FLAG_Runtime_Dirty);
	}

	inline bool IsRenderTarget() const noexcept
	{
		return (Flags & FLAG_RenderTarget);
	}

	inline bool IsDepthStencil() const noexcept
	{
		return (Flags & FLAG_DepthStencil);
	}


	virtual void GPU_UpdateResource() noexcept;
	void GPU_CommandCopy(ID3D12GraphicsCommandList* cmdList) const noexcept;


	inline ID3D12Resource* GPU_GetResource() const noexcept
	{
		return GpuAlloc->GetResource();
	}

	inline D3D12_RESOURCE_STATES GPU_GetState() const noexcept
	{
		return GpuState;
	}

	inline void GPU_SetState(D3D12_RESOURCE_STATES newState) noexcept
	{
		GpuState = newState;
	}

	inline void GPU_SetLoading() noexcept
	{
		RPG_Check(!IsRenderTarget() && !IsDepthStencil());
		RPG_Check(Flags & FLAG_Runtime_Dirty);

		Flags = (Flags & ~FLAG_Runtime_GPU_Loaded) | FLAG_Runtime_GPU_Loading;
	}

	inline bool GPU_IsLoading() const noexcept
	{
		return (Flags & FLAG_Runtime_GPU_Loading);
	}

	inline void GPU_SetLoaded() noexcept
	{
		RPG_Check(!IsRenderTarget() && !IsDepthStencil());
		RPG_Check(Flags & FLAG_Runtime_GPU_Loading);

		Flags = (Flags & ~(FLAG_Runtime_Dirty | FLAG_Runtime_GPU_Loading)) | FLAG_Runtime_GPU_Loaded;
	}

	inline bool GPU_IsLoaded() const noexcept
	{
		return (Flags & FLAG_Runtime_GPU_Loaded);
	}


private:
	void InitializeMips() noexcept;


protected:
	RpgName Name;


	enum EFlag : uint16_t
	{
		FLAG_None						= (0),
		FLAG_RenderTarget				= (1 << 0),
		FLAG_DepthStencil				= (1 << 1),
		FLAG_Runtime_Loading			= (1 << 2),
		FLAG_Runtime_Loaded				= (1 << 3),
		FLAG_Runtime_PendingDestroy		= (1 << 4),
		FLAG_Runtime_Dirty				= (1 << 5),
		FLAG_Runtime_GPU_Loading		= (1 << 6),
		FLAG_Runtime_GPU_Loaded			= (1 << 7),
	};
	uint16_t Flags;

	inline static constexpr uint16_t RUNTIME_FLAGS =
		FLAG_Runtime_Loading |
		FLAG_Runtime_Loaded |
		FLAG_Runtime_PendingDestroy |
		FLAG_Runtime_Dirty |
		FLAG_Runtime_GPU_Loading |
		FLAG_Runtime_GPU_Loaded;


	RpgTextureFormat::EType Format;
	uint16_t Width;
	uint16_t Height;
	uint8_t MipCount;
	size_t PixelSizeBytes;
	uint8_t* PixelData;
	RpgArrayInline<FMipData, RPG_TEXTURE_MAX_MIP> MipDatas;
	mutable RpgArrayInline<SRWLOCK, RPG_TEXTURE_MAX_MIP> MipLocks;
	ComPtr<D3D12MA::Allocation> PixelStagingBuffer;
	ComPtr<D3D12MA::Allocation> GpuAlloc;
	D3D12_RESOURCE_STATES GpuState;


public:
	static void s_CreateDefaults() noexcept;
	static void s_DestroyDefaults() noexcept;

	static const RpgSharedTexture2D& s_GetDefault_White() noexcept;

};




class RpgTextureRenderTarget : public RpgTexture2D
{
public:
	RpgTextureRenderTarget(const RpgName& in_Name, RpgTextureFormat::EType in_Format, uint16_t in_Width, uint16_t in_Height) noexcept;

};




class RpgTextureDepthStencil : public RpgTexture2D
{
public:
	RpgTextureDepthStencil(const RpgName& in_Name, RpgTextureFormat::EType in_Format, uint16_t in_Width, uint16_t in_Height) noexcept;

};




class RpgTextureDepthCube : public RpgTextureDepthStencil
{
public:
	RpgTextureDepthCube(const RpgName& in_Name, RpgTextureFormat::EType in_Format, uint16_t in_Width, uint16_t in_Height) noexcept;

	virtual void GPU_UpdateResource() noexcept override;

};
