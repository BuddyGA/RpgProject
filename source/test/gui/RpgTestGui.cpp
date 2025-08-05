#include "RpgTestGui.h"
#include "gui/RpgGuiCanvas.h"
#include "gui/RpgGuiLayout.h"
#include "gui/widget/RpgGuiButton.h"
#include "gui/widget/RpgGuiInputText.h"



void RpgTest::Gui::Create(RpgGuiCanvas& canvas) noexcept
{
	RpgGuiLayout* layoutA = canvas.AddChild<RpgGuiLayout>("layoutA", RpgPointFloat(256, 512), RpgGuiLayout::DIRECTION_VERTICAL);
	{
		layoutA->Position = RpgPointFloat(256, 256);
		layoutA->bScrollableVertical = true;

		for (int i = 0; i < 16; ++i)
		{
			layoutA->AddChild<RpgGuiButton>(RpgName::Format("layoutA_button_%i", i), RpgPointFloat(120, 24));
		}

		RpgGuiLayout* layoutB = layoutA->AddChild<RpgGuiLayout>("layoutB", RpgPointFloat(256, 256), RpgGuiLayout::DIRECTION_VERTICAL);
		{
			layoutB->ChildPadding = RpgRectFloat();
			layoutB->bScrollableVertical = true;
			
			for (int i = 0; i < 8; ++i)
			{
				layoutB->AddChild<RpgGuiButton>(RpgName::Format("layoutB_button_%i", i), RpgPointFloat(300, 24));
				layoutB->AddChild<RpgGuiInputText>(RpgName::Format("layoutB_inputtext_%i", i), RpgPointFloat(200, 24));
			}
		}
	}


	RpgGuiLayout* layoutC = canvas.AddChild<RpgGuiLayout>("layoutC", RpgPointFloat(256, 256), RpgGuiLayout::DIRECTION_VERTICAL);
	{
		layoutC->Position = RpgPointFloat(512, 512);
		layoutC->bScrollableVertical = true;

		layoutC->AddChild<RpgGuiInputText>("layoutC_inputtext_0", RpgPointFloat(200, 24));
		layoutC->AddChild<RpgGuiInputText>("layoutC_inputtext_1", RpgPointFloat(200, 24));
		layoutC->AddChild<RpgGuiInputText>("layoutC_inputtext_2", RpgPointFloat(200, 24));
	}
}
