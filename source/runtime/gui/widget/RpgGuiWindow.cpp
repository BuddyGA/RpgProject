#include "RpgGuiWindow.h"
#include "render/RpgRenderer2D.h"



RpgGuiWindow::RpgGuiWindow(const RpgName& in_Name) noexcept
	: RpgGuiWidget(in_Name)
{
	Flags = RpgGui::FLAG_Layout;
	Position = RpgPointFloat(16.0f, 16.0f);
	Dimension = RpgPointFloat(512.0f, 512.0f);
	BackgroundColor = RpgColor::BLACK_TRANSPARENT;
	Order = RPG_GUI_ORDER_WINDOW_DEFAULT;
	
	BorderThickness = 5.0f;
	BorderColor = RpgColor(20, 30, 50);
	TitleHeight = 24.0f;
	ContentBackgroundColor = RpgColor(10, 10, 10, 220);
	ContentChildPadding = RpgRectFloat(4.0f);
	ContentChildSpace = RpgPointFloat(4.0f);
	ContentDirection = RpgGuiLayout::DIRECTION_NONE;
	TitleButton = nullptr;
	TitleText = nullptr;
	LayoutContent = nullptr;

	SetVisibility(false);
}


void RpgGuiWindow::Initialize() noexcept
{
	TitleButton = AddChild<RpgGuiButton>(RpgName::Format("%s/title_button", *Name));
	TitleButton->BackgroundColor = BorderColor;

	TitleText = AddChild<RpgGuiText>(RpgName::Format("%s/title_text", *Name));

	LayoutContent = AddChild<RpgGuiLayout>(RpgName::Format("%s/content", *Name));
	LayoutContent->BackgroundColor = ContentBackgroundColor;
	LayoutContent->ChildPadding = ContentChildPadding;
	LayoutContent->ChildSpace = ContentChildSpace;
	LayoutContent->Direction = ContentDirection;
}


RpgRectFloat RpgGuiWindow::UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept
{
	const RpgRectBorders borders(AbsoluteRect, BorderThickness, 0.0f);
	const RpgRectFloat innerRect = borders.GetInnerRect();
	const RpgRectFloat titleRect(innerRect.Left, innerRect.Top, innerRect.Right, innerRect.Top + TitleHeight);

	TitleButton->Position = RpgPointFloat(BorderThickness, BorderThickness);
	TitleButton->Dimension = RpgPointFloat(innerRect.Right - innerRect.Left, TitleHeight);

	TitleText->Position = RpgPointFloat(TitleButton->Position.X + 4.0f, TitleButton->Position.Y + 4.0f);

	LayoutContent->Position = RpgPointFloat(BorderThickness, BorderThickness + TitleHeight);
	LayoutContent->Dimension = RpgPointFloat(innerRect.Right - innerRect.Left, innerRect.Bottom - titleRect.Bottom);

	return RpgGuiWidget::UpdateRect(context, canvasRect, offset);
}


void RpgGuiWindow::OnRender(RpgRenderer2D& renderer) const noexcept
{
	RpgGuiWidget::OnRender(renderer);

	const RpgRectBorders borders(AbsoluteRect, BorderThickness, 0.0f);

	for (int i = 0; i < RpgRectBorders::MAX_COUNT; ++i)
	{
		renderer.AddMeshRect(borders.BorderRects[i], BorderColor);
	}

	const RpgRectFloat innerRect = borders.GetInnerRect();

	const RpgRectFloat titleRect(innerRect.Left, innerRect.Top, innerRect.Right, innerRect.Top + TitleHeight);
	renderer.AddMeshRect(titleRect, BorderColor);

	const RpgRectFloat backgroundRect(titleRect.Left, titleRect.Bottom, innerRect.Right, innerRect.Bottom);
	renderer.AddMeshRect(backgroundRect, BackgroundColor);
}
