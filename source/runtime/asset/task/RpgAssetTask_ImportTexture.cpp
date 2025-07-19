#include "RpgAssetTask_ImportTexture.h"
#include "core/RpgMath.h"
#include "../RpgAssetImporter.h"
#include "thirdparty/stb/stb_image.h"
#include <compressonator.h>



namespace RpgCompressonator
{
	[[nodiscard]] inline static CMP_FORMAT To_CMP_FORMAT(RpgTextureFormat::EType format) noexcept
	{
		switch (format)
		{
			case RpgTextureFormat::TEX_2D_RGBA: return CMP_FORMAT_RGBA_8888;
			case RpgTextureFormat::TEX_2D_BC3U: return CMP_FORMAT_BC3;
			case RpgTextureFormat::TEX_2D_BC4U: return CMP_FORMAT_BC4;
			case RpgTextureFormat::TEX_2D_BC5S: return CMP_FORMAT_BC5_S;
			case RpgTextureFormat::TEX_2D_BC6H: return CMP_FORMAT_BC6H;
			case RpgTextureFormat::TEX_2D_BC7U: return CMP_FORMAT_BC7;

			default:
				RPG_NotImplementedYet();
				break;
		}

		return CMP_FORMAT_Unknown;
	}


	static void ImportFromFile(RpgSharedTexture2D& out_Texture, const RpgFilePath& filePath, RpgTextureFormat::EType format, bool bGenerateMipMaps) noexcept
	{
		CMP_MipSet srcMipSet{};
		CMP_ERROR cmpStatus = CMP_LoadTexture(*filePath, &srcMipSet);

		if (cmpStatus != CMP_OK)
		{
			RPG_LogError(RpgLogAssetImporter, "Fail to import texture from source file (%s). Load file data failed!", *filePath);
			return;
		}

		if (bGenerateMipMaps && srcMipSet.m_nMipLevels <= 1)
		{
			if (!(RpgMath::IsPowerOfTwo(srcMipSet.m_nWidth) && RpgMath::IsPowerOfTwo(srcMipSet.m_nHeight)))
			{
				RPG_LogError(RpgLogAssetImporter, "Fail to import texture from source file (%s). If generate mip-maps, width and height must be power of two! (W: %i, H: %i)", *filePath, srcMipSet.m_nWidth, srcMipSet.m_nHeight);
				CMP_FreeMipSet(&srcMipSet);

				return;
			}

			const CMP_INT maxMipLevels = CMP_CalcMaxMipLevel(srcMipSet.m_nHeight, srcMipSet.m_nWidth, false);
			RPG_Check(srcMipSet.m_nMaxMipLevels == maxMipLevels);

			const CMP_INT minSize = CMP_CalcMinMipSize(srcMipSet.m_nHeight, srcMipSet.m_nWidth, maxMipLevels);
			const CMP_INT status = CMP_GenerateMIPLevels(&srcMipSet, minSize);
			RPG_Check(status == 0);
		}


		// Compress texture?
		CMP_MipSet compressedMipSet{};
		const bool bCompressTexture = (format >= RpgTextureFormat::TEX_2D_BC3U && format <= RpgTextureFormat::TEX_2D_BC7U);

		if (bCompressTexture)
		{
			KernelOptions kernelOptions{};
			//kernelOptions.encodeWith = CMP_GPU_DXC;
			kernelOptions.threads = 0;
			kernelOptions.format = To_CMP_FORMAT(format);
			kernelOptions.fquality = 0.5f;

			cmpStatus = CMP_ProcessTexture(&srcMipSet, &compressedMipSet, kernelOptions, nullptr);

			if (cmpStatus != CMP_OK)
			{
				RPG_LogError(RpgLogAssetImporter, "Fail to import texture from source file (%s). Process texture compression failed!", *filePath);
				CMP_FreeMipSet(&srcMipSet);

				return;
			}
		}


		// Result
		CMP_MipSet* mipSet = bCompressTexture ? &compressedMipSet : &srcMipSet;
		RPG_Check(mipSet);

		out_Texture = RpgTexture2D::s_CreateShared2D(filePath.GetFileName(), format, mipSet->m_nWidth, mipSet->m_nHeight, mipSet->m_nMipLevels);
		RPG_Check(out_Texture->GetMipCount() == mipSet->m_nMipLevels);

		for (int m = 0; m < mipSet->m_nMipLevels; ++m)
		{
			const CMP_MipLevel* srcMipData = mipSet->m_pMipLevelTable[m];

			RpgTexture2D::FMipData dstMipData;
			uint8_t* dstPixelData = out_Texture->MipWriteLock(m, dstMipData);
			{
				RPG_Check(srcMipData->m_nWidth == dstMipData.Width);
				RPG_Check(srcMipData->m_nHeight == dstMipData.Height);
				RPG_Check(srcMipData->m_dwLinearSize == dstMipData.SizeBytes);

				for (int r = 0; r < dstMipData.RowCount; ++r)
				{
					const size_t dstOffset = static_cast<size_t>(r * dstMipData.Subresource.Footprint.RowPitch);
					const size_t srcOffset = static_cast<size_t>(r * dstMipData.RowBytes);
					RpgPlatformMemory::MemCopy(dstPixelData + dstOffset, srcMipData->m_pbData + srcOffset, dstMipData.RowBytes);
				}
			}
			out_Texture->MipWriteUnlock(m);
		}


		// Free compressonator mip sets
		if (bCompressTexture)
		{
			CMP_FreeMipSet(&compressedMipSet);
		}

		CMP_FreeMipSet(&srcMipSet);
	}


