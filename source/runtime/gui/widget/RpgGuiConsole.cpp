#include "RpgGuiConsole.h"
#include "../RpgGuiContext.h"
#include "render/RpgRenderer2D.h"
#include "core/RpgConsoleSystem.h"



#define RPG_GUI_CONSOLE_INPUT_TEXT_HEIGHT	24


RpgGuiConsole::RpgGuiConsole() noexcept
	: RpgGuiWindow("console")
{
	Position = RpgPointFloat(4.0f, 4.0f);
	Order = RPG_GUI_ORDER_WINDOW_CONSOLE;

	BorderThickness = 4.0f;
	ContentChildPadding = RpgRectFloat(0.0f);
	ContentChildSpace = RpgPointFloat(0.0f);
	ContentDirection = RpgGuiLayout::DIRECTION_VERTICAL;

	LayoutLog = nullptr;
	LogEntryCount = 0;

	InputTextCommand = nullptr;
	bJustOpened = false;
	bOpened = false;
}


void RpgGuiConsole::Initialize() noexcept
{
	RpgGuiWindow::Initialize();

	SetTitleText("CONSOLE");

	LayoutLog = AddContentChild<RpgGuiLayout>("console/logs");
	LayoutLog->ChildPadding = RpgRectFloat(4.0f);
	LayoutLog->Direction = RpgGuiLayout::DIRECTION_VERTICAL;
	LayoutLog->bScrollableVertical = true;

	Separator = AddContentChild<RpgGuiWidget>("console/separator");
	Separator->BackgroundColor = BorderColor;

	InputTextCommand = AddContentChild<RpgGuiInputText>("console/command");
	InputTextCommand->bExitFocusOnEnter = false;
	InputTextCommand->TextFont = RpgFont::s_GetDefault_ShareTechMono();
	InputTextCommand->BackgroundColor = RpgColor(15, 15, 15);
	InputTextCommand->BackgroundColorFocused = RpgColor(30, 30, 30);
	InputTextCommand->EventCommitted.AddObjectFunction(this, &RpgGuiConsole::Callback_InputTextCommand_Committed);
}


RpgRectFloat RpgGuiConsole::UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept
{
	const RpgRectFloat absRect = RpgGuiWindow::UpdateRect(context, canvasRect, offset);

	const RpgPointFloat canvasDimension = canvasRect.GetDimension();
	Dimension.X = canvasDimension.X * 0.5f;
	Dimension.Y = canvasDimension.Y * 0.5f;

	const RpgPointFloat contentDimension = GetWindowContentDimension();
	LayoutLog->Dimension = RpgPointFloat(0.0f, contentDimension.Y - Separator->Dimension.Y - InputTextCommand->Dimension.Y);
	Separator->Dimension = RpgPointFloat(0.0f, BorderThickness);
	InputTextCommand->Dimension = RpgPointFloat(0.0f, RPG_GUI_CONSOLE_INPUT_TEXT_HEIGHT);

	return absRect;
}


void RpgGuiConsole::OnUpdate(RpgGuiContext& context, RpgGuiWidget* parentLayout) noexcept
{
	const int consoleLogEntryCount = g_ConsoleSystem->GetLogCount();

	if (consoleLogEntryCount > LogEntryCount)
	{
		for (int i = LogEntryCount; i < consoleLogEntryCount; ++i)
		{
			RpgColor color;
			const char* message = g_ConsoleSystem->GetLogMessage(i, nullptr, &color);

			RpgGuiText* logText = LayoutLog->AddChild<RpgGuiText>(RpgName::Format("console/logs/entry_%i", i));
			logText->Font = RpgFont::s_GetDefault_ShareTechMono();
			logText->Color = color;
			logText->SetTextValue(RpgString(message));
		}

		LayoutLog->SetScrollValue(0.0f, RPG_GUI_SCROLL_VALUE_LAST);
		LogEntryCount = consoleLogEntryCount;
	}

	if (bJustOpened)
	{
		context.SetFocusWidget(InputTextCommand);
		bJustOpened = false;
	}
}


void RpgGuiConsole::Callback_InputTextCommand_Committed(const RpgString& value) noexcept
{
	g_ConsoleSystem->ExecuteCommand(*value);
	InputTextCommand->ClearValue();
}
