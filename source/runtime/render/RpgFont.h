#pragma once

#include "core/RpgVertex.h"
#include "RpgTexture.h"


// Font size small
#define RPG_FONT_SIZE_SMALL		10

// Font size medium
#define RPG_FONT_SIZE_MEDIUM	15

// Font size large
#define RPG_FONT_SIZE_LARGE		20

// Font tab indent pixel size
#define RPG_FONT_TAB_INDENT_PX	16.0f



struct RpgFontUnicodeRange
{
	void* PackedChars{ nullptr };
	int CodeStart{ 0 };
	int CodeCount{ 0 };
};


struct RpgFontMetric
{
	float ScalePx{ 0.0f };
	float SizePx{ 0.0f };
	float Ascent{ 0.0f };
	float Descent{ 0.0f };
	float LineSpace{ 0.0f };
};



typedef RpgSharedPtr<class RpgFont> RpgSharedFont;

class RpgFont
{
	RPG_NOCOPY(RpgFont)

public:
	RpgFont(const RpgName& in_Name, const RpgString& ttfFilePath, float in_SizePx, int in_UnicodeStart, int in_UnicodeCount) noexcept;
	~RpgFont() noexcept;

	RpgPointFloat CalculateTextDimension(const char* text, int length) const noexcept;
	RpgPointFloat CalculateTextCursorPosition(const char* text, int length, const RpgPointFloat& textPosition, int cursorIndex) const noexcept;
	RpgRectFloat CalculateTextSelectionRect(const char* text, int length, const RpgPointFloat& textPosition, int selectIndex, int selectCount) const noexcept;
	int GenerateTextVertex(const char* text, int length, RpgPointFloat textPosition, RpgColorRGBA color, RpgVertexMesh2DArray& out_Vertexes, RpgVertexIndexArray& out_Indexes, int* optOut_VertexCount = nullptr, int* optOut_IndexCount = nullptr) const noexcept;


	inline const RpgFontUnicodeRange& GetUnicodeRange() const noexcept
	{
		return UnicodeRange;
	}

	inline RpgFontMetric GetMetric() const noexcept
	{
		return Metric;
	}

	inline float GetPixelHeight() const noexcept
	{
		return Metric.Ascent - Metric.Descent;
	}

	inline const RpgSharedTexture2D& GetTexture() const noexcept
	{
		return Texture;
	}


private:
	RpgName Name;
	RpgFontUnicodeRange UnicodeRange;
	RpgFontMetric Metric;
	RpgSharedTexture2D Texture;


public:
	[[nodiscard]] static RpgSharedFont s_CreateShared(const RpgName& name, const RpgString& ttfFilePath, float sizePx, int unicodeStart, int unicodeCount) noexcept;

	static void s_CreateDefaults() noexcept;
	static void s_DestroyDefaults() noexcept;

	static const RpgSharedFont& s_GetDefault_Roboto() noexcept;
	static const RpgSharedFont& s_GetDefault_ShareTechMono() noexcept;

};