	static void ImportFromEmbedded(RpgSharedTexture2D& out_Texture, const RpgAssimp::FTextureEmbedded& embedded, RpgTextureFormat::EType format, bool bGenerateMipMaps) noexcept
	{
		RPG_Check(embedded.Data);
		RPG_Check(embedded.Width > 0);
		RPG_Check(embedded.Height == 0);

		RpgName extension;
		const int sizeBytes = embedded.Height > 0 ? embedded.Width * embedded.Height : embedded.Width;

		if (embedded.Height == 0)
		{
			// bmp
			if (embedded.FormatHint[0] == 'b' && embedded.FormatHint[1] == 'm' && embedded.FormatHint[2] == 'p')
			{
				extension = ".bmp";
			}
			// jpg
			else if (embedded.FormatHint[0] == 'j' && embedded.FormatHint[1] == 'p' && embedded.FormatHint[2] == 'g')
			{
				extension = ".jpg";
			}
			// png
			else if (embedded.FormatHint[0] == 'p' && embedded.FormatHint[1] == 'n' && embedded.FormatHint[2] == 'g')
			{
				extension = ".png";
			}
			else
			{
				RPG_NotImplementedYet();
			}

			const RpgFilePath sourceFilePath = RpgString::Format("%s__temp/%s", *RpgFileSystem::GetProjectDirPath(), *embedded.Name);
			{
				HANDLE fileHandle = RpgPlatformFile::FileOpen(*sourceFilePath, RpgPlatformFile::OPEN_MODE_WRITE_OVERWRITE);

				if (RpgPlatformFile::FileWrite(fileHandle, embedded.Data, sizeBytes))
				{
					ImportFromFile(out_Texture, sourceFilePath, format, bGenerateMipMaps);
					RpgPlatformFile::FileDelete(*sourceFilePath);
				}
				else
				{
					RPG_LogError(RpgLogAssetImporter, "Fail to import texture from embedded data (%s). Write file failed!", *embedded.Name);
				}

				RpgPlatformFile::FileClose(fileHandle);
			}

			return;
		}
		

		/*
		int width = 0;
		int height = 0;
		int channels = 0;

		stbi_uc* pixelData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(embedded.Data), sizeBytes, &width, &height, &channels, 4);

		if (pixelData == nullptr)
		{
			RPG_PLATFORM_LogError(RpgLogAssetImporter, "Fail to import texture from embedded data (%s). Load data failed!", *embedded.Name);
			return;
		}

		CMP_Texture srcTexture{};
		srcTexture.dwSize = sizeof(CMP_Texture);
		srcTexture.dwWidth = width;
		srcTexture.dwHeight = height;
		srcTexture.dwPitch = 0;
		srcTexture.format = CMP_FORMAT_RGBA_8888;
		srcTexture.dwDataSize = CMP_CalculateBufferSize(&srcTexture);
		srcTexture.pData = pixelData;

		CMP_MipSet srcMipSet{};
		srcMipSet.m_format = srcTexture.format;
		srcMipSet.dwWidth = srcTexture.dwWidth;
		srcMipSet.dwHeight = srcTexture.dwHeight;
		srcMipSet.pData = srcTexture.pData;
		CMP_ERROR cmpStatus = CMP_CreateMipSet(&srcMipSet, srcTexture.dwWidth, srcTexture.dwHeight, 1, ChannelFormat::CF_8bit, TextureType::TT_2D);
		RPG_PLATFORM_Check(cmpStatus == CMP_OK);
		

		if (bGenerateMipMaps && srcMipSet.m_nMipLevels <= 1)
		{
			if (!(RpgMath::IsPowerOfTwo(srcMipSet.m_nWidth) && RpgMath::IsPowerOfTwo(srcMipSet.m_nHeight)))
			{
				RPG_PLATFORM_LogError(RpgLogAssetImporter, "Fail to import texture from source file (%s). If generate mip-maps, width and height must be power of two! [W: %i, H: %i]", 
					*embedded.Name, srcMipSet.m_nWidth, srcMipSet.m_nHeight
				);

				CMP_FreeMipSet(&srcMipSet);
				stbi_image_free(pixelData);

				return;
			}

			const CMP_INT maxMipLevels = CMP_CalcMaxMipLevel(srcMipSet.m_nHeight, srcMipSet.m_nWidth, false);
			RPG_PLATFORM_Check(srcMipSet.m_nMaxMipLevels == maxMipLevels);

			const CMP_INT minSize = CMP_CalcMinMipSize(srcMipSet.m_nHeight, srcMipSet.m_nWidth, maxMipLevels);
			const CMP_INT status = CMP_GenerateMIPLevels(&srcMipSet, minSize);
			RPG_PLATFORM_Check(status == 0);
		}

		// Compress texture?
		CMP_Texture compressedTexture{};
		const bool bCompressTexture = (format >= RpgTextureFormat::TEX_2D_BC3 && format <= RpgTextureFormat::TEX_2D_BC7);

		if (bCompressTexture)
		{
			compressedTexture.dwSize = sizeof(CMP_Texture);
			compressedTexture.dwWidth = srcTexture.dwWidth;
			compressedTexture.dwHeight = srcTexture.dwHeight;
			compressedTexture.dwPitch = 0;
			compressedTexture.format = To_CMP_FORMAT(format);
			compressedTexture.dwDataSize = CMP_CalculateBufferSize(&compressedTexture);
			compressedTexture.pData = reinterpret_cast<CMP_BYTE*>(RpgPlatformMemory::MemMalloc(compressedTexture.dwDataSize));

			CMP_CompressOptions options = { 0 };
			options.dwSize = sizeof(options);
			options.fquality = 0.5f;
			options.dwnumThreads = 4;
			options.genGPUMipMaps = true;

			CMP_ERROR cmpError = CMP_ConvertTexture(&srcTexture, &compressedTexture, &options, nullptr);
			if (cmpError != CMP_OK)
			{
				RPG_PLATFORM_LogError(RpgLogAssetImporter, "Fail to import texture from embedded data [%s]. Compress texture failed!", *embedded.Name);
				return;
			}
		}

		CMP_Texture* dstTexture = bCompressTexture ? &compressedTexture : &srcTexture;
		RPG_PLATFORM_Check(dstTexture);

		out_Texture = RpgTexture2D::s_CreateShared2D(embedded.Name, format, width, height, 1);

		RpgTexture2D::FMipData dstMipData;
		uint8_t* dstPixelData = out_Texture->MipWriteLock(0, dstMipData);
		{
			RPG_PLATFORM_Check(dstTexture->dwWidth == dstMipData.Width);
			RPG_PLATFORM_Check(dstTexture->dwHeight == dstMipData.Height);
			RPG_PLATFORM_Check(dstTexture->dwDataSize == dstMipData.SizeBytes);

			for (int r = 0; r < dstMipData.RowCount; ++r)
			{
				const size_t dstOffset = static_cast<size_t>(r * dstMipData.Subresource.Footprint.RowPitch);
				const size_t srcOffset = static_cast<size_t>(r * dstMipData.RowBytes);
				RpgPlatformMemory::MemCopy(dstPixelData + dstOffset, dstTexture->pData + srcOffset, dstMipData.RowBytes);
			}
		}
		out_Texture->MipReadWriteUnlock(0);
		

		// Free allocs
		if (compressedTexture.pData)
		{
			RpgPlatformMemory::MemFree(compressedTexture.pData);
			compressedTexture.pData = nullptr;
		}

		stbi_image_free(pixelData);
		*/
	}

};


