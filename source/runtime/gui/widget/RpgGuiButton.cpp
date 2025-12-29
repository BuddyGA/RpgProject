#include "RpgGuiButton.h"
#include "render/RpgRenderer2D.h"



RpgGuiButton::RpgGuiButton(const RpgName& in_Name) noexcept
	: RpgGuiWidget(in_Name)
{
}


RpgGuiButton::RpgGuiButton(const RpgName& in_Name, RpgPointFloat in_Dimension) noexcept
	: RpgGuiButton(in_Name)
{
	Dimension = in_Dimension;
}


void RpgGuiButton::OnUpdate(RpgGuiContext& context, RpgGuiWidget* parentLayout) noexcept
{
	if (IsReleased())
	{
		EventPressed.Broadcast(this);
	}
}


void RpgGuiButton::OnRender(RpgRenderer2D& renderer) const noexcept
{
	RpgGuiWidget::OnRender(renderer);

	RpgColor color = BackgroundColor;

	if (IsHovered())
	{
		color = RpgColor(60, 80, 100);
	}

	renderer.AddMeshRect(AbsoluteRect, color);
}
