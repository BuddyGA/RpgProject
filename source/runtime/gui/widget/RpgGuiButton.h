#pragma once

#include "../RpgGuiWidget.h"


RPG_DELEGATE_DECLARE_OneParam(RpgGuiButtonEvent, class RpgGuiButton*, button)



class RpgGuiButton : public RpgGuiWidget
{
public:
	RpgGuiButtonEvent EventPressed;


public:
	RpgGuiButton(const RpgName& in_Name) noexcept;
	RpgGuiButton(const RpgName& in_Name, RpgPointFloat in_Dimension) noexcept;


protected:
	virtual void OnUpdate(RpgGuiContext& context) noexcept override;
	virtual void OnRender(RpgRenderer2D& renderer) const noexcept override;

};
