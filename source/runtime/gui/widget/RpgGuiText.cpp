#include "RpgGuiText.h"
#include "render/RpgRenderer2D.h"



RpgGuiText::RpgGuiText(const RpgName& in_Name) noexcept
	: RpgGuiWidget(in_Name)
{
	BackgroundColor = RpgColor::BLACK_TRANSPARENT;
	Color = RpgColor::WHITE;
	bDirtyDimension = false;
}


RpgRectFloat RpgGuiText::UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept
{
	if (bDirtyDimension)
	{
		const RpgSharedFont& useFont = Font.IsValid() ? Font : RpgFont::s_GetDefault_Roboto();
		Dimension = useFont->CalculateTextDimension(*TextValue, TextValue.GetLength());
		bDirtyDimension = false;
	}

	return RpgGuiWidget::UpdateRect(context, canvasRect, offset);
}


void RpgGuiText::OnRender(RpgRenderer2D& renderer) const noexcept
{
	RpgGuiWidget::OnRender(renderer);

	if (TextValue.IsEmpty())
	{
		return;
	}

	renderer.AddText(*TextValue, TextValue.GetLength(), AbsoluteRect.GetPosition(), Color, Font);
}
