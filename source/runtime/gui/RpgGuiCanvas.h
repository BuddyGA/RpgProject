#pragma once

#include "RpgGuiWidget.h"



class RpgGuiCanvas : public RpgGuiWidget
{
public:
	RpgGuiCanvas() noexcept
	{
		Flags = RpgGui::FLAG_Layout;
		BackgroundColor = RpgColor::BLACK_TRANSPARENT;
		Order = RPG_GUI_ORDER_CANVAS;
	}


	inline void UpdateWidgets(RpgGuiContext& context, RpgRectFloat rect) noexcept
	{
		Position = rect.GetPosition();
		Dimension = rect.GetDimension();

		UpdateState(context, nullptr);
		UpdateRect(context, AbsoluteRect, RpgPointFloat());
	}

};
