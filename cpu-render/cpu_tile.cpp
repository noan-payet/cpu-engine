#include "pch.h"

void cpu_tile::Reset()
{
	// Entity
	statsDrawnTriangleCount = 0;

	// Particle
	for ( size_t i=0 ; i<particleLocalCounts.size() ; i++ )
		particleLocalCounts[i] = 0;
	particleCount = 0;
	particleOffset = 0;
	particleOffsetTemp = 0;
}
