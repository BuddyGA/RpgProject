#pragma once

#include "RpgRenderTypes.h"



class RpgTerrain
{
public:
	RpgTerrain() noexcept;
	

private:
	struct FTile
	{
		RpgBoundingAABB Bound;
	};

	RpgPointFloat WorldDimension;
	RpgPointInt TileDimension;

};
