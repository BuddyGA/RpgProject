#pragma once

#include "../RpgGuiLayout.h"
#include "RpgGuiButton.h"
#include "RpgGuiText.h"



class RpgGuiWindow : public RpgGuiWidget
{
public:
	float BorderThickness;
	RpgColor BorderColor;
	float TitleHeight;
	RpgColor ContentBackgroundColor;
	RpgRectFloat ContentChildPadding;
	RpgPointFloat ContentChildSpace;
	RpgGuiLayout::EDirection ContentDirection;


public:
	RpgGuiWindow(const RpgName& in_Name) noexcept;
	virtual void Initialize() noexcept override;
	virtual RpgRectFloat UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept override;


	inline void SetTitleText(const RpgName& in_TitleText) noexcept
	{
		TitleText->SetTextValue(RpgString(*in_TitleText));
	}

	inline const RpgPointFloat GetWindowContentDimension() const noexcept
	{
		return LayoutContent->Dimension;
	}


	/*
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
	*/


protected:
	virtual void OnRender(RpgRenderer2D& renderer) const noexcept override;
	

	template<typename TWidget, typename...TConstructorArgs>
	inline TWidget* AddContentChild(TConstructorArgs&&... args) noexcept
	{
		return LayoutContent->AddChild<TWidget>(std::forward<TConstructorArgs>(args)...);
	}


private:
	RpgGuiButton* TitleButton;
	RpgGuiText* TitleText;
	RpgGuiLayout* LayoutContent;
	//bool bOpened;

};
