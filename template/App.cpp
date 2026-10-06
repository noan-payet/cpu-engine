#include "pch.h"

App::App()
{
	s_pApp = this;
	CPU_CALLBACK_START(OnStart);
	CPU_CALLBACK_UPDATE(OnUpdate);
	CPU_CALLBACK_EXIT(OnExit);
	CPU_CALLBACK_RENDER(OnRender);
}

App::~App()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App::SpawnObject()
{
	cpu_entity* pObject = cpuEngine.CreateEntity();
	pObject->pMesh = &m_meshObject;
	pObject->pMaterial = &m_materialObject;
	//pObject->transform.SetScaling(0.2f);

	//float time = cpuTime.total;
	//pObject->transform.OrbitAroundAxis(m_pCenter->transform.pos, CPU_VEC3_UP, 3.f, time * 2.f);
	//pObject->transform.SetPosition(cos(rand() % 6 + 1) * 3, 10.f, sin(rand() % 6 + 1) * 3);
	pObject->transform.pos = m_pCatcher->transform.pos;

	pObject->transform.LookAt(pObject->transform.pos.x, pObject->transform.pos.y - 10.f, pObject->transform.pos.z, CPU_VEC3_UP);
	//pObject->transform.Move(1.5f);
	m_object.push_back(pObject);
}

void App::OnStart()
{
	// YOUR CODE HERE

	// Resources
	m_font.Create(cpuDevice.GetHeight() <= 512 ? 14 : 28);
	m_meshSphere.CreateSphere(2.0f, 12, 12);
	m_meshObject.CreateSphere(2.0f, 12, 12);

	// UI
	// Shader
	m_materialCatcher.ps = MyPixelShader;
	m_materialObject.ps = MyPixelShader;

	// 3D
	m_pCenter = cpuEngine.CreateEntity();
	m_pCenter->transform.SetPosition(0.f, 0.f, 0.f);

	m_pCatcher = cpuEngine.CreateEntity();
	m_pCatcher->pMesh = &m_meshSphere;
	m_pCatcher->pMaterial = &m_materialCatcher;
	m_pCatcher->transform.SetScaling(0.2f);

	m_objectSpeed = 10.f;

	cpuEngine.GetCamera()->transform.SetPosition(0.f, 10.f *3, -8.f*3);
	cpuEngine.GetCamera()->transform.AddYPR(0.f, 45 * (XM_PI / 180));
}

void App::OnUpdate()
{
	// YOUR CODE HERE
	float dt = cpuTime.delta;
	float time = cpuTime.total;

	// Player Move
	if (cpuInput.IsLeft())
		m_playerMove -= dt * 2.f;
	if (cpuInput.IsRight())
		m_playerMove += dt * 2.f;

	m_pCatcher->transform.SetPosition(cos(m_playerMove) * 3, 0.f, sin(m_playerMove) * 3);

	// Object Spawn
	if (time / 1 == (int)time)
		SpawnObject();

	// Move missiles
	for (auto it = m_object.begin(); it != m_object.end(); ++it)
	{
		cpu_entity* pMissile = *it;
		pMissile->transform.Move(dt * m_objectSpeed);
		if (pMissile->lifetime > 10.0f)
			cpuEngine.Release(pMissile);
	}

	// Purge missiles
	for (auto it = m_object.begin(); it != m_object.end(); )
	{
		if ((*it)->dead)
			it = m_object.erase(it);
		else
			++it;
	}

	// Quit
	if (cpuInput.IsBackPressed())
		cpuEngine.Quit();
}

void App::OnExit()
{
	// YOUR CODE HERE
	m_object.clear();
}

void App::OnRender(int pass)
{
	// YOUR CODE HERE

	switch (pass)
	{
	case CPU_PASS_PARTICLE_BEGIN:
	{
		// Blur particles
		//cpuEngine.SetRT(m_rts[0]);
		//cpuEngine.ClearColor();
		break;
	}
	case CPU_PASS_PARTICLE_END:
	{
		// Blur particles
		//cpuEngine.Blur(10);
		//cpuEngine.SetMainRT();
		//cpuEngine.AlphaBlend(m_rts[0]);
		break;
	}
	case CPU_PASS_UI_END:
	{
		// Debug
		cpu_stats& stats = *cpuEngine.GetStats();
		std::string info = CPU_STR(cpuTime.fps) + " fps, ";
		info += CPU_STR(stats.drawnTriangleCount) + " triangles, ";
		info += CPU_STR(stats.clipEntityCount) + " clipped entities\n";
		info += CPU_STR(m_object.size()) + " missiles, ";
		info += CPU_STR(cpuEngine.GetParticleData()->alive) + " particles, ";
		info += CPU_STR(stats.threadCount) + " threads, ";
		info += CPU_STR(stats.tileCount) + " tiles";

		// Ray cast
		cpu_ray ray;
		cpuEngine.GetCursorRay(ray);
		cpu_hit hit;
		cpu_entity* pEntity = cpuEngine.HitEntity(hit, ray);
		if (pEntity)
		{
			info += "\nHIT: ";
			info += CPU_STR(pEntity->index).c_str();
		}

		XMFLOAT3 tint = { 1.0f, 1.0f, 0.8f };
		cpuDevice.DrawText(&m_font, info.c_str(), (int)(cpuDevice.GetWidth() * 0.5f), 10, CPU_TEXT_CENTER, &tint);
		break;
	}
	}
}

void App::MyPixelShader(cpu_ps_io& io)
{
	// YOUR CODE HERE
	io.color = io.p.color;
}
