#include "RpgGuiWindow.h"
#include "render/RpgRenderer2D.h"



RpgGuiWindow::RpgGuiWindow() noexcept
{
	Flags = RpgGui::FLAG_Layout;
	Position = RpgPointFloat(16.0f, 16.0f);
	Dimension = RpgPointFloat(512.0f, 512.0f);
	Order = 250;
	
	BorderThickness = 6.0f;
	BorderColor = RpgColorRGBA(20, 30, 50);
	BackgroundColor = RpgColorRGBA(10, 10, 10, 220);
	TitleHeight = 24.0f;
	LayoutContent = nullptr;
	bOpened = false;

	SetVisibility(false);
}


void RpgGuiWindow::Initialize() noexcept
{
	LayoutContent = AddChild<RpgGuiLayout>();
	LayoutContent->Name = RpgName::Format("%s/content", *Name);
	LayoutContent->Order = Order;
	LayoutContent->ChildPadding = RpgRectFloat(4.0f);
	LayoutContent->ChildSpace = RpgPointFloat(4.0f);
}


RpgRectFloat RpgGuiWindow::UpdateRect(const RpgGuiContext& context, const RpgGuiCanvas& canvas, const RpgPointFloat& offset) noexcept
{
	const RpgRectBorders borders(AbsoluteRect, BorderThickness, 0.0f);
	const RpgRectFloat innerRect = borders.GetInnerRect();
	const RpgRectFloat titleRect(innerRect.Left, innerRect.Top, innerRect.Right, innerRect.Top + TitleHeight);

	LayoutContent->Position = RpgPointFloat(BorderThickness, BorderThickness + TitleHeight);
	LayoutContent->Dimension = RpgPointFloat(innerRect.Right - innerRect.Left, innerRect.Bottom - titleRect.Bottom);

	return RpgGuiWidget::UpdateRect(context, canvas, offset);
}


void RpgGuiWindow::OnRender(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept
{
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

	if (!TitleText.IsEmpty())
	{
		renderer.AddText(*TitleText, TitleText.GetLength(), RpgPointFloat(titleRect.Left + 4.0f, titleRect.Top + 2.0f), RpgColorRGBA::WHITE);
	}
}
