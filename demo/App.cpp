#include "pch.h"

App::App()
{
	s_pApp = this;
	CPU_CALLBACK_START(OnStart);
	CPU_CALLBACK_UPDATE(OnUpdate);
	CPU_CALLBACK_EXIT(OnExit);
	CPU_CALLBACK_RENDER(OnRender);

	m_pShip = nullptr;
}

App::~App()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App::SpawnMissile()
{
	cpu_entity* pMissile = cpuEngine.CreateEntity();
	pMissile->pMesh = &m_meshMissile;
	pMissile->transform.SetScaling(0.2f);
	pMissile->transform.pos = m_pShip->GetEntity()->transform.pos;
	pMissile->transform.SetRotation(m_pShip->GetEntity()->transform);
	pMissile->transform.Move(1.5f);
	m_missiles.push_back(pMissile);
}

void App::SpawnMissileWithMouse()
{
	cpu_ray ray;
	cpuEngine.GetCursorRay(ray);
	cpu_entity* pMissile = cpuEngine.CreateEntity();
	pMissile->pMesh = &m_meshMissile;
	pMissile->transform.SetScaling(0.2f);
	pMissile->transform.pos = ray.pos;
	pMissile->transform.LookTo(ray.dir);
	pMissile->transform.Move(1.5f);
	pMissile->pMaterial = &m_materialMissile;
	m_missiles.push_back(pMissile);
}

