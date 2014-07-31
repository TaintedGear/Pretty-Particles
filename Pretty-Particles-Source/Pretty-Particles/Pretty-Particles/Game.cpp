#include "Game.h"

Game::Game() : m_system(1024, 800), m_shutdown(false), m_escape(false), m_wireframe(false)
{
}

Game::~Game()
{
	//if(!m_shutdown)
	//{
	//	Shutdown();
	//}
}

bool Game::Initialise(HINSTANCE hInstance, HINSTANCE pInstance, 
	LPWSTR cmdLine, int cmdShow)
{
	//Call system to initialise all the libarys needed
	if(!m_system.Initialise(hInstance, pInstance, cmdLine, cmdShow))
	{
		return false;
	}

	return true;
}

void Game::Run()
{
	//if the game object fail to load then return before the loop is entered
	if(!LoadContent())
	{
		MessageBoxA(NULL, "Failed to load game objects", "ERROR!", MB_OK | MB_ICONERROR);
		return;
	}

	//Initalise the game timer
	m_system.GetGameTimer()->Reset();

	//The game loop is open until the system gets a WM_QUIT msg
	while(!m_system.Done() && !m_escape)
	{
		//starts the game timer
		m_system.GetGameTimer()->Tick();

		//Detect input
		m_system.GetDirectInput()->DetectInput();

		//Update the game
		Update(m_system.GetGameTimer()->GetDeltaTime());

		//change wireframe mode
		m_system.GetDX()->SetWireframeMode(m_wireframe);

		Render();
	}

	//Unload all the objects after the loop has closed
	UnloadContent();
}

void Game::Shutdown()
{
	if(!m_shutdown)
	{
		m_system.Shutdown();
		m_shutdown = true;
	}
}

bool Game::LoadContent()
{
	//Load all the game assets from the folders
	if(!m_resources.LoadContent(m_system.GetDX()))
	{
		return false;
	}

	if (!m_particleManager.LoadParticles(m_system.GetDX()))
	{
		return false;
	}

	XMFLOAT3 centerPos = m_particleManager.GetParticleCenterPos();
	centerPos.z = -300.0f;

	m_cam.Initialise(centerPos, 0.0f, 0.0f, 0.0f, XM_PIDIV4,
		m_system.GetAspectRatio(), m_particleManager.GetParticleCenterPos());

	return true;
}

void Game::UnloadContent()
{	
	m_particleManager.ShutdownParticles();
	m_resources.UnloadContent();
}

void Game::Update(float dt)
{
	if (m_system.GetDirectInput()->GetKeyboardState(DIK_ESCAPE))
	{
		m_escape = true;
	}

	//Move the mass point in the x and y
	MassPoint newPoint = m_particleManager.GetMassPoint();

	float pointSpeed = 30.0f;
	if (m_system.GetDirectInput()->GetKeyboardState(DIK_A))
	{
		newPoint.pos.x -= pointSpeed * dt;
	}
	if (m_system.GetDirectInput()->GetKeyboardState(DIK_D))
	{
		newPoint.pos.x += pointSpeed * dt;
	}
	if (m_system.GetDirectInput()->GetKeyboardState(DIK_W))
	{
		newPoint.pos.y += pointSpeed * dt;
	}
	if (m_system.GetDirectInput()->GetKeyboardState(DIK_S))
	{
		newPoint.pos.y -= pointSpeed * dt;
	}

	if (m_system.GetDirectInput()->GetKeyboardState(DIK_Q))
	{
		newPoint.pointMass += 1.0f * dt;
	}
	if (m_system.GetDirectInput()->GetKeyboardState(DIK_E))
	{
		newPoint.pointMass -= 1.0f * dt;
	}

	if (newPoint.pointMass < 0.1f)
	{
		newPoint.pointMass = 0.1f;
	}

	m_particleManager.SetMassPoint(newPoint);

	//m_cam.Update(dt, m_system.GetDirectInput(), m_system.GetWindowWidth(), m_system.GetWindowHeight());

	m_cam.Update(dt);

	m_particleManager.UpdateParticles(dt, m_system.GetDX(), m_cam);
}

void Game::Render()
{
	//Clear the depthstencil and render target view
	m_system.GetDX()->StartRender();

	m_particleManager.RenderParticles(m_system.GetDX(), m_cam);

	//Present all the object
	m_system.GetDX()->EndRender();
}

System Game::GetSystem()
{
	return m_system;
}
