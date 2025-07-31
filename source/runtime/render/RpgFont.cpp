#include "RpgFont.h"
#include "core/RpgFilePath.h"
#include "thirdparty/stb/stb_truetype.h"


RPG_LOG_DECLARE_CATEGORY_STATIC(RpgLogFont, VERBOSITY_DEBUG)


RpgFont::RpgFont(const RpgName& in_Name, const RpgString& ttfFilePath, float in_SizePx, int in_UnicodeStart, int in_UnicodeCount) noexcept
{
	RPG_LogDebug(RpgLogFont, "Create font (%s)", *in_Name);

	Name = in_Name;

	RpgArray<uint8_t> fileData;
	const bool bReadFileSuccess = RpgFileSystem::ReadFromFile(ttfFilePath, fileData);
	RPG_Check(bReadFileSuccess);

	stbtt_fontinfo stbFontInfo{};
	const int stbError = stbtt_InitFont(&stbFontInfo, fileData.GetData(), 0);
	RPG_Check(stbError != 0);

	// Init font datas
	RpgPointInt textureDimension(256);
	RpgArray<uint8_t> texturePixels(textureDimension.X * textureDimension.Y);

	UnicodeRange.CodeStart = in_UnicodeStart;
	UnicodeRange.CodeCount = in_UnicodeCount;
	UnicodeRange.PackedChars = RpgPlatformMemory::MemMalloc(sizeof(stbtt_packedchar) * UnicodeRange.CodeCount);

	Metric.ScalePx = stbtt_ScaleForPixelHeight(&stbFontInfo, in_SizePx);

	stbtt_pack_context stbPackContext{};
	{
		int res = stbtt_PackBegin(&stbPackContext, texturePixels.GetData(), textureDimension.X, textureDimension.Y, 0, 1, nullptr);
		RPG_Check(res == 1);

		// TODO: Resize texture dimension if packing failed (res == 0)

		stbtt_PackSetOversampling(&stbPackContext, 2, 2);

		res = stbtt_PackFontRange(&stbPackContext, fileData.GetData(), 0, in_SizePx, UnicodeRange.CodeStart, UnicodeRange.CodeCount, static_cast<stbtt_packedchar*>(UnicodeRange.PackedChars));
		RPG_Check(res == 1);

		stbtt_PackEnd(&stbPackContext);
	}

	int tempAscent, tempDescent, tempLineGap;
	stbtt_GetFontVMetrics(&stbFontInfo, &tempAscent, &tempDescent, &tempLineGap);

	Metric.SizePx = in_SizePx;
	Metric.Ascent = static_cast<float>(tempAscent) * Metric.ScalePx;
	Metric.Descent = static_cast<float>(tempDescent) * Metric.ScalePx;
	Metric.LineSpace = (Metric.Ascent - Metric.Descent) + static_cast<float>(tempLineGap) * Metric.ScalePx;

	Texture = RpgTexture2D::s_CreateShared2D(RpgName::Format("TEX2D_%s", *Name), RpgTextureFormat::TEX_2D_R, textureDimension.X, textureDimension.Y, 1);
	{
		RpgTexture2D::FMipData mipData;
		uint8_t* pixelData = Texture->MipWriteLock(0, mipData);
		RpgPlatformMemory::MemCopy(pixelData, texturePixels.GetData(), texturePixels.GetMemorySizeBytes_Allocated());
		Texture->MipWriteUnlock(0);
	}
}


RpgFont::~RpgFont() noexcept
{
	RPG_LogDebug(RpgLogFont, "Destroy font (%s)", *Name);

	RpgPlatformMemory::MemFree(UnicodeRange.PackedChars);
}


