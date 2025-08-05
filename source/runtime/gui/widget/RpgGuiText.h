#pragma once

#include "../RpgGuiWidget.h"



class RpgGuiText : public RpgGuiWidget
{
public:
	RpgSharedFont Font;
	RpgColor Color;


public:
	RpgGuiText(const RpgName& in_Name) noexcept;
	virtual RpgRectFloat UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept override;


	inline void SetTextValue(const RpgString& in_Value) noexcept
	{
		TextValue = in_Value;
		bDirtyDimension = true;
	}


protected:
	virtual void OnRender(RpgRenderer2D& renderer) const noexcept override;

private:
	RpgString TextValue;
	bool bDirtyDimension;

};
