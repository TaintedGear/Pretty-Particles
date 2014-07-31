/*
	Contains the game structure
*/
#ifndef GAME_H
#define GAME_H

#include <Windows.h>

#include "System.h"
#include "Timer.h"
#include "DebugCamera.h"
#include "ResourceManager.h"
#include "ParticleManager.h"

class Game
{
public:
	Game();
	~Game();

	bool Initialise(HINSTANCE hInstance, HINSTANCE pInstance,
		LPWSTR cmdLine, int cmdShow);
	void Run();
	void Shutdown();

	System GetSystem();
private:
	bool LoadContent();
	void UnloadContent();

	void Update(float dt);

	void Render();

	bool m_shutdown,
		m_escape,
		m_wireframe;

	System m_system;
	ResourceManager m_resources;
	ParticleManager m_particleManager;
	//DebugCamera m_cam;
	Camera m_cam;
};

#endif