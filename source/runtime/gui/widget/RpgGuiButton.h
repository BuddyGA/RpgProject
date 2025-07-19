#pragma once

#include "../RpgGuiTypes.h"



class RpgGuiButton : public RpgGuiWidget
{
public:
	RpgDelegate EventPressed;


public:
	RpgGuiButton() noexcept;
	RpgGuiButton(const RpgName& in_Name, RpgPointFloat in_Dimension) noexcept;


protected:
	virtual void OnUpdate(RpgGuiContext& context) noexcept override;
	virtual void OnRender(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept override;

};