void App::SpawnAsteroid()
{
	float time = cpuTime.total;

	cpu_entity* pMissile = cpuEngine.CreateEntity();
	pMissile->pMesh = &m_meshMissile;
	pMissile->transform.SetScaling(0.2f);
	pMissile->transform.OrbitAroundAxis(m_pEarth->transform.pos, m_pEarth->transform.up, 7.f, time + 0.1f * (rand() % 2 + -1));
	pMissile->transform.LookAt(m_pEarth->transform.pos.x, m_pEarth->transform.pos.y, m_pEarth->transform.pos.z, m_pEarth->transform.up);
	pMissile->transform.Move(1.5f);
	pMissile->pMaterial = &m_materialMissile;
	m_missiles.push_back(pMissile);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App::OnStart()
{
	// YOUR CODE HERE

	// Render
	//cpuEngine.EnableBoxRender();

	// Resources
	m_font.Create(cpuDevice.GetHeight()<=512 ? 14 : 28);
	m_textureBird.Load("bird_amiga.png");
	m_textureEarth.Load("earth.png");
	m_meshShip.CreateSpaceship();
	m_meshMissile.CreateSphere(0.5f);
	m_meshSphere.CreateSphere(2.0f, 12, 12);
	m_rts[0] = cpuEngine.CreateRT();

	// UI
	m_pSprite = cpuEngine.CreateSprite();
	m_pSprite->pTexture = &m_textureBird;
	m_pSprite->CenterAnchor();
	m_pSprite->x = 40;
	m_pSprite->y = 0;

	// Shader
	m_materialShip.color = cpu::ToColor(255, 128, 0);
	m_materialMissile.ps = MissileShader;
	m_materialMoon.ps = MoonShader;
	m_materialEarth.pTexture = &m_textureEarth;

	// 3D
	m_missileSpeed = 10.0f;
	m_pEarth = cpuEngine.CreateEntity();
	m_pEarth->pMesh = &m_meshSphere;
	m_pEarth->pMaterial = &m_materialEarth;
	m_pEarth->transform.pos.x = 3.0f;
	m_pEarth->transform.pos.y = 3.0f;
	m_pEarth->transform.pos.z = 5.0f;
	m_pMoon = cpuEngine.CreateEntity();
	m_pMoon->pMesh = &m_meshSphere;
	m_pMoon->pMaterial = &m_materialMoon;
	m_pMoon->transform.SetScaling(0.1f);

	// Ship
	m_pShip = new Ship;
	m_pShip->Create(&m_meshShip, &m_materialShip);
	m_pShip->GetFSM()->ToState(CPU_ID(StateShipIdle));

	// Particle
	cpuEngine.GetParticleData()->Create(2000000);
	cpuEngine.GetParticlePhysics()->gy = -0.5f;
	m_pEmitter = cpuEngine.CreateParticleEmitter();
	m_pEmitter->rate = 1.0f;
	m_pEmitter->colorMin = cpu::ToColor(255, 0, 0);
	m_pEmitter->colorMax = cpu::ToColor(255, 128, 0);
	m_pEmitter2 = cpuEngine.CreateParticleEmitter();
	m_pEmitter2->rate = 0.25f;
	m_pEmitter2->colorMin = cpu::ToColor(0, 0, 255);
	m_pEmitter2->colorMax = cpu::ToColor(0, 128, 255);
	m_pEmitter2->pos.x = -2.0f;

	// Test
	//m_pEmitter->blend = CPU_PARTICLE_OPAQUE;
	//m_pEmitter->colorMin = cpu::ToColor(0, 0, 0);
	//m_pEmitter->colorMax = cpu::ToColor(16, 16, 16);

	// Debug: texture
	//float roomSize = 100.0f;
	//cpu_mesh* pMesh = new cpu_mesh;
	//pMesh->CreatePlane(roomSize, roomSize);
	//XMMATRIX matrix = XMMatrixRotationX(XM_PIDIV2);
	//pMesh->Transform(matrix);
	//matrix = XMMatrixTranslation(0.0f, -2.0f, 0.0f);
	//pMesh->Transform(matrix);
	//pMesh->Optimize();
	//cpu_entity* pE = cpuEngine.CreateEntity();
	//pE->pMesh = pMesh;
	//pE->pMaterial = new cpu_material;
	//pE->pMaterial->pTexture = &m_textureEarth;

	// Camera
	//cpuEngine.GetCamera()->transform.pos.z = -5.0f;
	m_shipCam = *cpuEngine.GetCamera();
	m_moonCam = *cpuEngine.GetCamera();
}

void App::OnUpdate()
{
	// YOUR CODE HERE

	float dt = cpuTime.delta;
	float time = cpuTime.total;

	// Move sprite
	m_pSprite->y = 60 + cpu::RoundToInt(sinf(time)*20.0f);

	// Turn earth
	m_pEarth->transform.AddYPR(-dt);

	// Move rock
	float* pMoonPos = &moonPos;

	// m_missiles

	if (m_CamMode == 0)
	{
		m_pMoon->transform.OrbitAroundAxis(m_pEarth->transform.pos, m_pEarth->transform.up, 3.0f, time * 2.0f);
		*pMoonPos = time * 2.0f;
	}
		
	if (m_CamMode == 1)
	{
		m_pMoon->transform.OrbitAroundAxis(m_pEarth->transform.pos, m_pEarth->transform.up, 3.0f, moonPos);
		SpawnAsteroid();
	}
		
	m_pEmitter->pos = m_pMoon->transform.pos;

	if (m_pMoon->transform.dir.x != 0 && m_pMoon->transform.dir.z != 0)
	{
		m_pEmitter->dir = m_pMoon->transform.dir;
		m_pEmitter->dir.x = -m_pEmitter->dir.x;
		m_pEmitter->dir.y = -m_pEmitter->dir.y;
		m_pEmitter->dir.z = -m_pEmitter->dir.z;
	}

	// Turn camera
	if (m_CamMode == 0)
	{
		cpuEngine.GetCamera()->transform.quat = m_pShip->GetEntity()->transform.quat;
		cpuEngine.GetCamera()->transform.SetRotationFromQuaternion();
	}
	if (m_CamMode == 1)
	{
		cpuEngine.GetCamera()->transform.quat = m_pMoon->transform.quat;
		cpuEngine.GetCamera()->transform.SetRotationFromQuaternion();
	}
	

	//float yaw = m_pShip->GetEntity()->transform.quat.y;
	//float pitch = m_pShip->GetEntity()->transform.quat.x;
	//float roll = m_pShip->GetEntity()->transform.quat.z;

	//if (yaw < -0.1f && yaw > 0.1f)
	//	yaw * -1.f * dt;
	//if (pitch < -0.1f && pitch > 0.1f)
	//	pitch * -1.f * dt;
	//if (roll < -0.1f && roll > 0.1f)
	//	roll * -1.f * dt;

	//cpuEngine.GetCamera()->transform.AddYPR(yaw, pitch, roll);

	float ship_x = m_pShip->GetEntity()->transform.pos.x - m_pShip->GetEntity()->transform.dir.x * 10.0f;
	float ship_y = (m_pShip->GetEntity()->transform.pos.y - m_pShip->GetEntity()->transform.dir.y * 10.0f) + 2.5f;
	float ship_z = (m_pShip->GetEntity()->transform.pos.z - m_pShip->GetEntity()->transform.dir.z * 10.0f) - 3.f;
	m_shipCam.transform.SetPosition(ship_x, ship_y, ship_z);

	float moon_x = m_pEarth->transform.pos.x;
	float moon_y = m_pEarth->transform.pos.y + 20.f;
	float moon_z = m_pEarth->transform.pos.z;
	m_moonCam.transform.SetPosition(moon_x, moon_y, moon_z);

	if (m_CamMode == 0)
		cpuEngine.GetCamera()->transform.pos = m_shipCam.transform.pos; time = moonPos;
	if (m_CamMode == 1)
	{
		float Rad = 90 * (XM_PI / 180);
		cpuEngine.GetCamera()->transform.pos = m_moonCam.transform.pos;
		cpuEngine.GetCamera()->transform.AddYPR(0.f, Rad);
	}

	if (cpuInput.vi.IsKeyPressed(VK_F1))
		m_CamMode = 0;
	if (cpuInput.vi.IsKeyPressed(VK_F2))
		m_CamMode = 1;

	// Move
	switch (m_CamMode)
	{
	case 0:
	{
		if (cpuInput.IsUp())
			m_pShip->GetEntity()->transform.AddYPR(0, dt * 1.f);
		if (cpuInput.IsDown())
			m_pShip->GetEntity()->transform.AddYPR(0, dt * -1.f);
		if (cpuInput.IsLeft())
		{
			m_pShip->GetEntity()->transform.AddYPR(0, 0, dt * 1.f);
			//cpuEngine.GetCamera()->transform.AddYPR(-dt*XM_PI);

			m_pShip->GetEntity()->transform.pos.x -= dt * 1.0f;
		}


		if (cpuInput.IsRight())
		{
			m_pShip->GetEntity()->transform.AddYPR(0, 0, dt * -1.f);
			//cpuEngine.GetCamera()->transform.AddYPR(dt*XM_PI);

			m_pShip->GetEntity()->transform.pos.x += dt * 1.0f;
		}
		break;
	}
	case 1:
	{
		if (cpuInput.IsLeft())
			moonPos -= dt;
		if (cpuInput.IsRight())
			moonPos += dt;
		break;
	}
	default:
		break;
	}
	
	

	// Move missiles
	for ( auto it=m_missiles.begin() ; it!=m_missiles.end() ; ++it )
	{
		cpu_entity* pMissile = *it;
		pMissile->transform.Move(dt*m_missileSpeed);
		if ( pMissile->lifetime>10.0f )
			cpuEngine.Release(pMissile);
	}

	// Fire
	if ( cpuInput.IsActionPressed() || cpuInput.IsAction(1) )
		cpuApp.SpawnMissileWithMouse();

	// Purge missiles
	for ( auto it=m_missiles.begin() ; it!=m_missiles.end() ; )
	{
		if ( (*it)->dead )
			it = m_missiles.erase(it);
		else
			++it;
	}

	// Quit
	if ( cpuInput.IsBackPressed() )
		cpuEngine.Quit();
}

void App::OnExit()
{
	// YOUR CODE HERE

	if ( m_pShip )
		m_pShip->Destroy();
	CPU_DELPTR(m_pShip);
	m_missiles.clear();
}

void App::OnRender(int pass)
{
	// YOUR CODE HERE

	switch ( pass )
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
			info += CPU_STR(m_missiles.size()) + " missiles, ";
			info += CPU_STR(cpuEngine.GetParticleData()->alive) + " particles, ";
			info += CPU_STR(stats.threadCount) + " threads, ";
			info += CPU_STR(stats.tileCount) + " tiles";

			// Ray cast
			cpu_ray ray;
			cpuEngine.GetCursorRay(ray);
			cpu_hit hit;
			cpu_entity* pEntity = cpuEngine.HitEntity(hit, ray);
			if ( pEntity )
			{
				info += "\nHIT: ";
				info += CPU_STR(pEntity->index).c_str();
			}

			XMFLOAT3 tint = { 1.0f, 1.0f, 0.8f };
			cpuDevice.DrawText(&m_font, info.c_str(), (int)(cpuDevice.GetWidth()*0.5f), 10, CPU_TEXT_CENTER, &tint);
			break;
		}
	}
}

