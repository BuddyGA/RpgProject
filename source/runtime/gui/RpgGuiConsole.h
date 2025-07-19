#pragma once

#include "RpgGuiTypes.h"



class RpgGuiConsole : public RpgGuiWidget
{
public:
	RpgGuiConsole() noexcept;

protected:
	virtual void OnRender(const RpgGuiContext& context, RpgRenderer2D& renderer, const RpgRectFloat& parentClipRect) const noexcept override;

};
