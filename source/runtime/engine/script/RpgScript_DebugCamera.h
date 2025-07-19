#pragma once

#include "core/RpgMath.h"
#include "core/dsa/RpgArray.h"
#include "core/world/RpgGameObject.h"


class RpgRenderComponent_Light;



class RpgScript_DebugCamera : public RpgGameObjectScript
{
	RPG_GAMEOBJECT_SCRIPT("RpgScript - DebugCamera")

public:
	float PitchMin;
	float PitchMax;
	float RotationSpeed;
	float MoveSpeed;


public:
	RpgScript_DebugCamera() noexcept;


	inline void GetRotationPitchYaw(float& out_Pitch, float& out_Yaw) const noexcept
	{
		out_Pitch = PitchValue;
		out_Yaw = YawValue;
	}

protected:
	virtual void AttachedToGameObject() noexcept override;
	virtual void TickUpdate(float deltaTime) noexcept override;


private:
	RpgRenderComponent_Light* Flashlight;
	float PitchValue;
	float YawValue;
	RpgPointFloat SavedMousePos;
	bool bInitialized;

};
