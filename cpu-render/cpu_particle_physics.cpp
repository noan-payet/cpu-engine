#include "pch.h"

cpu_particle_physics::cpu_particle_physics()
{
	gx = 0.0f;
	gy = -1.0f;
	gz = 0.0f;
	drag = 0.0f;
	maxSpeed = 0.0f;
	minX = 0.0f;
	minY = 0.0f;
	minZ = 0.0f;
	maxX = 0.0f;
	maxY = 0.0f;
	maxZ = 0.0f;
	useBounds = false;
	bounce = 0.0f;
}
