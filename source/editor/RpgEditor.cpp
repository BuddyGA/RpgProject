#include "RpgEditor.h"
#include "gui/RpgGuiCanvas.h"



RpgEditor* g_Editor = nullptr;


RpgEditor::RpgEditor() noexcept
{

}


void RpgEditor::SetupGUI(RpgGuiCanvas& canvas) noexcept
{
	AssetBrowser = canvas.AddChild<RpgEditorAssetBrowser>();
	AssetBrowser->SetTitleText("ASSET BROWSER");
}


void RpgEditor::MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept
{

}


void RpgEditor::MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept
{

}


void RpgEditor::MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept
{

}


void RpgEditor::KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept
{
	if (e.bIsDown)
	{
		if (e.Button == RpgInputKey::KEYBOARD_F1)
		{
			AssetBrowser->Toggle();
		}
	}
}
