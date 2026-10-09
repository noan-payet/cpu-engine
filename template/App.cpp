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
	pObject->transform.SetScaling(0.2f);

	//float time = cpuTime.total;
	//pObject->transform.OrbitAroundAxis(m_pCenter->transform.pos, CPU_VEC3_UP, 3.f, time * 2.f);
	float angle = rand() % 360 + 1;
	angle = Radiant(angle);
	pObject->transform.SetPosition(cos(angle) * 2.85f, 10.f, sin(angle) * 2.85f);
	//pObject->transform.SetPosition(0,0,0);

	//pObject->transform.Move(1.5f);

	XMFLOAT3 particlePos = pObject->transform.pos;
	particlePos.y = 0.5f;

	SpawnParticles(particlePos);

	m_object.push_back(pObject);
}

void App::SpawnParticles(XMFLOAT3 pos)
{
	cpu_particle_emitter* pEmitter = cpuEngine.CreateParticleEmitter();
	pEmitter->rate = 0.1f;
	pEmitter->colorMin = cpu::ToColor(0, 255, 0);
	pEmitter->colorMax = cpu::ToColor(255, 0, 0);

	pEmitter->spread = 0.1f;
	pEmitter->speedMax = 0.75f;

	pEmitter->pos = pos;

	m_pEmitter.push_back(pEmitter);
}

void App::ObjectCollision()
{
	for (auto it = m_object.begin(); it != m_object.end(); ++it)
	{
		cpu_entity* pObject = *it;

		float oRadius = pObject->sphere.radius + pObject->transform.sca.y;
		float cRadius = m_pCatcher->sphere.radius + m_pCatcher->transform.sca.y;

		float dRadius = oRadius + cRadius;
		//dRadius = dRadius * dRadius;

		//XMFLOAT3 oPos = pObject->sphere.center;
		//XMFLOAT3 cPos = m_pCatcher->sphere.center;

		FXMVECTOR oPos = XMLoadFloat3(&pObject->sphere.center);
		GXMVECTOR cPos = XMLoadFloat3(&m_pCatcher->sphere.center);

		XMVECTOR vPos = cPos - oPos;
		vPos = vPos * vPos;
		float vX = XMVectorGetX(vPos);
		float vY = XMVectorGetY(vPos);
		float vZ = XMVectorGetZ(vPos);

		float dPos = vX + vY + vZ;

		if (dPos <= dRadius)
		{
			m_gInfo.score++;
			cpuEngine.Release(pObject);
		}
	}
}

void App::OnStart()
{
	// YOUR CODE HERE

	// Resources
	m_font.Create(cpuDevice.GetHeight() <= 512 ? 14 : 28);
	m_meshSphere.CreateSphere(2.0f, 12, 12);
	m_meshObject.CreateSphere(2.0f, 12, 12);
	XMFLOAT3 black = { 0,0,0 };
	m_meshCircle.CreateCircle(3.5f, 360, black);
	m_meshCenterCircle.CreateCircle(2.5f, 360);

	// UI
	// Shader
	m_materialCatcher.ps = MyPixelShader;
	m_materialObject.ps = ObjectShader;
	m_materialCircle.ps = MyPixelShader;

	// 3D
	m_pCircle = cpuEngine.CreateEntity();
	m_pCircle->transform.SetPosition(0.f, 0.f, 0.f);
	m_pCircle->pMesh = &m_meshCircle;
	m_pCircle->pMaterial = &m_materialCircle;

	m_pCenterCircle = cpuEngine.CreateEntity();
	m_pCenterCircle->transform.SetPosition(0.f, 0.5f, 0.f);
	m_pCenterCircle->pMesh = &m_meshCenterCircle;
	m_pCenterCircle->pMaterial = &m_materialCircle;

	m_pCatcher = cpuEngine.CreateEntity();
	m_pCatcher->pMesh = &m_meshSphere;
	m_pCatcher->pMaterial = &m_materialCatcher;
	m_pCatcher->transform.SetScaling(0.2f);

	m_objectSpeed = 10.f;

	// Player Info
	m_gInfo.life = 3;
	m_gInfo.score = 0;

	// Particle
	cpuEngine.GetParticleData()->Create(200000);
	cpuEngine.GetParticlePhysics()->gy = -0.5f;

	// Camera
	cpuEngine.GetCamera()->transform.SetPosition(0.f, 10.f, -8.f);

	m_p45Cam = *cpuEngine.GetCamera();
	m_p90Cam = *cpuEngine.GetCamera();

	m_p45Cam.transform.AddYPR(0.f, 45 * (XM_PI / 180));
	m_p90Cam.transform.AddYPR(0.f, 90 * (XM_PI / 180));

	cpuEngine.GetCamera()->transform.quat = m_p45Cam.transform.quat;
	cpuEngine.GetCamera()->transform.SetRotationFromQuaternion();
	
}

