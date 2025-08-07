#pragma once

#include "RpgGuiInputText.h"
#include "RpgGuiWindow.h"



class RpgGuiConsole : public RpgGuiWindow
{
public:
	RpgGuiConsole() noexcept;
	virtual void Initialize() noexcept override;
	virtual RpgRectFloat UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept override;


	inline void Open() noexcept
	{
		if (!bOpened)
		{
			SetVisibility(true);
			bJustOpened = true;
			bOpened = true;
		}
	}

	inline void Close() noexcept
	{
		if (bOpened)
		{
			bOpened = false;
			SetVisibility(false);
		}
	}

	inline void Toggle() noexcept
	{
		if (bOpened)
		{
			Close();
		}
		else
		{
			Open();
		}
	}


protected:
	virtual void OnUpdate(RpgGuiContext& context, RpgGuiWidget* parentLayout) noexcept override;

private:
	void Callback_InputTextCommand_Committed(const RpgString& value) noexcept;


private:
	RpgGuiLayout* LayoutLog;
	int LogEntryCount;

	RpgGuiWidget* Separator;
	RpgGuiInputText* InputTextCommand;
	bool bJustOpened;
	bool bOpened;

};
