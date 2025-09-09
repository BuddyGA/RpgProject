#pragma once

#include "render/RpgModel.h"
#include "animation/RpgAnimationTypes.h"
#include "RpgEditorAssetBrowser.h"
#include "script/RpgEditorScript_Camera.h"



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
	void TickUpdate(float deltaTime) noexcept;
	void Render2d(RpgRenderer2D& r2d) noexcept;

	void SaveLevel(const RpgName& name) noexcept;
	void LevelLoaded(RpgWorld* world) noexcept;

	void ImportTexture(RpgSharedTexture2D& out_Texture, const RpgEditorImportSetting_Texture& setting) noexcept;
	void ImportModel(RpgArray<RpgSharedModel>& out_Models, RpgSharedAnimationSkeleton& out_Skeleton, RpgArray<RpgSharedAnimationClip>& out_Animations, const RpgEditorImportSetting_Model& setting) noexcept;


private:
	RpgString OpenImportAssetDialog() const noexcept;


private:
	RpgEditorAssetBrowserWindow* AssetBrowser;

	// main world
	RpgWorld* MainWorld;

	// Camera object
	RpgGameObject CameraObject;

	// Script camera
	RpgEditorScript_Camera CameraScript;

};
