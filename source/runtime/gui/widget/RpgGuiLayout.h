#pragma once

#include "../RpgGuiTypes.h"



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
	RpgGuiLayout() noexcept;
	RpgGuiLayout(const RpgName& in_Name, RpgPointFloat in_Dimension, EDirection in_Direction) noexcept;

	virtual RpgRectFloat UpdateRect(const RpgGuiContext& context, const RpgPointFloat& offset) noexcept override;

	inline void SetScrollValue(float x, float y) noexcept;


protected:
	virtual void OnRender(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept override;


private:
	RpgRectFloat ContentRect;
	RpgPointFloat ScrollValue;

};
