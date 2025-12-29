#pragma once

#include "../RpgGuiLayout.h"



class RpgGuiTreeItem : public RpgGuiWidget
{
	
};



class RpgGuiTree : public RpgGuiLayout
{
public:
	RpgGuiTree(const RpgName& in_Name) noexcept;


private:
	RpgGuiWidget* SelectedChild;

};
