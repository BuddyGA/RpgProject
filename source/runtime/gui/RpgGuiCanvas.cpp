#include "RpgGuiTypes.h"



void RpgGuiCanvas::Update(RpgGuiContext& context, RpgRectFloat rect) noexcept
{
	if (!Name.IsEmpty())
	{
		RootWidget.Name = RpgName::Format("%s_root_widget", *Name);
	}

	RootWidget.Position = rect.GetPosition();
	RootWidget.Dimension = rect.GetDimension();

	RootWidget.UpdateState(context);
	RootWidget.UpdateRect(context, *this, RpgPointFloat());
}
