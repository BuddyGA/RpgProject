#pragma once

#include "RpgGuiWidget.h"



class RpgGuiLayout : public RpgGuiWidget
{
public:
	enum EDirection : uint8_t
	{
		DIRECTION_NONE = 0,
		DIRECTION_HORIZONTAL,
		DIRECTION_VERTICAL
	};


public:
	RpgRectFloat ChildPadding;
	RpgPointFloat ChildSpace;
	bool bScrollableHorizontal;
	bool bScrollableVertical;
	EDirection Direction;


public:
	RpgGuiLayout(const RpgName& in_Name) noexcept;
	RpgGuiLayout(const RpgName& in_Name, RpgPointFloat in_Dimension, EDirection in_Direction) noexcept;

	virtual RpgRectFloat UpdateRect(const RpgGuiContext& context, const RpgRectFloat& canvasRect, const RpgPointFloat& offset) noexcept override;

	void SetScrollValue(float x, float y) noexcept;


protected:
	virtual void OnRender(RpgRenderer2D& renderer) const noexcept override;


private:
	void ApplyScroll(float x, float y) noexcept;


private:
	RpgRectFloat ContentRect;
	RpgPointFloat ScrollValue;
	RpgPointFloat PendingScrollValue;

};
