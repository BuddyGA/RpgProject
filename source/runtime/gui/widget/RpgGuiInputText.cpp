#include "RpgGuiInputText.h"
#include "render/RpgRenderer2D.h"



RpgGuiInputText::RpgGuiInputText() noexcept
{
	Flags |= RpgGui::FLAG_InputFocus;

	NumberMinValue = 0.0f;
	NumberMaxValue = 0.0f;
	NumberDragValue = 0.0f;
	bNumberOnly = false;
	bNumberMouseMiddleDrag = false;
	bCommitOnValueChanged = false;
	bCommitOnLostFocus = true;
	bExitFocusOnEnter = true;

	State = STATE_NONE;
	CursorIndex = RPG_INDEX_INVALID;
	SelectedAnchorIndex = RPG_INDEX_INVALID;
	SelectedCharIndex = RPG_INDEX_INVALID;
	SelectedCharCount = 0;
	bSelecting = false;
	bWasCancelled = false;
}


RpgGuiInputText::RpgGuiInputText(const RpgName& in_Name, RpgPointFloat in_Dimension) noexcept
	: RpgGuiInputText()
{
	Name = in_Name;
	Dimension = in_Dimension;
}


void RpgGuiInputText::OnFocusedEnter(RpgGuiContext& context) noexcept
{
	RPG_LogDebug(RpgLogGui, "%s: Begin text edit (Got focus)", *Name);

	ClearSelection();
	State = STATE_EDIT;
	TempValue = Value;
	CursorIndex = Value.GetLength();
	bWasCancelled = false;

	context.BeginTextInput();
}


void RpgGuiInputText::OnFocusedExit(RpgGuiContext& context) noexcept
{
	RPG_LogDebug(RpgLogGui, "%s: End text edit (lost focus)", *Name);

	if (!bWasCancelled && bCommitOnLostFocus)
	{
		Value = TempValue;
		EventCommitted.Broadcast(Value);
	}

	TempValue.Clear();

	if (State == STATE_EDIT)
	{
		context.EndTextInput();
	}

	State = STATE_NONE;
	bWasCancelled = false;
}


void RpgGuiInputText::OnUpdate(RpgGuiContext& context) noexcept
{
	bool bCommitted = false;
	bool bShouldClearFocus = false;
	
	if (State == STATE_EDIT)
	{
		if (context.IsKeyButtonDown(RpgInputKey::KEYBOARD_SHIFT_LEFT))
		{
			RPG_LogDebug(RpgLogGui, "Shift pressed");
		}
		
		if (!bSelecting && context.IsKeyButtonDown(RpgInputKey::KEYBOARD_SHIFT_LEFT))
		{
			bSelecting = true;
			SelectedAnchorIndex = CursorIndex;
		}
		else if (context.IsKeyButtonUp(RpgInputKey::KEYBOARD_SHIFT_LEFT))
		{
			bSelecting = false;
		}


		char c = context.TextInputChar;

		if (c == 8) // BACKSPACE
		{
			if (RemoveSelectedChars())
			{
				if (bSelecting)
				{
					SelectedAnchorIndex = CursorIndex;
				}

				bCommitted = bCommitOnValueChanged;
			}
			else if (CursorIndex > 0 && CursorIndex <= TempValue.GetLength())
			{
				TempValue.RemoveCharAt(--CursorIndex);
				bCommitted = bCommitOnValueChanged;
			}
		}
		else if (c == 13) // ENTER
		{
			RPG_LogDebug(RpgLogGui, "%s: End edit, committed (ENTER)", *Name);

			if (bNumberOnly)
			{
				const float number = RpgMath::Clamp(TempValue.ToFloat(), NumberMinValue, NumberMaxValue);
				TempValue = RpgString::FromFloat(number);
			}

			bCommitted = true;
			bShouldClearFocus = bExitFocusOnEnter;
		}
		else if (c == 27) // ESCAPE
		{
			RPG_LogDebug(RpgLogGui, "%s: End edit, cancelled (ESCAPE)", *Name);
			bShouldClearFocus = true;
			bWasCancelled = true;
		}
		else if (IsValidInputChar(c))
		{
			if (RemoveSelectedChars() && bSelecting)
			{
				SelectedAnchorIndex = CursorIndex;
			}

			bSelecting = false;
			TempValue.InsertCharAt(c, CursorIndex++);
			bCommitted = bCommitOnValueChanged;
		}
		else if (context.IsKeyButtonDown(RpgInputKey::KEYBOARD_ARROW_LEFT))
		{
			if (!bSelecting && SelectedCharCount > 0)
			{
				CursorIndex = RpgMath::Min(CursorIndex, SelectedAnchorIndex);
				ClearSelection();
			}
			else
			{
				--CursorIndex;
			}
		}
		else if (context.IsKeyButtonDown(RpgInputKey::KEYBOARD_ARROW_RIGHT))
		{
			if (!bSelecting && SelectedCharCount > 0)
			{
				CursorIndex = RpgMath::Max(CursorIndex, SelectedAnchorIndex);
				ClearSelection();
			}
			else
			{
				++CursorIndex;
			}
		}
		else if (context.IsKeyButtonDown(RpgInputKey::KEYBOARD_HOME))
		{
			if (!bSelecting && SelectedCharCount > 0)
			{
				ClearSelection();
			}

			CursorIndex = 0;
		}
		else if (context.IsKeyButtonDown(RpgInputKey::KEYBOARD_END))
		{
			if (!bSelecting && SelectedCharCount > 0)
			{
				ClearSelection();
			}

			CursorIndex = TempValue.GetLength();
		}
		else if (context.IsKeyButtonDown(RpgInputKey::KEYBOARD_DELETE))
		{
			if (RemoveSelectedChars())
			{
				if (bSelecting)
				{
					SelectedAnchorIndex = CursorIndex;
				}

				bCommitted = bCommitOnValueChanged;
			}
			else if (CursorIndex >= 0 && CursorIndex < TempValue.GetLength())
			{
				TempValue.RemoveCharAt(CursorIndex);
				bCommitted = bCommitOnValueChanged;
			}
		}
	}
	else if (State == STATE_DRAG)
	{

	}

	CursorIndex = RpgMath::Clamp(CursorIndex, 0, TempValue.GetLength());

	if (bSelecting)
	{
		SelectedCharIndex = RpgMath::Min(SelectedAnchorIndex, CursorIndex);
		SelectedCharCount = RpgMath::Max(SelectedAnchorIndex, CursorIndex) - SelectedCharIndex;

		RPG_Check(SelectedCharIndex >= 0 && (SelectedCharIndex + SelectedCharCount) <= TempValue.GetLength());
	}

	if (bCommitted)
	{
		Value = TempValue;
		EventCommitted.Broadcast(Value);
	}

	if (bShouldClearFocus)
	{
		RPG_Assert(context.CurrentFocused == this);
		context.CurrentFocused = nullptr;
	}
}


