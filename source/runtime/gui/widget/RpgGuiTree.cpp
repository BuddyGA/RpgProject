#include "RpgGuiTree.h"



RpgGuiTree::RpgGuiTree(const RpgName& in_Name) noexcept
	: RpgGuiLayout(in_Name)
{
	Dimension = RpgPointFloat(200.0f, 100.0f);
	bScrollableVertical = true;
	Direction = RpgGuiLayout::DIRECTION_VERTICAL;
	SelectedChild = nullptr;
}
