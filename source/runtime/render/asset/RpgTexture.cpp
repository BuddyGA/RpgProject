#include "RpgTexture.h"


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



RpgTexture2D::RpgTexture2D(const RpgName& in_Name) noexcept
	: RpgAssetObject(in_Name)
	, MipDatas()
	, MipLocks()
{
	Flags = FLAG_None;
	Format = RpgTextureFormat::NONE;
	Width = 0;
	Height = 0;
	MipCount = 1;
	PixelSizeBytes = 0;
	PixelData = nullptr;
	GpuState = D3D12_RESOURCE_STATE_COMMON;
}


RpgTexture2D::RpgTexture2D(const RpgName& in_Name, RpgTextureFormat::EType in_Format, uint16_t in_Width, uint16_t in_Height, uint8_t in_MipCount) noexcept
	: RpgTexture2D(in_Name)
{
	RPG_Check(in_Format >= RpgTextureFormat::TEX_2D_R && in_Format <= RpgTextureFormat::TEX_2D_BC7U);
	Format = in_Format;

	RPG_Check(in_Width >= RPG_TEXTURE_MIN_DIM && in_Width <= RPG_TEXTURE_MAX_DIM);
	Width = in_Width;

	RPG_Check(in_Height >= RPG_TEXTURE_MIN_DIM && in_Height <= RPG_TEXTURE_MAX_DIM);
	Height = in_Height;

	RPG_Check(in_MipCount > 0 && in_MipCount <= RPG_TEXTURE_MAX_MIP);
	MipCount = in_MipCount;

	InitializeMips();
}


RpgTexture2D::~RpgTexture2D() noexcept
{
	RPG_LogDebug(RpgLogTexture, "Destroy texture (%s)", *GetAssetName());

	if (PixelStagingBuffer && PixelData)
	{
		RpgD3D12::UnmapBuffer(PixelStagingBuffer.Get());
		PixelData = nullptr;
	}
}


void RpgTexture2D::AssetStreamWrite(RpgStreamWriter& writer) noexcept
{
	// only save non-runtime flags
	const uint16_t savedFlags = (Flags & ~RUNTIME_FLAGS);
	RPG_Check(!(savedFlags & (FLAG_RenderTarget | FLAG_DepthStencil)));
	writer.Write(savedFlags);

	writer.Write(Format);
	writer.Write(Width);
	writer.Write(Height);
	writer.Write(MipCount);
	writer.Write(PixelSizeBytes);
	writer.WriteData(PixelData, static_cast<uint32_t>(PixelSizeBytes));
}


void RpgTexture2D::AssetStreamRead(RpgStreamReader& reader, uint16_t version) noexcept
{
	reader.Read(Flags);
	reader.Read(Format);
	reader.Read(Width);
	reader.Read(Height);
	reader.Read(MipCount);
	reader.Read(PixelSizeBytes);

	InitializeMips();
	reader.ReadData(PixelData, static_cast<uint32_t>(PixelSizeBytes));

	Flags &= ~FLAG_Runtime_Loading;
	Flags |= FLAG_Runtime_Dirty | FLAG_Runtime_Loaded;
}


bool RpgTexture2D::IsAssetLoaded() noexcept
{
	return (Flags & FLAG_Runtime_Loaded);
}


void RpgTexture2D::SetAssetLoading() noexcept
{
	Flags |= FLAG_Runtime_Loading;
}


