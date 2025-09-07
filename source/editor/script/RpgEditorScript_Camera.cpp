#include "RpgEditorScript_Camera.h"
#include "core/RpgInputSystem.h"
#include "core/world/RpgWorld.h"
#include "render/world/RpgRenderComponent.h"



RpgEditorScript_Camera::RpgEditorScript_Camera() noexcept
{
	Flashlight = nullptr;
	PitchValue = 75.0f;
	YawValue = 45.0f;
	bInitialized = false;

	PitchMin = -80.0f;
	PitchMax = 80.0f;
	RotationSpeed = 90.0f;
	MoveSpeed = 500.0f;
}


void RpgEditorScript_Camera::AttachedToGameObject() noexcept
{
	if (!bInitialized)
	{
		bInitialized = true;

		RpgTransform transform = GameObject.GetWorldTransform();
		transform.Position = RpgVector3(0.0f, 800.0f, 0.0f);
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(PitchValue, 45.0f, 0.0f);

		GameObject.SetWorldTransform(transform);

		Flashlight = &GameObject.AddComponent<RpgRenderComponent_Light>();
		Flashlight->Type = RpgRenderLight::TYPE_SPOT_LIGHT;
		Flashlight->ColorIntensity = RpgColorLinear(1.0f, 1.0f, 1.0f, 2.0f);
		Flashlight->AttenuationRadius = 1600.0f;
		Flashlight->SpotInnerConeDegree = 20.0f;
		Flashlight->SpotOuterConeDegree = 40.0f;
		Flashlight->bCastShadow = false;
		Flashlight->bIsVisible = false;
	}
}


void RpgEditorScript_Camera::TickUpdate(float deltaTime) noexcept
{
	RpgTransform transform = GameObject.GetWorldTransform();

	if (g_InputSystem->IsKeyButtonPressed(RpgInputKey::MOUSE_RIGHT))
	{
		RPG_Log(RpgLogTemp, "Camera FreeFly update movement BEGIN");
		RpgPlatformMouse::Capture(NULL, true);
		RpgPlatformMouse::SetCursorHidden(true);
		SavedMousePos = RpgPointFloat(RpgPlatformMouse::GetCursorPosition(NULL));
	}
	else if (g_InputSystem->IsKeyButtonReleased(RpgInputKey::MOUSE_RIGHT))
	{
		RPG_Log(RpgLogTemp, "Camera FreeFly update movement END");
		RpgPlatformMouse::SetCursorPosition(NULL, RpgPointInt(SavedMousePos));
		RpgPlatformMouse::Capture(NULL, false);
		RpgPlatformMouse::SetCursorHidden(false);
	}

	if (g_InputSystem->IsKeyButtonDown(RpgInputKey::MOUSE_RIGHT))
	{
		RpgVector3 moveAxis;

		if (g_InputSystem->IsKeyButtonDown(RpgInputKey::KEYBOARD_W))
		{
			moveAxis.Z = 1.0f;
		}

		if (g_InputSystem->IsKeyButtonDown(RpgInputKey::KEYBOARD_S))
		{
			moveAxis.Z = -1.0f;
		}

		if (g_InputSystem->IsKeyButtonDown(RpgInputKey::KEYBOARD_D))
		{
			moveAxis.X = 1.0f;
		}

		if (g_InputSystem->IsKeyButtonDown(RpgInputKey::KEYBOARD_A))
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

	GameObject.SetWorldTransform(transform);


	if (g_InputSystem->IsKeyButtonPressed(RpgInputKey::KEYBOARD_F))
	{
		Flashlight->bIsVisible = !Flashlight->bIsVisible;
	}
}
