#pragma once

#include "core/RpgDelegate.h"
#include "input/RpgInputTypes.h"
#include "render/RpgFont.h"
#include "render/RpgMaterial.h"


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogGui)


#define RPG_GUI_SCROLL_VALUE_FIRST		0
#define RPG_GUI_SCROLL_VALUE_LAST		-1

#define RPG_GUI_ORDER_CANVAS			250
#define RPG_GUI_ORDER_WINDOW_DEFAULT	240
#define RPG_GUI_ORDER_WINDOW_FOCUSED	230
#define RPG_GUI_ORDER_WINDOW_CONSOLE	220
#define RPG_GUI_ORDER_WINDOW_DIALOGUE	10


constexpr const char* RPG_GUI_TAG_WINDOW = "window";
constexpr const char* RPG_GUI_TAG_CONSOLE = "console";


class RpgGuiWidget;
class RpgGuiLayout;
class RpgGuiCanvas;
class RpgGuiContext;



enum class RpgGuiAlignHorizontal : uint8_t
{
	LEFT = 0,
	CENTER,
	RIGHT
};



enum class RpgGuiAlignVertical : uint8_t
{
	TOP = 0,
	CENTER,
	BOTTOM
};



namespace RpgGui
{
	enum Flags : uint16_t
	{
		FLAG_None				= (0),
		FLAG_Layout				= (1 << 0),
		FLAG_InputFocus			= (1 << 1),
		FLAG_DragDropItem		= (1 << 2),
		FLAG_DragDropTarget		= (1 << 3),
		FLAG_State_Invisible	= (1 << 4),
		FLAG_State_Hovered		= (1 << 5),
		FLAG_State_Pressed		= (1 << 6),
		FLAG_State_Released		= (1 << 7),
		FLAG_State_Focused		= (1 << 8),
	};


	extern RpgPointFloat CalculateTextAlignmentPosition(const RpgRectFloat& rect, const RpgString& text, const RpgSharedFont& font, RpgGuiAlignHorizontal alignH, RpgGuiAlignVertical alignV) noexcept;
}
