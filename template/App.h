#pragma once

class App
{
public:
	App();
	virtual ~App();

	static App& GetInstance() { return *s_pApp; }

	void SpawnObject();
	void SpawnParticles(XMFLOAT3 pos);
	void ObjectCollision();

	void OnStart();
	void OnUpdate();
	void OnExit();
	void OnRender(int pass);

	int Radiant(int degree);

	static void MyPixelShader(cpu_ps_io& io);
	static void ObjectShader(cpu_ps_io& io);

	struct gInfo
	{
		int score = 0;
		int life = 0;
	};

private:
	inline static App* s_pApp = nullptr;

	// Resources
	cpu_font m_font;
	cpu_mesh m_meshSphere;
	cpu_mesh m_meshObject;
	cpu_mesh m_meshCircle;
	cpu_mesh m_meshCenterCircle;
	
	// UI
	// Shader
	cpu_material m_materialCatcher;
	cpu_material m_materialObject;
	cpu_material m_materialCircle;

	// 3D
	cpu_entity* m_pCatcher;
	cpu_entity* m_pCircle;
	cpu_entity* m_pCenterCircle;
	std::list<cpu_entity*> m_object;
	std::list<cpu_particle_emitter*> m_pEmitter;
	float m_objectSpeed;

	// Gameplay
	float m_playerMove = 0.f;
	float second = 0;
	int difficulty = 4;

	// Info
	gInfo m_gInfo;

	// Camera
	cpu_camera m_p45Cam;
	cpu_camera m_p90Cam;

protected:
	cpu_fsm<App>* m_pLoop;
};
