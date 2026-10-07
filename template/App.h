#pragma once

class App
{
public:
	App();
	virtual ~App();

	static App& GetInstance() { return *s_pApp; }

	void SpawnObject();

	void OnStart();
	void OnUpdate();
	void OnExit();
	void OnRender(int pass);

	static void MyPixelShader(cpu_ps_io& io);
	static void ObjectShader(cpu_ps_io& io);

private:
	inline static App* s_pApp = nullptr;

	// Resources
	cpu_font m_font;
	cpu_mesh m_meshSphere;
	cpu_mesh m_meshObject;
	
	// UI
	// Shader
	cpu_material m_materialCatcher;
	cpu_material m_materialObject;

	// 3D
	cpu_entity* m_pCatcher;
	cpu_entity* m_pCenter;
	std::list<cpu_entity*> m_object;
	float m_objectSpeed;

	// Gameplay
	float m_playerMove = 0.f;
	float second = 0;
	int difficulty = 4;
};
