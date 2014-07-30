#include "Camera.h"
Camera::Camera() 
{
	xRight = XMFLOAT3(1, 0, 0);
	xUp = XMFLOAT3(0, 1, 0);

	//initialize from garbage
	//m_QuatRot = XMQuaternionIdentity();
	//Offset the camera initially, otherwise multiuplication breaks
	//SetFocalPointAndPosition(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 1, 0));
}

Camera::~Camera()
{
	//destructor
}


void Camera::Update(float dt)
{

}

void Camera::Initialise(XMFLOAT3 position, float yaw, float pitch, float roll,
	float fov, float aspectRatio, XMFLOAT3 target)
{
	//Initalise position and  target
	SetFocalPointAndPosition(target, position);

	//SetUp Rotation
	RotYaw(yaw);
	RotPitch(pitch);
	RotRoll(roll);
	
	//calcualte the projMatrix
	RebuildProjMatrix(fov, aspectRatio);
}

void Camera::RebuildProjMatrix(float fov, float aspectRatio)
{
	m_projMat = XMMatrixPerspectiveFovLH(fov, aspectRatio, 0.1f, 10000.0f);
}


XMMATRIX Camera::GetProjMatrix() const
{
	return m_projMat;
}

void Camera::SetFocalPoint(XMFLOAT3 &v3FocalPoint)
{
	XMFLOAT3 xPos;
	XMStoreFloat3(&xPos, vPosition);
	SetFocalPointAndPosition(v3FocalPoint, xPos);
}

void Camera::SetFocalPointAndPosition(XMFLOAT3 &point, XMFLOAT3 &position)
{
	XMVECTOR vLookAt;
	XMVECTOR vUp, vRight;

	vFocalPoint = XMLoadFloat3(&point);
	vPosition = XMLoadFloat3(&position);

	vUp = XMLoadFloat3(&xUp);
	vRight = XMLoadFloat3(&xRight);

	//will determine the roattion origin
	vLookAt = vFocalPoint - vPosition;
	vLookAt = XMVector3Normalize(vLookAt);
	//determine right and up depend on the current position , determine the directions
	vRight = XMVector3Cross(vUp, vLookAt);
	vRight = XMVector3Normalize(vRight);

	vUp = XMVector3Cross(vLookAt, vRight);
	vUp = XMVector3Normalize(vUp);

	SetAxis(vLookAt, vUp, vRight);
}

void Camera::SetAxis(XMVECTOR vLookAt, XMVECTOR vUp, XMVECTOR vRight)
{
	XMMATRIX Rotation;
	Rotation = XMMatrixIdentity();
	//first column  X axis
	//second colums Y axis
	//third column Z axis
	Rotation(0, 0) = XMVectorGetX(vRight);
	Rotation(0, 1) = XMVectorGetX(vUp);
	Rotation(0, 2) = XMVectorGetX(vLookAt);

	Rotation(1, 0) = XMVectorGetY(vRight);
	Rotation(1, 1) = XMVectorGetY(vUp);
	Rotation(1, 2) = XMVectorGetY(vLookAt);

	Rotation(2, 0) = XMVectorGetZ(vRight);
	Rotation(2, 1) = XMVectorGetZ(vUp);
	Rotation(2, 2) = XMVectorGetZ(vLookAt);

	m_QuatRot = XMQuaternionRotationMatrix(Rotation);
}

XMMATRIX Camera::GetViewMatrix() const
{
	return m_viewMat;
}

void Camera::CalculateViewMatrix()
{
	XMMATRIX translationMat;

	m_viewMat = XMMatrixIdentity();
	translationMat = XMMatrixIdentity();

	//set up rotation matrix,viewmat is used as temp.
	m_viewMat = XMMatrixRotationQuaternion(m_QuatRot);
	//offesets to world origin
	translationMat = XMMatrixTranslation(XMVectorGetX(-vPosition), XMVectorGetY(-vPosition), XMVectorGetZ(-vPosition));
	//camera look at offsets the camera, allowing it to move around its own origin(lookat)
	m_viewMat = XMMatrixMultiply(translationMat, m_viewMat);
}

void Camera::MoveCamera(XMVECTOR &pos)
{
	vPosition += pos;
}

void Camera::SetPosition(XMFLOAT3 pos)
{
	vPosition = XMLoadFloat3( &pos );
}

XMVECTOR Camera::GetLookAt()
{
	XMMATRIX rot;
	rot = XMMatrixRotationQuaternion(m_QuatRot);

	XMFLOAT3 lookAt;
	lookAt = XMFLOAT3(rot(0, 0), rot(1, 1), rot(2, 1));

	return XMLoadFloat3(&lookAt);
}

XMVECTOR Camera::GetRightLeft()
{
	XMMATRIX rot;
	rot = XMMatrixRotationQuaternion(m_QuatRot);

	XMFLOAT3 Xaxis;
	Xaxis = XMFLOAT3(rot(0, 0), rot(1, 0), rot(2, 0));

	return XMLoadFloat3(&Xaxis);
}

XMVECTOR Camera::GetUpDown()
{
	XMMATRIX rot;
	rot = XMMatrixRotationQuaternion(m_QuatRot);

	XMFLOAT3 Yaxis;
	Yaxis = XMFLOAT3(rot(0, 1), rot(1, 1), rot(2, 1));

	return XMLoadFloat3(&Yaxis);
}

XMVECTOR Camera::GetForwardBackward()
{
	XMMATRIX rot;
	rot = XMMatrixRotationQuaternion(m_QuatRot);

	XMFLOAT3 Zaxis;
	Zaxis = XMFLOAT3(rot(0, 2), rot(1, 2), rot(2, 2));

	return XMLoadFloat3(&Zaxis);
}

void Camera::RotYaw(float angle)
{
	XMVECTOR vUp;
	XMFLOAT3 fUp = XMFLOAT3(0, 1, 0);
	vUp = XMLoadFloat3(&fUp);

	XMVECTOR quatRot;
	quatRot = XMQuaternionRotationAxis(vUp, -angle);
	m_QuatRot = XMQuaternionMultiply(quatRot, m_QuatRot);
}

void Camera::RotPitch(float angle)
{
	XMVECTOR vRight;
	XMFLOAT3 fRight = XMFLOAT3(1, 0, 0);
	vRight = XMLoadFloat3(&fRight);

	XMVECTOR quatRot;
	quatRot = XMQuaternionRotationAxis(vRight, angle);
	m_QuatRot = XMQuaternionMultiply(m_QuatRot, quatRot);
}

void Camera::RotRoll(float angle)
{
	XMVECTOR vRight;
	XMFLOAT3 fRight = XMFLOAT3(0, 0, 1);
	vRight = XMLoadFloat3(&fRight);

	XMVECTOR quatRot;
	quatRot = XMQuaternionRotationAxis(vRight, angle);
	m_QuatRot = XMQuaternionMultiply(m_QuatRot, quatRot);
}

XMFLOAT3 Camera::GetPosition()
{
	XMFLOAT3 f_Pos;
	XMStoreFloat3(&f_Pos, vPosition);
	return f_Pos;
}

