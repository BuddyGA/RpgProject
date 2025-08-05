#pragma once

#include "RpgEditorAssetBrowser.h"



extern class RpgEditor* g_Editor;

class RpgEditor
{
	RPG_NOCOPYMOVE(RpgEditor)

public:
	RpgEditor() noexcept;
	void SetupGUI(RpgGuiCanvas& canvas) noexcept;
	void MouseMove(const RpgPlatformMouseMoveEvent& e) noexcept;
	void MouseWheel(const RpgPlatformMouseWheelEvent& e) noexcept;
	void MouseButton(const RpgPlatformMouseButtonEvent& e) noexcept;
	void KeyboardButton(const RpgPlatformKeyboardEvent& e) noexcept;


private:
	RpgEditorAssetBrowser* AssetBrowser;

};