RpgPointFloat RpgFont::CalculateTextDimension(const char* text, int length) const noexcept
{
	if (text == nullptr || length == 0)
	{
		return RpgPointFloat();
	}

	const stbtt_packedchar* packedChars = static_cast<stbtt_packedchar*>(UnicodeRange.PackedChars);

	RpgPointFloat dim(0.0f, Metric.Ascent - Metric.Descent);
	float x = dim.X;
	float y = dim.Y;

	for (int i = 0; i < length; ++i)
	{
		char c = text[i];
		RPG_Check(c != '\0');

		if (c == '\n')
		{
			y += Metric.LineSpace;
			dim.X = RpgMath::Max(dim.X, x);
			x = 0;

			continue;
		}

		if (c == '\t')
		{
			x += RPG_FONT_TAB_INDENT_PX;
			continue;
		}

		x += packedChars[c - UnicodeRange.CodeStart].xadvance;
	}

	dim.X = RpgMath::Max(dim.X, x);
	dim.Y = RpgMath::Max(dim.Y, y);

	return dim;
}


RpgPointFloat RpgFont::CalculateTextCursorPosition(const char* text, int length, const RpgPointFloat& textPosition, int cursorIndex) const noexcept
{
	RpgPointFloat pos = textPosition;

	if (text == nullptr || length == 0 || cursorIndex == 0)
	{
		return pos;
	}

	RPG_Check(cursorIndex >= 0 && cursorIndex <= length);

	const stbtt_packedchar* packedChars = static_cast<stbtt_packedchar*>(UnicodeRange.PackedChars);
	float prevCharWidth = 0;

	for (int i = 0; i <= length; ++i)
	{
		if (i <= cursorIndex)
		{
			pos.X += prevCharWidth;
		}

		char c = text[i];

		if (c == '\t')
		{
			prevCharWidth = RPG_FONT_TAB_INDENT_PX;
			continue;
		}

		if (c == '\n')
		{
			pos.Y += Metric.LineSpace;
			continue;
		}

		if (i == length)
		{
			break;
		}

		prevCharWidth = packedChars[c - UnicodeRange.CodeStart].xadvance;
	}

	return pos;
}


RpgRectFloat RpgFont::CalculateTextSelectionRect(const char* text, int length, const RpgPointFloat& textPosition, int selectIndex, int selectCount) const noexcept
{
	RpgPointFloat pos = textPosition;

	if (text == nullptr || length == 0)
	{
		return RpgRectFloat(pos.X, pos.Y, pos.X, pos.Y);
	}

	RPG_Check(selectCount > 0);
	RPG_Check(selectIndex >= 0 && (selectIndex + selectCount) <= length);

	const stbtt_packedchar* packedChars = static_cast<stbtt_packedchar*>(UnicodeRange.PackedChars);
	const int lastIndex = selectIndex + selectCount;
	float prevCharWidth = 0.0f;
	
	RpgPointFloat dim;
	dim.Y = Metric.Ascent - Metric.Descent;

	for (int i = 0; i <= length; ++i)
	{
		if (i <= selectIndex)
		{
			pos.X += prevCharWidth;
		}
		else if (i <= lastIndex)
		{
			dim.X += prevCharWidth;
		}

		if (i == lastIndex)
		{
			break;
		}

		char c = text[i];

		if (c == '\t')
		{
			prevCharWidth = RPG_FONT_TAB_INDENT_PX;
			continue;
		}

		prevCharWidth = packedChars[c - UnicodeRange.CodeStart].xadvance;
	}

	return RpgRectFloat(pos.X, pos.Y, pos.X + dim.X, pos.Y + dim.Y);
}


