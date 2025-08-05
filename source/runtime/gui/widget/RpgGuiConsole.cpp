#include "RpgGuiConsole.h"
#include "../RpgGuiContext.h"
#include "render/RpgRenderer2D.h"
#include "core/RpgConsoleSystem.h"



RpgGuiConsole::RpgGuiConsole() noexcept
{
	Name = "console";
	Flags = RpgGui::FLAG_Layout;
	Position = RpgPointFloat(8.0f, 8.0f);
	Order = RPG_GUI_ORDER_WINDOW_CONSOLE;
	BorderThickness = 4.0f;
	InputTextHeight = 24.0f;
	LogLayout = nullptr;
	InputTextCommand = nullptr;
	bJustOpened = false;
	bOpened = false;

	SetVisibility(false);
}


void RpgGuiConsole::Initialize() noexcept
{
	LogLayout = AddChild<RpgGuiLayout>("console/logs");

	InputTextCommand = AddChild<RpgGuiInputText>("console/command");
	InputTextCommand->bExitFocusOnEnter = false;
	InputTextCommand->TextFont = RpgFont::s_GetDefault_ShareTechMono();
	InputTextCommand->DefaultBackgroundColor = RpgColor(10, 10, 10);
	InputTextCommand->FocusedBackgroundColor = RpgColor(20, 20, 20);
	InputTextCommand->EventCommitted.AddObjectFunction(this, &RpgGuiConsole::Callback_InputTextCommand_Committed);
}


RpgRectFloat RpgGuiConsole::UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept
{
	Dimension = RpgPointFloat(canvasRect.GetWidth() * 0.5f, canvasRect.GetHeight() * 0.5f);

	LogLayout->Position = RpgPointFloat(BorderThickness, BorderThickness);
	LogLayout->Dimension = RpgPointFloat(Dimension.X - BorderThickness * 2.0f, Dimension.Y - BorderThickness * 2.0f - InputTextHeight - BorderThickness);

	InputTextCommand->Position = RpgPointFloat(BorderThickness, LogLayout->Dimension.Y + BorderThickness * 2.0f);
	InputTextCommand->Dimension = RpgPointFloat(Dimension.X - BorderThickness * 2.0f, InputTextHeight);

	return RpgGuiWidget::UpdateRect(context, canvasRect, offset);
}


void RpgGuiConsole::OnUpdate(RpgGuiContext& context) noexcept
{
	if (bJustOpened)
	{
		context.SetFocusWidget(InputTextCommand);
		bJustOpened = false;
	}
}





void RpgGuiConsole::OnRender(RpgRenderer2D& renderer) const noexcept
{
	RPG_Check(renderer.GetCurrentOrderValue() == RPG_GUI_ORDER_WINDOW_CONSOLE);

	RpgRectBorders borders(AbsoluteRect, BorderThickness, 0.0f);
	const RpgColor borderColor(50, 50, 50);

	for (int i = 0; i < RpgRectBorders::MAX_COUNT; ++i)
	{
		renderer.AddMeshRect(borders.BorderRects[i], borderColor);
	}

	const RpgRectFloat innerRect = borders.GetInnerRect();

	const RpgRectFloat logRect(innerRect.Left, innerRect.Top, innerRect.Right, innerRect.Bottom - InputTextHeight - BorderThickness);
	renderer.AddMeshRect(logRect, RpgColor(10, 10, 10, 220));

	const RpgRectFloat separatorRect(innerRect.Left, logRect.Bottom, innerRect.Right, logRect.Bottom + BorderThickness);
	renderer.AddMeshRect(separatorRect, borderColor);
}


void RpgGuiConsole::Callback_InputTextCommand_Committed(const RpgString& value) noexcept
{
	g_ConsoleSystem->ExecuteCommand(*value);
	InputTextCommand->ClearValue();
}