void RpgTexture2D::GPU_UpdateResource() noexcept
{
	const bool bIsRenderTarget = (Flags & FLAG_RenderTarget);
	const bool bIsDepthStencil = (Flags & FLAG_DepthStencil);
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

		RPG_D3D12_SetDebugNameAllocation(GpuAlloc, "RES_%s", *GetAssetName());
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
	RPG_Check(!(Flags & (FLAG_RenderTarget | FLAG_DepthStencil)));
	RPG_Check(MipCount > 0 && MipCount <= RPG_TEXTURE_MAX_MIP);

	const D3D12_RESOURCE_DESC textureDesc = RpgD3D12::CreateResourceDesc_Texture(RPG_TEXTURE_FORMAT_TO_DXGI_FORMAT[static_cast<uint8_t>(Format)], Width, Height, MipCount);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT subresources[RPG_TEXTURE_MAX_MIP];
	uint32_t numRows[RPG_TEXTURE_MAX_MIP];
	uint64_t rowBytes[RPG_TEXTURE_MAX_MIP];
	RpgD3D12::GetDevice()->GetCopyableFootprints(&textureDesc, 0, MipCount, 0, subresources, numRows, rowBytes, &PixelSizeBytes);
	RPG_Check(PixelSizeBytes <= RPG_MAX_COUNT);

	if (PixelData && PixelStagingBuffer)
	{
		RpgD3D12::UnmapBuffer(PixelStagingBuffer.Get());
		PixelData = nullptr;
	}

	RpgD3D12::ResizeBuffer(PixelStagingBuffer, PixelSizeBytes, true);
	RPG_D3D12_SetDebugNameAllocation(PixelStagingBuffer, "STG_%s", *GetAssetName());

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

		const uint16_t checkWidth = Width >> m;
		mip.Width = (checkWidth > 1) ? checkWidth : 1;

		const uint16_t checkHeight = Height >> m;
		mip.Height = (checkHeight > 1) ? checkHeight : 1;

		InitializeSRWLock(&MipLocks[m]);
	}
}



static RpgArray<RpgSharedTexture2D> DefaultTextures;


void RpgTexture2D::CreateDefaults() noexcept
{
	RPG_LogDebug(RpgLogTexture, "Create default textures...");

	// texture2d-white
	{
		RpgSharedTexture2D defWhite = RpgPointer::MakeShared<RpgTexture2D>("tex2d_def_white", RpgTextureFormat::TEX_2D_RGBA, RPG_TEXTURE_MIN_DIM, RPG_TEXTURE_MIN_DIM, 1);
		
		FMipData mipData;
		uint8_t* pixelData = defWhite->MipWriteLock(0, mipData);
		RpgPlatformMemory::Set(pixelData, 255, mipData.SizeBytes);
		defWhite->MipWriteUnlock(0);

		DefaultTextures.AddValue(defWhite);
	}
}


void RpgTexture2D::DestroyDefaults() noexcept
{
	DefaultTextures.Clear(true);
}


const RpgSharedTexture2D& RpgTexture2D::GetDefault_White() noexcept
{
	return DefaultTextures[0];
}



RpgTextureRenderTarget::RpgTextureRenderTarget(const RpgName& in_Name, RpgTextureFormat::EType in_Format, uint16_t in_Width, uint16_t in_Height) noexcept
	: RpgTexture2D(in_Name)
{
	Flags = FLAG_RenderTarget | FLAG_Runtime_Loaded;

	RPG_Check(in_Format >= RpgTextureFormat::TEX_RT_RGBA && in_Format <= RpgTextureFormat::TEX_RT_BGRA);
	Format = in_Format;

	Width = in_Width;
	Height = in_Height;
}




RpgTextureDepthStencil::RpgTextureDepthStencil(const RpgName& in_Name, RpgTextureFormat::EType in_Format, uint16_t in_Width, uint16_t in_Height) noexcept
	: RpgTexture2D(in_Name)
{
	Flags = FLAG_DepthStencil | FLAG_Runtime_Loaded;

	RPG_Check(in_Format >= RpgTextureFormat::TEX_DS_16 && in_Format <= RpgTextureFormat::TEX_DS_32);
	Format = in_Format;

	Width = in_Width;
	Height = in_Height;
}



RpgTextureDepthCube::RpgTextureDepthCube(const RpgName& in_Name, RpgTextureFormat::EType in_Format, uint16_t in_Width, uint16_t in_Height) noexcept
	: RpgTextureDepthStencil(in_Name, in_Format, in_Width, in_Height)
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

		RPG_D3D12_SetDebugNameAllocation(GpuAlloc, "RES_%s", *GetAssetName());
	}
}
