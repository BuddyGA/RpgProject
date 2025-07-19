#include "RpgGuiButton.h"
#include "render/RpgRenderer2D.h"


RpgGuiButton::RpgGuiButton() noexcept
{
}


RpgGuiButton::RpgGuiButton(const RpgName& in_Name, RpgPointFloat in_Dimension) noexcept
	: RpgGuiButton()
{
	Name = in_Name;
	Dimension = in_Dimension;
}


void RpgGuiButton::OnUpdate(RpgGuiContext& context) noexcept
{
	if (IsReleased())
	{
		EventPressed.Broadcast();
	}
}


void RpgGuiButton::OnRender(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept
{
	RpgColorRGBA color = RpgColorRGBA(30, 40, 50);

	if (IsHovered())
	{
		color = RpgColorRGBA(60, 80, 100);
	}

	renderer.AddMeshRect(AbsoluteRect, color);
}
