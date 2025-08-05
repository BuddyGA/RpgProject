#include "RpgGuiWindow.h"
#include "render/RpgRenderer2D.h"



RpgGuiWindow::RpgGuiWindow(const RpgName& in_Name) noexcept
	: RpgGuiWidget(in_Name)
{
	Flags = RpgGui::FLAG_Layout;
	Position = RpgPointFloat(16.0f, 16.0f);
	Dimension = RpgPointFloat(512.0f, 512.0f);
	Order = RPG_GUI_ORDER_WINDOW_DEFAULT;
	
	BorderThickness = 6.0f;
	BorderColor = RpgColor(20, 30, 50);
	BackgroundColor = RpgColor(10, 10, 10, 220);
	TitleHeight = 24.0f;
	TitleButton = nullptr;
	TitleText = nullptr;
	LayoutContent = nullptr;
	bOpened = false;

	SetVisibility(false);
}


void RpgGuiWindow::Initialize() noexcept
{
	TitleButton = AddChild<RpgGuiButton>(RpgName::Format("%s/title_button", *Name));

	TitleText = AddChild<RpgGuiText>(RpgName::Format("%s/title_text", *Name));

	LayoutContent = AddChild<RpgGuiLayout>(RpgName::Format("%s/content", *Name));
	LayoutContent->ChildPadding = RpgRectFloat(4.0f);
	LayoutContent->ChildSpace = RpgPointFloat(4.0f);
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
	RPG_Check(renderer.GetCurrentOrderValue() == RPG_GUI_ORDER_WINDOW_DEFAULT);

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
