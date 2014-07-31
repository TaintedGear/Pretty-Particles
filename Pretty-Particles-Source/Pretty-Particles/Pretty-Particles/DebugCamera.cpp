#include "DebugCamera.h"

DebugCamera::DebugCamera() : Camera(), m_rotate(0.0f), m_aimX(0.0f), m_aimY(0.0f), m_mouseSens(100.0f)
{
}

DebugCamera::~DebugCamera()
{
}

void DebugCamera::Initialise(XMFLOAT3 position, float yaw, float pitch, float roll,
	 float fov, float aspectRatio, XMFLOAT3 target)
{
	Camera::Initialise(position, yaw, pitch, roll,
		fov, aspectRatio, target);
}

void DebugCamera::Update(float dt, DirectInput* dInput, float windowWidth, float windowHeight)
{
	if(dInput->GetKeyboardState(DIK_UP))
	{
		XMVECTOR vMove = GetForwardBackward();
		vMove *= 100.0f * dt;
		MoveCamera(vMove);
	}

	if(dInput->GetKeyboardState(DIK_DOWN))
	{
		XMVECTOR vMove = -GetForwardBackward();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}

	if(dInput->GetKeyboardState(DIK_LEFT))
	{
		XMVECTOR vMove = -GetRightLeft();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}

	if(dInput->GetKeyboardState(DIK_RIGHT))
	{
		XMVECTOR vMove = GetRightLeft();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}

	m_aimX = m_mouseSens * (dInput->GetMouseX() / 1024) * dt;
	m_aimY = -m_mouseSens * (dInput->GetMouseY() / 800) * dt;

	RotPitch(m_aimY);
	RotYaw(m_aimX);
	
	Camera::Update(dt);
}