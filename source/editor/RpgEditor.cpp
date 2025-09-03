#include "RpgEditor.h"
#include "core/RpgConsoleSystem.h"
#include "gui/RpgGuiCanvas.h"
#include "asset/RpgAssetImporter.h"
#include "asset/RpgAssetManager.h"
#include <ShObjIdl_core.h>



RpgEditor* g_Editor = nullptr;


RpgEditor::RpgEditor() noexcept
{

}


void RpgEditor::SetupGUI(RpgGuiCanvas& canvas) noexcept
{
	AssetBrowser = canvas.AddChild<RpgEditorAssetBrowserWindow>();
	AssetBrowser->SetTitleText("ASSET BROWSER");
	AssetBrowser->Refresh();
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
	const bool bIsCtrlDown = GetAsyncKeyState(VK_LCONTROL) & 0x8000;
	
	if (e.bIsDown)
	{
		if (e.Button == RpgInputKey::KEYBOARD_F1)
		{
			if (bIsCtrlDown)
			{
				const RpgFilePath sourceFilePath = OpenImportAssetDialog();
				if (sourceFilePath.IsFilePath())
				{
					RPG_CONSOLE_Log(RpgLogEditor, "Importing asset from source file (%s)", *sourceFilePath);
					
					if (RpgAssetFileModel::IsFileSupported(sourceFilePath))
					{
						RpgAssetImportSetting_Model setting;
						setting.SourceFilePath = sourceFilePath;
						setting.bImportMaterialTexture = true;
						setting.bGenerateTextureMipMaps = true;
						setting.bImportSkeleton = true;
						setting.bImportAnimation = true;
						setting.Scale = 1.0f;

						RpgArray<RpgSharedModel> models;
						RpgSharedAnimationSkeleton skeleton;
						RpgArray<RpgSharedAnimationClip> animations;
						g_AssetImporter->ImportModel(models, skeleton, animations, setting);

						if (models.IsEmpty())
						{
							RPG_CONSOLE_Warn(RpgLogEditor, "Mesh model not found from source file (%s)!", *sourceFilePath);
							return;
						}

						for (const auto& mdl : models)
						{
							RPG_Check(mdl->GetMeshCount() == 1);
							g_AssetManager->SaveMesh(mdl->GetMeshLod(0, 0));
						}
					}
					else if (RpgAssetFileImage::IsFileSupported(sourceFilePath))
					{
						RPG_NotImplementedYet();
					}
				}
			}
			else
			{
				const bool bVisible = AssetBrowser->IsVisible();
				AssetBrowser->SetVisibility(!bVisible);
			}
		}
	}
}


RpgString RpgEditor::OpenImportAssetDialog() const noexcept
{
	RpgString filePath;

	const wchar_t* TITLE = L"Import Asset";

	const COMDLG_FILTERSPEC FILTER_FILE_TYPES[] =
	{
		{ L"Supported Files", L"*.obj;*.fbx;*.gltf;*.glb;*.bmp;*.png;*.tga;*.dds;*.jpg" },
		{ L"Model Files", L"*.obj;*.fbx;*.gltf;*.glb" },
		{ L"Image Files", L"*.bmp;*.png;*.tga;*.dds;*.jpg" },
	};


	IFileDialog* openDialog = nullptr;
	if (!SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&openDialog))))
	{
		RPG_LogError(RpgLogEditor, "Fail to open import asset dialog!");
		return filePath;
	}

	openDialog->SetTitle(TITLE);
	openDialog->SetFileTypes(ARRAYSIZE(FILTER_FILE_TYPES), FILTER_FILE_TYPES);
	openDialog->SetFileTypeIndex(1);

	if (!SUCCEEDED(openDialog->Show(RpgPlatformProcess::GetMainWindowHandle())))
	{
		openDialog->Release();
		return filePath;
	}

	IShellItem* result = nullptr;
	if (!SUCCEEDED(openDialog->GetResult(&result)))
	{
		RPG_LogError(RpgLogEditor, "Fail to get open file dialog result!");
		openDialog->Release();
		return filePath;
	}

	wchar_t* tempFilePath = nullptr;
	if (!SUCCEEDED(result->GetDisplayName(SIGDN_FILESYSPATH, &tempFilePath)))
	{
		RPG_LogError(RpgLogEditor, "Fail to get open file dialog result!");
		openDialog->Release();
		return filePath;
	}

	filePath.Resize(MAX_PATH);
	RpgPlatformString::WStringToMultibyte(*filePath, tempFilePath, MAX_PATH);

	CoTaskMemFree(tempFilePath);
	result->Release();
	openDialog->Release();

	return filePath;
}