int RpgFont::GenerateTextVertex(const char* text, int length, RpgPointFloat textPosition, RpgColorRGBA color, RpgVertexMesh2DArray& out_Vertexes, RpgVertexIndexArray& out_Indexes, int* optOut_VertexCount, int* optOut_IndexCount) const noexcept
{
	if (optOut_VertexCount)
	{
		*optOut_VertexCount = 0;
	}

	if (optOut_IndexCount)
	{
		*optOut_IndexCount = 0;
	}

	if (text == nullptr || length == 0)
	{
		return 0;
	}

	const stbtt_packedchar* packedChars = static_cast<stbtt_packedchar*>(UnicodeRange.PackedChars);
	const RpgPointInt texDim = Texture->GetDimension();

	float px = textPosition.X;
	float py = textPosition.Y;
	py += Metric.Ascent;

	uint32_t vertexCount = 0;
	int indexCount = 0;
	int charCount = 0;

	for (int i = 0; i < length; ++i)
	{
		char c = text[i];
		RPG_Assert(c);

		if (c == '\n')
		{
			px = textPosition.X;
			py += Metric.LineSpace;
			continue;
		}

		if (c == '\t')
		{
			px += RPG_FONT_TAB_INDENT_PX;
			continue;
		}

		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(packedChars, texDim.X, texDim.Y, c - UnicodeRange.CodeStart, &px, &py, &quad, 0);
		RPG_Assert(quad.x1 - quad.x0 >= 0.0f);
		RPG_Assert(quad.y1 - quad.y0 >= 0.0f);

		const RpgVertex::FMesh2D vertexes[4] =
		{
			{ DirectX::XMFLOAT2(quad.x0, quad.y0), DirectX::XMFLOAT2(quad.s0, quad.t0), color },
			{ DirectX::XMFLOAT2(quad.x1, quad.y0), DirectX::XMFLOAT2(quad.s1, quad.t0), color },
			{ DirectX::XMFLOAT2(quad.x1, quad.y1), DirectX::XMFLOAT2(quad.s1, quad.t1), color },
			{ DirectX::XMFLOAT2(quad.x0, quad.y1), DirectX::XMFLOAT2(quad.s0, quad.t1), color },
		};

		out_Vertexes.InsertAtRange(vertexes, 4, RPG_INDEX_LAST);

		const RpgVertex::FIndex indexes[6] =
		{
			vertexCount,
			vertexCount + 1,
			vertexCount + 2,
			vertexCount + 2,
			vertexCount + 3,
			vertexCount
		};

		out_Indexes.InsertAtRange(indexes, 6, RPG_INDEX_LAST);

		vertexCount += 4;
		indexCount += 6;
		++charCount;
	}

	if (optOut_VertexCount)
	{
		*optOut_VertexCount = static_cast<int>(vertexCount);
	}

	if (optOut_IndexCount)
	{
		*optOut_IndexCount = indexCount;
	}

	return charCount;
}





static RpgArray<RpgSharedFont> DefaultFonts;


RpgSharedFont RpgFont::s_CreateShared(const RpgName& name, const RpgString& ttfFilePath, float sizePx, int unicodeStart, int unicodeCount) noexcept
{
	return RpgSharedFont(new RpgFont(name, ttfFilePath, sizePx, unicodeStart, unicodeCount));
}


void RpgFont::s_CreateDefaults() noexcept
{
	RPG_LogDebug(RpgLogFont, "Create default fonts...");

	const RpgString fontAssetDirPath = RpgFileSystem::GetAssetRawDirPath() + "font/";

	DefaultFonts.AddValue(s_CreateShared("FNT_DEF_Roboto", fontAssetDirPath + "Roboto-Regular.ttf", RPG_FONT_SIZE_MEDIUM, 32, 96));
	DefaultFonts.AddValue(s_CreateShared("FNT_DEF_ShareTechMono", fontAssetDirPath + "ShareTechMono-Regular.ttf", RPG_FONT_SIZE_MEDIUM, 32, 96));
}


void RpgFont::s_DestroyDefaults() noexcept
{
	DefaultFonts.Clear(true);
}


const RpgSharedFont& RpgFont::s_GetDefault_Roboto() noexcept
{
	return DefaultFonts[0];
}


const RpgSharedFont& RpgFont::s_GetDefault_ShareTechMono() noexcept
{
	return DefaultFonts[1];
}