void App::MissileShader(cpu_ps_io& io)
{
	// garder seulement le rouge du pixel éclairé
	io.color.x = io.p.color.x;
}

void App::MoonShader(cpu_ps_io& io)
{
	float time = cpuTime.total;
	float scale = ((sinf(time*3.0f)*0.5f)+0.5f) * 0.5f + 0.5f; 
	io.color.x = io.p.color.x * scale;
	io.color.y = io.p.color.y * scale;
	io.color.z = io.p.color.z;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ship::Ship()
{
	m_pEntity = nullptr;
	m_pFSM = nullptr;
}

Ship::~Ship()
{
}

void Ship::Create(cpu_mesh* pMesh, cpu_material* pMaterial)
{
	m_pEntity = cpuEngine.CreateEntity();
	m_pEntity->pMesh = pMesh;
	m_pEntity->pMaterial = pMaterial;
	m_pEntity->transform.pos.z = 5.0f;
	m_pEntity->transform.pos.y = -3.0f;

	m_pFSM = cpuEngine.CreateFSM(this);
	m_pFSM->SetPostGlobal<StateShipGlobal>();
	m_pFSM->Add<StateShipIdle>();
	m_pFSM->Add<StateShipBlink>();
}

void Ship::Destroy()
{
	m_pFSM = cpuEngine.Release(m_pFSM);
	m_pEntity = cpuEngine.Release(m_pEntity);
}

void Ship::Update()
{
	float dt = cpuTime.delta;

	// Turn ship
	//m_pEntity->transform.AddYPR(dt, dt, dt);

	// Move ship
	m_pEntity->transform.pos.x += m_pEntity->transform.dir.x * dt;
	m_pEntity->transform.pos.y += m_pEntity->transform.dir.y * dt;
	m_pEntity->transform.pos.z += m_pEntity->transform.dir.z * dt;

	// Fire
	if ( cpuInput.vi.IsKey(VK_SPACE) )
		cpuApp.SpawnMissile();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void StateShipGlobal::OnEnter(Ship& cur, int from, void* pParam)
{
}

void StateShipGlobal::OnExecute(Ship& cur)
{
	cur.Update();
}

void StateShipGlobal::OnExit(Ship& cur, int to)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void StateShipIdle::OnEnter(Ship& cur, int from, void* pParam)
{
}

void StateShipIdle::OnExecute(Ship& cur)
{
	// Blink every 3 seconds
	if ( cur.GetFSM()->totalTime>3.0f )
	{
		cur.GetFSM()->ToState(CPU_ID(StateShipBlink));
		return;
	}
}

void StateShipIdle::OnExit(Ship& cur, int to)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void StateShipBlink::OnEnter(Ship& cur, int from, void* pParam)
{
}

void StateShipBlink::OnExecute(Ship& cur)
{
	float v = fmod(cur.GetFSM()->totalTime, 0.2f);
	if ( v<0.1f )
	{
		cur.GetEntity()->visible = true;
	}
	else
	{
		cur.GetEntity()->visible = false;
	}

	if ( cur.GetFSM()->totalTime>1.0f )
	{
		cur.GetFSM()->ToState(CPU_ID(StateShipIdle));
		return;
	}
}

void StateShipBlink::OnExit(Ship& cur, int to)
{
}
