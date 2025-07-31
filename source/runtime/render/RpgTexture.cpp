#include "RpgTexture.h"
#include "core/RpgMath.h"


RPG_LOG_DECLARE_CATEGORY_STATIC(RpgLogTexture, VERBOSITY_DEBUG)



const DXGI_FORMAT RPG_TEXTURE_FORMAT_TO_DXGI_FORMAT[static_cast<uint8_t>(RpgTextureFormat::MAX_COUNT)] =
{
	DXGI_FORMAT_UNKNOWN,				// NONE = 0,
	DXGI_FORMAT_R8_UNORM,				// TEX_2D_R,
	DXGI_FORMAT_R8G8_UNORM,				// TEX_2D_RG,
	DXGI_FORMAT_R8G8B8A8_UNORM,			// TEX_2D_RGBA,
	DXGI_FORMAT_BC3_UNORM,				// TEX_2D_BC3U,
	DXGI_FORMAT_BC4_UNORM,				// TEX_2D_BC4U,
	DXGI_FORMAT_BC5_SNORM,				// TEX_2D_BC5S,
	DXGI_FORMAT_BC6H_UF16,				// TEX_2D_BC6H,
	DXGI_FORMAT_BC7_UNORM,				// TEX_2D_BC7U,
	DXGI_FORMAT_R8G8B8A8_UNORM,			// TEX_RT_RGBA,
	DXGI_FORMAT_B8G8R8A8_UNORM,			// TEX_RT_BGRA,
	DXGI_FORMAT_D16_UNORM,				// TEX_DS_16,
	DXGI_FORMAT_D24_UNORM_S8_UINT,		// TEX_DS_24_8,
	DXGI_FORMAT_D32_FLOAT,				// TEX_DS_32,
};



RpgTexture2D::RpgTexture2D(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height, uint8_t mipCount, uint16_t flags) noexcept
	: Format(RpgTextureFormat::NONE)
	, Width(0)
	, Height(0)
	, MipCount(0)
	, Flags(FLAG_None)
	, GpuState(D3D12_RESOURCE_STATE_COMMON)
{
	RPG_LogDebug(RpgLogTexture, "Create texture (%s)", *name);

	RPG_Check(!name.IsEmpty());
	Name = name;

	if (flags & FLAG_IsRenderTarget)
	{
		RPG_Check(format >= RpgTextureFormat::TEX_RT_RGBA && format <= RpgTextureFormat::TEX_RT_BGRA);
	}
	else if (flags & FLAG_IsDepthStencil)
	{
		RPG_Check(format >= RpgTextureFormat::TEX_DS_16 && format <= RpgTextureFormat::TEX_DS_32);
	}
	else
	{
		RPG_Check(format >= RpgTextureFormat::TEX_2D_R && format <= RpgTextureFormat::TEX_2D_BC7U);
	}

	Format = format;

	//RPG_PLATFORM_Check(width >= RPG_TEXTURE_MIN_DIM && width <= RPG_TEXTURE_MAX_DIM);
	Width = width;

	//RPG_PLATFORM_Check(height >= RPG_TEXTURE_MIN_DIM && height <= RPG_TEXTURE_MAX_DIM);
	Height = height;

	RPG_Check(mipCount > 0 && mipCount <= RPG_TEXTURE_MAX_MIP);
	MipCount = mipCount;

	TotalSizeBytes = 0;
	Flags = flags;

	PixelData = nullptr;

	if (!(Flags & (FLAG_IsRenderTarget | FLAG_IsDepthStencil)))
	{
		InitializeMips();
	}
}


RpgTexture2D::~RpgTexture2D() noexcept
{
	RPG_LogDebug(RpgLogTexture, "Destroy texture (%s)", *Name);

	if (PixelStagingBuffer && PixelData)
	{
		RpgD3D12::UnmapBuffer(PixelStagingBuffer.Get());
		PixelData = nullptr;
	}
}


