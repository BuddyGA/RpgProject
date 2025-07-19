#pragma once

#include "core/RpgString.h"
#include "core/RpgPointer.h"
#include "RpgRenderTypes.h"


// Minimum texture dimension
#define RPG_TEXTURE_MIN_DIM		64

// Maxiimum texture dimension
#define RPG_TEXTURE_MAX_DIM		4096

// Maximum texture mip count
#define RPG_TEXTURE_MAX_MIP		13



typedef RpgSharedPtr<class RpgTexture2D> RpgSharedTexture2D;

class RpgTexture2D
{
	RPG_NOCOPY(RpgTexture2D)

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
	RpgTexture2D(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height, uint8_t mipCount, uint16_t flags) noexcept;
	~RpgTexture2D() noexcept;


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

	inline size_t GetTotalSizeBytes() const noexcept
	{
		return TotalSizeBytes;
	}


	inline void Resize(uint16_t newWidth, uint16_t newHeight) noexcept
	{
		RPG_Check(newWidth >= RPG_TEXTURE_MIN_DIM && newWidth <= RPG_TEXTURE_MAX_DIM);
		RPG_Check(newHeight >= RPG_TEXTURE_MIN_DIM && newHeight <= RPG_TEXTURE_MAX_DIM);
		RPG_Check((Flags & (FLAG_IsRenderTarget | FLAG_IsDepthStencil)));

		Width = newWidth;
		Height = newHeight;
	}


	inline const uint8_t* MipReadLock(uint8_t mipLevel, FMipData& out_MipData) const noexcept
	{
		RPG_Check(mipLevel >= 0 && mipLevel < MipCount);
		RPG_Check(!(Flags & (FLAG_IsRenderTarget | FLAG_IsDepthStencil)));

		AcquireSRWLockShared(&MipLocks[mipLevel]);
		out_MipData = MipDatas[mipLevel];

		return PixelData + out_MipData.Subresource.Offset;
	}

	inline void MipReadUnlock(uint8_t mipLevel) noexcept
	{
		RPG_Check(mipLevel >= 0 && mipLevel < MipCount);
		RPG_Check(!(Flags & (FLAG_IsRenderTarget | FLAG_IsDepthStencil)));

		ReleaseSRWLockShared(&MipLocks[mipLevel]);
	}


	inline uint8_t* MipWriteLock(uint8_t mipLevel, FMipData& out_MipData) noexcept
	{
		RPG_Check(mipLevel >= 0 && mipLevel < MipCount);
		RPG_Check(!(Flags & (FLAG_IsRenderTarget | FLAG_IsDepthStencil)));

		AcquireSRWLockExclusive(&MipLocks[mipLevel]);
		out_MipData = MipDatas[mipLevel];
		Flags |= FLAG_Dirty;

		return PixelData + out_MipData.Subresource.Offset;
	}

	inline void MipWriteUnlock(uint8_t mipLevel) noexcept
	{
		RPG_Check(mipLevel >= 0 && mipLevel < MipCount);
		RPG_Check(!(Flags & (FLAG_IsRenderTarget | FLAG_IsDepthStencil)));

		ReleaseSRWLockExclusive(&MipLocks[mipLevel]);
	}


	inline bool IsDirty() const noexcept
	{
		return (Flags & FLAG_Dirty);
	}

	inline bool IsRenderTarget() const noexcept
	{
		return (Flags & FLAG_IsRenderTarget);
	}

	inline bool IsDepthStencil() const noexcept
	{
		return (Flags & FLAG_IsDepthStencil);
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
		RPG_Check(Flags & FLAG_Dirty);

		Flags = (Flags & ~FLAG_GPU_Loaded) | FLAG_GPU_Loading;
	}

	inline bool GPU_IsLoading() const noexcept
	{
		return (Flags & FLAG_GPU_Loading);
	}

	inline void GPU_SetLoaded() noexcept
	{
		RPG_Check(!IsRenderTarget() && !IsDepthStencil());
		RPG_Check(Flags & FLAG_GPU_Loading);

		Flags = (Flags & ~(FLAG_Dirty | FLAG_GPU_Loading)) | FLAG_GPU_Loaded;
	}

	inline bool GPU_IsLoaded() const noexcept
	{
		return (Flags & FLAG_GPU_Loaded);
	}


private:
	void InitializeMips() noexcept;


protected:
	RpgName Name;
	RpgTextureFormat::EType Format;
	uint16_t Width;
	uint16_t Height;
	uint8_t MipCount;
	size_t TotalSizeBytes;
	RpgArrayInline<FMipData, RPG_TEXTURE_MAX_MIP> MipDatas;
	mutable RpgArrayInline<SRWLOCK, RPG_TEXTURE_MAX_MIP> MipLocks;
	ComPtr<D3D12MA::Allocation> PixelStagingBuffer;
	uint8_t* PixelData;


	enum EFlag : uint16_t
	{
		FLAG_None				= (0),
		FLAG_Loading			= (1 << 0),
		FLAG_Loaded				= (1 << 1),
		FLAG_PendingDestroy		= (1 << 2),
		FLAG_Dirty				= (1 << 3),
		FLAG_IsRenderTarget		= (1 << 4),
		FLAG_IsDepthStencil		= (1 << 5),
		FLAG_GPU_Loading		= (1 << 6),
		FLAG_GPU_Loaded			= (1 << 7),
	};
	uint16_t Flags;

	ComPtr<D3D12MA::Allocation> GpuAlloc;
	D3D12_RESOURCE_STATES GpuState;


public:
	[[nodiscard]] static RpgSharedTexture2D s_CreateShared2D(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height, uint8_t mipCount) noexcept;
	[[nodiscard]] static RpgSharedTexture2D s_CreateSharedRenderTarget(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height) noexcept;
	[[nodiscard]] static RpgSharedTexture2D s_CreateSharedDepthStencil(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height) noexcept;

	static void s_CreateDefaults() noexcept;
	static void s_DestroyDefaults() noexcept;

	static const RpgSharedTexture2D& s_GetDefault_White() noexcept;

};



typedef RpgSharedPtr<class RpgTextureDepthCube> RpgSharedTextureDepthCube;

class RpgTextureDepthCube : public RpgTexture2D
{
public:
	RpgTextureDepthCube(const RpgName name, RpgTextureFormat::EType format, uint16_t width, uint16_t height) noexcept;

	virtual void GPU_UpdateResource() noexcept override;


public:
	[[nodiscard]] static RpgSharedTextureDepthCube s_CreateShared(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height) noexcept;

};
