#include "RpgEditor.h"
#include "core/asset/RpgAssetSystem.h"
#include "gui/RpgGuiCanvas.h"
#include "render/world/RpgRenderComponent.h"
#include "render/world/RpgRenderWorldSubsystem.h"
#include "animation/world/RpgAnimationWorldSubsystem.h"
#include "game/RpgGameApp.h"
#include "task/RpgEditorTask_ImportModel.h"
#include <ShObjIdl_core.h>
#include <compressonator.h>



RpgEditor* g_Editor = nullptr;


RpgEditor::RpgEditor() noexcept
{
	AssetBrowser = nullptr;
	MainWorld = nullptr;

	CMP_InitFramework();
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
					
					if (RpgEditorImportModel::IsFileSupported(sourceFilePath))
					{
						RpgEditorImportSetting_Model setting;
						setting.SourceFilePath = sourceFilePath;
						setting.bImportMaterialTexture = true;
						setting.bGenerateTextureMipMaps = true;
						setting.bImportSkeleton = true;
						setting.bImportAnimation = true;
						setting.Scale = 1.0f;

						RpgArray<RpgSharedModel> models;
						RpgSharedAnimationSkeleton skeleton;
						RpgArray<RpgSharedAnimationClip> animations;
						ImportModel(models, skeleton, animations, setting);

						if (models.IsEmpty())
						{
							RPG_CONSOLE_Warn(RpgLogEditor, "Mesh model not found from source file (%s)!", *sourceFilePath);
							return;
						}

						for (const auto& mdl : models)
						{
							RPG_Check(mdl->GetMeshCount() == 1);
							RpgSharedMesh mesh = mdl->GetMeshLod(0, 0);
							g_AssetSystem->SaveAsset<RpgMesh>(mesh, "game/mesh");
						}
					}
					else if (RpgEditorImportTexture::IsFileSupported(sourceFilePath))
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
		else if (e.Button == RpgInputKey::KEYBOARD_EQUALS)
		{
			g_GameApp->GetMainRenderer()->Gamma += 0.01f;
		}
		else if (e.Button == RpgInputKey::KEYBOARD_MINUS)
		{
			g_GameApp->GetMainRenderer()->Gamma -= 0.01f;
		}
		else if (e.Button == RpgInputKey::KEYBOARD_0)
		{
			RpgRenderComponent_Camera* cameraComp = CameraObject.GetComponent<RpgRenderComponent_Camera>();
			cameraComp->bFrustumCulling = !cameraComp->bFrustumCulling;
		}
		else if (e.Button == RpgInputKey::KEYBOARD_9)
		{
			RpgAnimationWorldSubsystem* subsystem = MainWorld->Subsystem_Get<RpgAnimationWorldSubsystem>();
			subsystem->bDebugDrawSkeletonBones = !subsystem->bDebugDrawSkeletonBones;
		}
		else if (e.Button == RpgInputKey::KEYBOARD_8)
		{
			RpgRenderWorldSubsystem* subsystem = MainWorld->Subsystem_Get<RpgRenderWorldSubsystem>();
			subsystem->bDebugDrawMeshBound = !subsystem->bDebugDrawMeshBound;
		}
		else if (e.Button == RpgInputKey::KEYBOARD_F8)
		{
			RPG_Check(MainWorld);
			MainWorld->SaveLevel("level_main");
		}
		else if (e.Button == RpgInputKey::KEYBOARD_F9)
		{
			CameraObject.DetachScript(&CameraScript);
			CameraObject = RpgGameObject();
			g_GameApp->OpenLevel(RpgString("game/level_main"));
		}
		else if (e.Button == RpgInputKey::KEYBOARD_F10)
		{
			if (MainWorld->HasStartedPlay())
			{
				MainWorld->DispatchStopPlay();
			}
			else
			{
				MainWorld->DispatchStartPlay();
			}
		}
	}
}


void RpgEditor::TickUpdate(float deltaTime) noexcept
{

}


void RpgEditor::Render2d(RpgRenderer2D& r2d) noexcept
{
	RpgRenderer* mainRenderer = g_GameApp->GetMainRenderer();

	const RpgPointInt windowDimension = g_GameApp->GetWindowDimension();
	const RpgTransform cameraTransform = !CameraObject.IsNull() ? CameraObject.GetWorldTransform() : RpgTransform();

	float cameraPitch, cameraYaw;
	CameraScript.GetRotationPitchYaw(cameraPitch, cameraYaw);

	RpgRenderComponent_Camera* cameraComp = !CameraObject.IsNull() ? CameraObject.GetComponent<RpgRenderComponent_Camera>() : nullptr;

	// Debug info
	static RpgString debugInfoText;

	debugInfoText = RpgString::Format(
		"WindowSize: %i, %i\n"
		"CameraPosition: %.2f, %.2f, %.2f\n"
		"CameraPitchYaw: %.2f, %.2f\n"
		"CameraFrustumCulling: %d\n"
		"Gamma: %.2f\n"
		"VSync: %d\n"
		"\n"
		"GameObject: %i\n"
		, windowDimension.X, windowDimension.Y
		, cameraTransform.Position.X, cameraTransform.Position.Y, cameraTransform.Position.Z
		, cameraPitch, cameraYaw
		, cameraComp ? cameraComp->bFrustumCulling : false
		, mainRenderer->Gamma
		, mainRenderer->GetVsync()
		, MainWorld->GetGameObjectCount()
	);

	r2d.AddText(*debugInfoText, debugInfoText.GetLength(), RpgPointFloat(8.0f, 8.0f), RpgColor(255, 255, 255));
}


void RpgEditor::LevelLoaded(RpgWorld* world) noexcept
{
	MainWorld = world;

	CameraObject = MainWorld->CreateGameObject("editor_camera", nullptr, true);
	CameraObject.AddComponent<RpgRenderComponent_Camera>();
	CameraObject.SpawnAtTransform(RpgTransform());
	CameraObject.AttachScript(&CameraScript);

	g_GameApp->SetMainCamera(CameraObject);
}


void RpgEditor::ImportTexture(RpgSharedTexture2D& out_Texture, const RpgEditorImportSetting_Texture& setting) noexcept
{
	RPG_IsMainThread();

	RPG_NotImplementedYet();
}


void RpgEditor::ImportModel(RpgArray<RpgSharedModel>& out_Models, RpgSharedAnimationSkeleton& out_Skeleton, RpgArray<RpgSharedAnimationClip>& out_Animations, const RpgEditorImportSetting_Model& setting) noexcept
{
	RPG_IsMainThread();

	out_Models.Clear();
	out_Skeleton.Release();
	out_Animations.Clear();

	RpgEditorTask_ImportModel task;
	task.Reset();
	task.SourceFilePath = setting.SourceFilePath;
	task.Scale = setting.Scale;
	task.bImportMaterialTexture = setting.bImportMaterialTexture;
	task.bImportSkeleton = setting.bImportSkeleton;
	task.bImportAnimation = setting.bImportAnimation;
	task.bGenerateTextureMipMaps = setting.bGenerateTextureMipMaps;
	task.bIgnoreTextureNormals = setting.bIgnoreTextureNormals;
	task.Execute();

	out_Models = task.GetImportedModels();

	if (setting.bImportSkeleton)
	{
		out_Skeleton = task.GetImportedSkeleton();
	}

	if (setting.bImportAnimation)
	{
		out_Animations = task.GetImportedAnimations();
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