void RpgTexture2D::GPU_UpdateResource() noexcept
{
	const bool bIsRenderTarget = (Flags & FLAG_IsRenderTarget);
	const bool bIsDepthStencil = (Flags & FLAG_IsDepthStencil);
	bool bShouldCreateNew = (GpuAlloc == nullptr);

	if (!bShouldCreateNew && (bIsRenderTarget || bIsDepthStencil))
	{
		RPG_Assert(GpuAlloc);

		const D3D12_RESOURCE_DESC desc = GpuAlloc->GetResource()->GetDesc();
		bShouldCreateNew = (Width != desc.Width || Height != desc.Height);
	}

	if (bShouldCreateNew)
	{
		const DXGI_FORMAT dxgiFormat = RPG_TEXTURE_FORMAT_TO_DXGI_FORMAT[static_cast<uint8_t>(Format)];
		GpuState = D3D12_RESOURCE_STATE_COMMON;

		if (bIsRenderTarget)
		{
			GpuAlloc = RpgD3D12::CreateRenderTarget(dxgiFormat, GpuState, Width, Height, RpgColorLinear(0.0f, 0.0f, 0.0f, 1.0f));
		}
		else if (bIsDepthStencil)
		{
			GpuAlloc = RpgD3D12::CreateDepthStencil(dxgiFormat, GpuState, Width, Height, 1.0f, 0);
		}
		else
		{
			GpuAlloc = RpgD3D12::CreateTexture2D(dxgiFormat, GpuState, Width, Height, MipCount);
		}

		RPG_D3D12_SetDebugNameAllocation(GpuAlloc, "RES_%s", *Name);
	}
}


void RpgTexture2D::GPU_CommandCopy(ID3D12GraphicsCommandList* cmdList) const noexcept
{
	ID3D12Resource* textureResource = GpuAlloc->GetResource();
	ID3D12Resource* stagingResource = PixelStagingBuffer->GetResource();

	for (int m = 0; m < static_cast<int>(MipCount); ++m)
	{
		AcquireSRWLockShared(&MipLocks[m]);
		{
			const FMipData& mipData = MipDatas[m];

			D3D12_TEXTURE_COPY_LOCATION dstCopy{};
			dstCopy.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dstCopy.pResource = textureResource;
			dstCopy.SubresourceIndex = m;

			D3D12_TEXTURE_COPY_LOCATION srcCopy{};
			srcCopy.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			srcCopy.pResource = stagingResource;
			srcCopy.PlacedFootprint = mipData.Subresource;
			srcCopy.PlacedFootprint.Offset = mipData.Subresource.Offset;

			cmdList->CopyTextureRegion(&dstCopy, 0, 0, 0, &srcCopy, nullptr);
		}
		ReleaseSRWLockShared(&MipLocks[m]);
	}
}


void RpgTexture2D::InitializeMips() noexcept
{
	RPG_Check(MipCount > 0 && MipCount <= RPG_TEXTURE_MAX_MIP);

	const D3D12_RESOURCE_DESC textureDesc = RpgD3D12::CreateResourceDesc_Texture(RPG_TEXTURE_FORMAT_TO_DXGI_FORMAT[static_cast<uint8_t>(Format)], Width, Height, MipCount);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT subresources[RPG_TEXTURE_MAX_MIP];
	uint32_t numRows[RPG_TEXTURE_MAX_MIP];
	uint64_t rowBytes[RPG_TEXTURE_MAX_MIP];
	RpgD3D12::GetDevice()->GetCopyableFootprints(&textureDesc, 0, MipCount, 0, subresources, numRows, rowBytes, &TotalSizeBytes);
	RPG_Check(TotalSizeBytes <= RPG_MAX_COUNT);
	//PixelData.Resize(static_cast<int>(totalSizeBytes));

	if (PixelData && PixelStagingBuffer)
	{
		RpgD3D12::UnmapBuffer(PixelStagingBuffer.Get());
		PixelData = nullptr;
	}

	RpgD3D12::ResizeBuffer(PixelStagingBuffer, TotalSizeBytes, true);
	RPG_D3D12_SetDebugNameAllocation(PixelStagingBuffer, "STG_%s", *Name);

	PixelData = RpgD3D12::MapBuffer<uint8_t>(PixelStagingBuffer.Get());

	MipDatas.Resize(MipCount);
	MipLocks.Resize(MipCount);

	for (int m = 0; m < MipCount; ++m)
	{
		FMipData& mip = MipDatas[m];
		mip.Subresource = subresources[m];
		mip.RowCount = numRows[m];
		mip.RowBytes = static_cast<uint32_t>(rowBytes[m]);
		mip.SizeBytes = static_cast<uint32_t>(mip.RowCount * mip.RowBytes);
		mip.Width = RpgMath::Max(Width >> m, 1);
		mip.Height = RpgMath::Max(Height >> m, 1);

		InitializeSRWLock(&MipLocks[m]);
	}
}



