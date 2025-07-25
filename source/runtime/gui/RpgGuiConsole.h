#pragma once

#include "widget/RpgGuiInputText.h"



class RpgGuiConsole : public RpgGuiWidget
{
public:
	float BorderThickness;
	float InputTextHeight;


public:
	RpgGuiConsole() noexcept;
	virtual void Initialize() noexcept override;
	virtual RpgRectFloat UpdateRect(const RpgGuiContext& context, const RpgGuiCanvas& canvas, const RpgPointFloat& offset) noexcept override;


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
	virtual void OnUpdate(RpgGuiContext& context) noexcept override;
	virtual void OnRender(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept override;

private:
	void Callback_InputTextCommand_Committed(const RpgString& value) noexcept;


private:
	RpgGuiLayout* LogLayout;
	RpgGuiInputText* InputTextCommand;
	bool bJustOpened;
	bool bOpened;

};
