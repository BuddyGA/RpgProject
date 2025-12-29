#pragma once

#include "core/RpgTypes.h"


class RpgRenderer;


namespace RpgRenderThread
{
	extern void Initialize() noexcept;
	extern void Shutdown() noexcept;
	extern void WaitFrame(int frameIndex) noexcept;
	extern void FrameRender(uint64_t frameCounter, int frameIndex, float deltaTime, RpgRenderer* renderer) noexcept;

}; // RpgRenderThread