static RpgArray<RpgSharedTexture2D> DefaultTextures;


RpgSharedTexture2D RpgTexture2D::s_CreateShared2D(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height, uint8_t mipCount) noexcept
{
	return RpgSharedTexture2D(new RpgTexture2D(name, format, width, height, mipCount, 0));
}


RpgSharedTexture2D RpgTexture2D::s_CreateSharedRenderTarget(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height) noexcept
{
	return RpgSharedTexture2D(new RpgTexture2D(name, format, width, height, 1, FLAG_IsRenderTarget));
}


RpgSharedTexture2D RpgTexture2D::s_CreateSharedDepthStencil(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height) noexcept
{
	return RpgSharedTexture2D(new RpgTexture2D(name, format, width, height, 1, FLAG_IsDepthStencil));
}


void RpgTexture2D::s_CreateDefaults() noexcept
{
	RPG_LogDebug(RpgLogTexture, "Create default textures...");

	// texture2d-white
	{
		RpgSharedTexture2D defWhite = s_CreateShared2D("TEX2D_DEF_White", RpgTextureFormat::TEX_2D_RGBA, RPG_TEXTURE_MIN_DIM, RPG_TEXTURE_MIN_DIM, 1);
		
		FMipData mipData;
		uint8_t* pixelData = defWhite->MipWriteLock(0, mipData);
		RpgPlatformMemory::MemSet(pixelData, 255, mipData.SizeBytes);
		defWhite->MipWriteUnlock(0);

		DefaultTextures.AddValue(defWhite);
	}
}


void RpgTexture2D::s_DestroyDefaults() noexcept
{
	DefaultTextures.Clear(true);
}


const RpgSharedTexture2D& RpgTexture2D::s_GetDefault_White() noexcept
{
	return DefaultTextures[0];
}





RpgTextureDepthCube::RpgTextureDepthCube(const RpgName name, RpgTextureFormat::EType format, uint16_t width, uint16_t height) noexcept
	: RpgTexture2D(name, format, width, height, 1, FLAG_IsDepthStencil)
{
}


void RpgTextureDepthCube::GPU_UpdateResource() noexcept
{
	bool bShouldCreateNew = (GpuAlloc == nullptr);

	if (!bShouldCreateNew)
	{
		RPG_Assert(GpuAlloc);

		const D3D12_RESOURCE_DESC desc = GpuAlloc->GetResource()->GetDesc();
		bShouldCreateNew = (Width != desc.Width || Height != desc.Height);
	}

	if (bShouldCreateNew)
	{
		const DXGI_FORMAT dxgiFormat = RPG_TEXTURE_FORMAT_TO_DXGI_FORMAT[static_cast<uint8_t>(Format)];
		GpuState = D3D12_RESOURCE_STATE_COMMON;
		GpuAlloc = RpgD3D12::CreateDepthCube(dxgiFormat, GpuState, Width, Height);

		RPG_D3D12_SetDebugNameAllocation(GpuAlloc, "RES_%s", *Name);
	}
}



RpgSharedTextureDepthCube RpgTextureDepthCube::s_CreateShared(const RpgName& name, RpgTextureFormat::EType format, uint16_t width, uint16_t height) noexcept
{
	return RpgSharedTextureDepthCube(new RpgTextureDepthCube(name, format, width, height));
}