void App::OnUpdate()
{
	// YOUR CODE HERE
	float dt = cpuTime.delta;
	float time = cpuTime.total;

	// Camera Mode
	if (cpuInput.vi.IsKeyPressed(VK_F1))
	{
		cpuEngine.GetCamera()->transform.quat = m_p45Cam.transform.quat;
		cpuEngine.GetCamera()->transform.SetRotationFromQuaternion();

		cpuEngine.GetCamera()->transform.SetPosition(0.f, 10.f, -8.f);
	}
	if (cpuInput.vi.IsKeyPressed(VK_F2))
	{
		cpuEngine.GetCamera()->transform.quat = m_p90Cam.transform.quat;
		cpuEngine.GetCamera()->transform.SetRotationFromQuaternion();

		cpuEngine.GetCamera()->transform.SetPosition(0.f, 10.f * 3, 0.f);
	}

	// Player Move
	if (cpuInput.IsLeft())
		m_playerMove += dt * 2.f;
	if (cpuInput.IsRight())
		m_playerMove -= dt * 2.f;

	m_pCatcher->transform.SetPosition(cos(m_playerMove) * 3, 0.5f, sin(m_playerMove) * 3);

	// Object Spawn
	second += dt;

	if (second > difficulty)
	{
		SpawnObject();
		second = 0;
		if (difficulty != 2 && m_gInfo.score % 10 == 1)
			difficulty -= 1;
	}

	// Collision
	ObjectCollision();

	// Move missiles
	for (auto it = m_object.begin(); it != m_object.end(); ++it)
	{
		cpu_entity* pMissile = *it;
		pMissile->transform.pos.y -= dt;
	}

	// Purge missiles
	for (auto it = m_object.begin(); it != m_object.end(); )
	{
		cpu_entity* pMissile = *it;
		if (pMissile->transform.pos.y <= 0.5f)
		{
			m_gInfo.life--;

			for (auto itE = m_pEmitter.begin(); itE != m_pEmitter.end();)
			{
				cpu_particle_emitter* pEmitter = *itE;
				const XMFLOAT2 ePos = { pEmitter->pos.x, pEmitter->pos.z };
				const XMFLOAT2 oPos = { pMissile->transform.pos.x, pMissile->transform.pos.z };
				FXMVECTOR eVPos = XMLoadFloat2(&ePos);
				GXMVECTOR oVPos = XMLoadFloat2(&oPos);

				if (XMVector2Equal(eVPos, oVPos))
				{
					cpuEngine.Release(pEmitter);
					itE = m_pEmitter.erase(itE);
				}
				else
					++itE;
			}

			cpuEngine.Release(pMissile);
			it = m_object.erase(it);
		}
		else if ((*it)->dead)
		{
			for (auto itE = m_pEmitter.begin(); itE != m_pEmitter.end();)
			{
				cpu_particle_emitter* pEmitter = *itE;
				const XMFLOAT2 ePos = { pEmitter->pos.x, pEmitter->pos.z };
				const XMFLOAT2 oPos = { pMissile->transform.pos.x, pMissile->transform.pos.z };
				FXMVECTOR eVPos = XMLoadFloat2(&ePos);
				GXMVECTOR oVPos = XMLoadFloat2(&oPos);

				if (XMVector2Equal(eVPos, oVPos))
				{
					cpuEngine.Release(pEmitter);
					itE = m_pEmitter.erase(itE);
				}
				else
					++itE;
			}

			cpuEngine.Release(pMissile);
			it = m_object.erase(it);
		}
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
		info += CPU_STR(stats.drawnTriangleCount) + " triangles\n";
		/*info += CPU_STR(stats.clipEntityCount) + " clipped entities\n";
		info += CPU_STR(m_object.size()) + " missiles, ";
		info += CPU_STR(cpuEngine.GetParticleData()->alive) + " particles, ";
		info += CPU_STR(stats.threadCount) + " threads, ";
		info += CPU_STR(stats.tileCount) + " tiles,";
		info += " time " + CPU_STR(cpuTime.total);*/
		info += CPU_STR(m_gInfo.life) + " life, ";
		info += CPU_STR(m_gInfo.score) + " score";

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

int App::Radiant(int degree)
{
	return degree * XM_PI / 180;
}

void App::MyPixelShader(cpu_ps_io& io)
{
	// YOUR CODE HERE
	io.color = io.p.color;
}

void App::ObjectShader(cpu_ps_io& io)
{
	// garder seulement le rouge du pixel éclairé
	io.color.x = io.p.color.x;
}