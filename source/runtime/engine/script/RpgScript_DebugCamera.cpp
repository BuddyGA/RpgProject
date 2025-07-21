#include "RpgScript_DebugCamera.h"
#include "core/world/RpgWorld.h"
#include "input/RpgInputManager.h"
#include "render/world/RpgRenderComponent.h"



RpgScript_DebugCamera::RpgScript_DebugCamera() noexcept
{
	Flashlight = nullptr;
	PitchValue = 0.0f;
	YawValue = 0.0f;
	bInitialized = false;

	PitchMin = -80.0f;
	PitchMax = 80.0f;
	RotationSpeed = 90.0f;
	MoveSpeed = 500.0f;
}


void RpgScript_DebugCamera::AttachedToGameObject() noexcept
{
	if (!bInitialized)
	{
		bInitialized = true;

		RpgTransform transform = World->GameObject_GetWorldTransform(GameObject);
		transform.Position = RpgVector3(0.0f, 500.0f, 0.0f);
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(0.0f, 0.0f, 0.0f);

		World->GameObject_SetWorldTransform(GameObject, transform);

		Flashlight = World->GameObject_AddComponent<RpgRenderComponent_Light>(GameObject);
		Flashlight->Type = RpgRenderLight::TYPE_SPOT_LIGHT;
		Flashlight->ColorIntensity = RpgColorLinear(1.0f, 1.0f, 1.0f, 2.0f);
		Flashlight->AttenuationRadius = 1600.0f;
		Flashlight->SpotInnerConeDegree = 20.0f;
		Flashlight->SpotOuterConeDegree = 40.0f;
		Flashlight->bCastShadow = false;
		Flashlight->bIsVisible = false;
	}
}


void RpgScript_DebugCamera::TickUpdate(float deltaTime) noexcept
{
	RpgTransform transform = World->GameObject_GetWorldTransform(GameObject);

	if (g_InputManager->IsKeyButtonPressed(RpgInputKey::MOUSE_RIGHT))
	{
		RPG_Log(RpgLogTemp, "Camera FreeFly update movement BEGIN");
		RpgPlatformMouse::Capture(NULL, true);
		RpgPlatformMouse::SetCursorHidden(true);
		SavedMousePos = RpgPointFloat(RpgPlatformMouse::GetCursorPosition(NULL));
	}
	else if (g_InputManager->IsKeyButtonReleased(RpgInputKey::MOUSE_RIGHT))
	{
		RPG_Log(RpgLogTemp, "Camera FreeFly update movement END");
		RpgPlatformMouse::SetCursorPosition(NULL, RpgPointInt(SavedMousePos));
		RpgPlatformMouse::Capture(NULL, false);
		RpgPlatformMouse::SetCursorHidden(false);
	}

	if (g_InputManager->IsKeyButtonDown(RpgInputKey::MOUSE_RIGHT))
	{
		RpgVector3 moveAxis;

		if (g_InputManager->IsKeyButtonDown(RpgInputKey::KEYBOARD_W))
		{
			moveAxis.Z = 1.0f;
		}

		if (g_InputManager->IsKeyButtonDown(RpgInputKey::KEYBOARD_S))
		{
			moveAxis.Z = -1.0f;
		}

		if (g_InputManager->IsKeyButtonDown(RpgInputKey::KEYBOARD_D))
		{
			moveAxis.X = 1.0f;
		}

		if (g_InputManager->IsKeyButtonDown(RpgInputKey::KEYBOARD_A))
		{
			moveAxis.X = -1.0f;
		}

		const RpgPointFloat currentMousePos = RpgPointFloat(RpgPlatformMouse::GetCursorPosition(NULL));
		const RpgPointFloat deltaCursorPos = currentMousePos - SavedMousePos;
		PitchValue += 0.25f * deltaCursorPos.Y;
		YawValue += 0.25f * deltaCursorPos.X;
		PitchValue = RpgMath::Clamp(PitchValue, PitchMin, PitchMax);
		YawValue = RpgMath::ClampDegree(YawValue);
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(PitchValue, YawValue, 0.0f);

		RpgVector3 moveDirection = transform.GetAxisRight() * moveAxis.X;
		moveDirection += transform.GetAxisUp() * moveAxis.Y;
		moveDirection += transform.GetAxisForward() * moveAxis.Z;
		moveDirection.Normalize();
		moveDirection *= MoveSpeed * deltaTime;
		transform.Position += moveDirection;

		RpgPlatformMouse::SetCursorPosition(NULL, RpgPointInt(SavedMousePos));
	}

	World->GameObject_SetWorldTransform(GameObject, transform);


	if (g_InputManager->IsKeyButtonPressed(RpgInputKey::KEYBOARD_F))
	{
		Flashlight->bIsVisible = !Flashlight->bIsVisible;
	}
}