void RpgGuiInputText::OnRender(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept
{
	RpgGuiLayout::OnRender(context, renderer, parentClipRect);

	// Set cursor
	if (IsHovered())
	{
		SetCursor(LoadCursorA(NULL, IDC_IBEAM));
	}


	const RpgSharedFont& font = RpgFont::s_GetDefault_Roboto();
	const float pixelHeight = font->GetPixelHeight();


	// Draw rect
	RpgColorRGBA color = RpgColorRGBA(10, 20, 30);

	if (IsFocused())
	{
		color = RpgColorRGBA(20, 30, 60);
		renderer.AddLineRect(AbsoluteRect, RpgColorRGBA::WHITE);
	}

	renderer.AddMeshRect(AbsoluteRect, color);


	// Draw text
	const RpgString& textString = (State == STATE_NONE) ? Value : TempValue;
	RpgPointFloat textPos;

	if (!textString.IsEmpty())
	{
		textPos = RpgGui::CalculateTextAlignmentPosition(AbsoluteRect, textString, font, RpgGuiAlignHorizontal::LEFT, RpgGuiAlignVertical::CENTER);
		textPos.X += 2.0f;
		renderer.AddText(textString.GetData(), textString.GetLength(), textPos, RpgColorRGBA::WHITE);
	}
	else
	{
		textPos.X = AbsoluteRect.Left + 2.0f;
		textPos.Y = AbsoluteRect.Top + (AbsoluteRect.GetDimension().Y * 0.5f) - (pixelHeight * 0.5f);
	}

	if ((GetTickCount64() / 500) % 2 == 0 && (State == STATE_EDIT))
	{
		const RpgPointFloat cursorPos = font->CalculateTextCursorPosition(textString.GetData(), textString.GetLength(), textPos, CursorIndex);
		renderer.AddMeshRect(RpgRectFloat(cursorPos.X, cursorPos.Y, cursorPos.X + 1.0f, cursorPos.Y + pixelHeight), RpgColorRGBA(200, 0, 0));
	}


	// Draw selection
	if (SelectedCharCount > 0)
	{
		const RpgRectFloat selectRect = font->CalculateTextSelectionRect(textString.GetData(), textString.GetLength(), textPos, SelectedCharIndex, SelectedCharCount);
		renderer.AddMeshRect(selectRect, RpgColorRGBA(100, 150, 250));
	}
}
