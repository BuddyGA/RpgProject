#pragma once

#include "widget/RpgGuiLayout.h"



class RpgGuiWindow : public RpgGuiWidget
{
public:
	float BorderThickness;
	RpgColorRGBA BorderColor;
	RpgColorRGBA BackgroundColor;
	RpgName TitleText;
	float TitleHeight;


public:
	RpgGuiWindow() noexcept;
	virtual void Initialize() noexcept override;
	virtual RpgRectFloat UpdateRect(const RpgGuiContext& context, const RpgGuiCanvas& canvas, const RpgPointFloat& offset) noexcept override;


	inline void Open() noexcept
	{
		if (!bOpened)
		{
			SetVisibility(true);
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
	virtual void OnRender(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept override;
	

	template<typename TWidget, typename...TConstructorArgs>
	inline TWidget* AddContentChild(TConstructorArgs&&... args) noexcept
	{
		return LayoutContent->AddChild<TWidget>(std::forward<TConstructorArgs>(args)...);
	}


private:
	RpgGuiLayout* LayoutContent;
	bool bOpened;

};
