#pragma once

#include "core/world/RpgComponent.h"
#include "../RpgPhysicsTypes.h"



class RpgPhysicsComponent_Filter
{
	RPG_COMPONENT_TYPE("RpgComponent (Physics) - Filter");

public:
	// Object collision channel
	RpgPhysicsCollision::EChannel ObjectChannel;

	// Object response againts other channels
	RpgPhysicsCollision::FResponseChannels ResponseChannels;


public:
	RpgPhysicsComponent_Filter() noexcept
	{
		ObjectChannel = RpgPhysicsCollision::CHANNEL_NONE;
		ResponseChannels = RpgPhysicsCollision::DEFAULT_COLLISION_RESPONSE_CHANNELS_IgnoreAll;
	}


	inline void Destroy() noexcept
	{
		// Nothing to do
	}

};



class RpgPhysicsComponent_Collision
{
	RPG_COMPONENT_TYPE("RpgComponent (Physics) - Collision");

public:
	RpgPhysicsComponent_Collision() noexcept
	{
		Shape = RpgPhysicsCollision::SHAPE_NONE;
		bUpdateBounding = false;
	}


	inline void Destroy() noexcept
	{
		// Nothing to do
	}


	inline void SetShapeAs_Sphere(float radius) noexcept
	{
		Size = RpgVector4(radius);
		Shape = RpgPhysicsCollision::SHAPE_SPHERE;
		bUpdateBounding = true;
	}


	inline void SetShapeAs_Box(RpgVector3 halfExtents) noexcept
	{
		Size = RpgVector4(halfExtents.X, halfExtents.Y, halfExtents.Z, 0.0f);
		Shape = RpgPhysicsCollision::SHAPE_BOX;
		bUpdateBounding = true;
	}


	inline void SetShapeAs_Capsule(float radius, float halfHeight) noexcept
	{
		Size = RpgVector4(radius, halfHeight, 0.0f, 0.0f);
		Shape = RpgPhysicsCollision::SHAPE_CAPSULE;
		bUpdateBounding = true;
	}


	inline float GetSpeed() const noexcept
	{
		return Velocity.GetMagnitude();
	}


private:
	// Internal bounding sphere for broadphase
	RpgBoundingSphere Bound;

	// - Sphere (X = Radius, Y = Radius, Z = Radius, W = Radius)
	// - Box (XYZ = Half Extents, W = 0.0f)
	// - Capsule (X = Radius, Y = HalfHeight, Z = 0.0f, W = 0.0f)
	RpgVector4 Size;

	// Collision shape
	RpgPhysicsCollision::EShape Shape;

	// Linear velocity, rate of position change over time
	RpgVector3 Velocity;

	// Angular velocity, rate of orientation change over time
	RpgVector3 AngularVelocity;

	// Set true to update internal bounding AABB
	bool bUpdateBounding;


	friend RpgPhysicsWorldSubsystem;
	friend RpgPhysicsTask_UpdateBound;
	friend RpgPhysicsTask_UpdateShape;

};
