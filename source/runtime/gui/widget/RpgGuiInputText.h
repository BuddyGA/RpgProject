#pragma once

#include "../RpgGuiLayout.h"



class RpgGuiInputText : public RpgGuiWidget
{
public:
	typedef RpgArrayInline<char, 8> FCharFilterArray;


	RPG_DELEGATE_DECLARE_OneParam(FEventCommitted, const RpgString&, Value)
	FEventCommitted EventCommitted;


public:
	FCharFilterArray CharFilters;
	float NumberMinValue;
	float NumberMaxValue;
	float NumberDragValue;
	bool bNumberOnly;
	bool bNumberMouseMiddleDrag;
	bool bCommitOnValueChanged;
	bool bCommitOnLostFocus;
	bool bExitFocusOnEnter;
	RpgSharedFont TextFont;
	RpgColor TextColor;
	RpgColor HighlightColor;
	RpgColor DefaultBackgroundColor;
	RpgColor FocusedBackgroundColor;
	RpgString Value;


public:
	RpgGuiInputText(const RpgName& in_Name) noexcept;
	RpgGuiInputText(const RpgName& in_Name, RpgPointFloat in_Dimension) noexcept;


	inline void ClearValue() noexcept
	{
		Value.Clear();
		TempValue.Clear();
	}


protected:
	virtual void OnFocusedEnter(RpgGuiContext& context) noexcept override;
	virtual void OnFocusedExit(RpgGuiContext& context) noexcept override;
	virtual void OnUpdate(RpgGuiContext& context) noexcept override;
	virtual void OnRender(RpgRenderer2D& renderer) const noexcept override;


private:
	enum EState : uint8_t
	{
		STATE_NONE,
		STATE_EDIT,
		STATE_DRAG
	};


private: 
	inline bool IsValidInputChar(char c) const noexcept
	{
		if (c == '\0')
		{
			return false;
		}

		if (bNumberOnly)
		{
			return (c >= '0' && c <= '9') || (c == '.') || (c == '-');
		}

		return CharFilters.FindIndexByValue(c) == RPG_INDEX_INVALID;
	}


	inline bool RemoveSelectedChars() noexcept
	{
		if (SelectedCharCount == 0)
		{
			return false;
		}

		bool bRemoved = TempValue.RemoveCharAtRange(SelectedCharIndex, SelectedCharCount);
		CursorIndex = SelectedCharIndex;
		SelectedCharIndex = RPG_INDEX_INVALID;
		SelectedCharCount = 0;

		return bRemoved;
	}


	inline void SelectAllChars() noexcept
	{
		if (TempValue.IsEmpty())
		{
			return;
		}

		const int charLength = TempValue.GetLength();
		CursorIndex = charLength;
		SelectedAnchorIndex = 0;
		SelectedCharIndex = 0;
		SelectedCharCount = charLength;
	}


	inline void ClearSelection() noexcept
	{
		SelectedAnchorIndex = RPG_INDEX_INVALID;
		SelectedCharIndex = RPG_INDEX_INVALID;
		SelectedCharCount = 0;
		bSelecting = false;
	}


private:
	EState State;
	RpgString TempValue;
	int CursorIndex;
	int SelectedAnchorIndex;
	int SelectedCharIndex;
	int SelectedCharCount;
	bool bSelecting;
	bool bWasCancelled;

};