RpgAssetTask_ImportTexture::RpgAssetTask_ImportTexture() noexcept
{
	Format = RpgTextureFormat::TEX_2D_RGBA;
	bGenerateMipMaps = false;
}


void RpgAssetTask_ImportTexture::Reset() noexcept
{
	RpgThreadTask::Reset();

	SourceEmbedded = RpgAssimp::FTextureEmbedded();
	SourceFilePath.Clear();
	bGenerateMipMaps = false;
	Format = RpgTextureFormat::TEX_2D_RGBA;
	Result.Release();
}


void RpgAssetTask_ImportTexture::Execute() noexcept
{
	RPG_Check(SourceFilePath.IsFilePath() || SourceEmbedded.Data);

	if (SourceFilePath.IsFilePath())
	{
		RPG_Log(RpgLogAssetImporter, "[Thread-%u]: Import texture from file (%s)", GetCurrentThreadId(), *SourceFilePath);
		RpgCompressonator::ImportFromFile(Result, SourceFilePath, Format, bGenerateMipMaps);
	}
	else
	{
		RPG_Log(RpgLogAssetImporter, "[Thread-%u]: Import texture from embedded data (%s)", GetCurrentThreadId(), *SourceEmbedded.Name);
		RpgCompressonator::ImportFromEmbedded(Result, SourceEmbedded, Format, bGenerateMipMaps);
	}

	if (Result)
	{
		RPG_Log(RpgLogAssetImporter, "[Thread-%u]: Import texture SUCCESS!\n"
			"\tSourceFile: %s\n"
			"\tTexture: %s\n"
			"\tFormat: %s\n"
			"\tMipCount: %u\n",
			GetCurrentThreadId(),
			*SourceFilePath,
			*Result->GetName(),
			RpgTextureFormat::NAMES[Result->GetFormat()],
			Result->GetMipCount()
		);
	}
}
