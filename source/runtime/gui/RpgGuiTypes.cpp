#include "RpgGuiTypes.h"
#include "input/RpgInputManager.h"
#include "render/RpgRenderer2D.h"
#include <algorithm>


RPG_LOG_DEFINE_CATEGORY(RpgLogGui, VERBOSITY_DEBUG)



RpgPointFloat RpgGui::CalculateTextAlignmentPosition(const RpgRectFloat& rect, const RpgString& text, const RpgSharedFont& font, RpgGuiAlignHorizontal alignH, RpgGuiAlignVertical alignV) noexcept
{
	const RpgPointFloat rectDim = rect.GetDimension();
	const RpgPointFloat textDim = font->CalculateTextDimension(*text, text.GetLength());

	RpgPointFloat pos;

	switch (alignH)
	{
		case RpgGuiAlignHorizontal::LEFT: pos.X = rect.Left; break;
		case RpgGuiAlignHorizontal::CENTER: pos.X = rect.Left + (rectDim.X - textDim.X) * 0.5f; break;
		case RpgGuiAlignHorizontal::RIGHT: pos.X = rect.Right - textDim.X; break;
		default: break;
	}

	switch (alignV)
	{
		case RpgGuiAlignVertical::TOP: pos.Y = rect.Top; break;
		case RpgGuiAlignVertical::CENTER: pos.Y = rect.Top + (rectDim.Y - textDim.Y) * 0.5f; break;
		case RpgGuiAlignVertical::BOTTOM: pos.Y = rect.Bottom - textDim.Y; break;
		default: break;
	}

	return pos;
}
