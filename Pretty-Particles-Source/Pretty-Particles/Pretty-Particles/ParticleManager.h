#ifndef PARTICLE_MANAGER_H
#define PARTICLE_MANAGER_H

#include <iostream>
#include <math.h>

#include "DxGraphics.h"
#include "Camera.h"
#include "ResourceManager.h"

using std::vector;

//acts as both particle and vertices
struct Particle
{
	XMFLOAT3 pos;
	XMFLOAT3 vel;
	XMFLOAT4 colour;
};

struct MassPoint
{
	XMFLOAT3 pos;
	float pointMass;

	float particleMass;
	float dragForce;
	float gravityConst;
	float forceDistance;

	float dt;
	XMFLOAT3 pad;
};

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	bool LoadParticles(DxGraphics* pDxGraphics);
	void UpdateParticles(float dt, DxGraphics* pDxGraphics, Camera& cam);
	void RenderParticles(DxGraphics* pDxGraphics, Camera& cam);
	void ShutdownParticles();

	float GetParticleAmount();
	XMFLOAT3 GetParticleCenterPos();
	MassPoint GetMassPoint();
	void SetMassPoint(MassPoint massPoint);

private:
	bool BuildVertexList();
	bool BuildBuffers(ID3D11Device* pDevice);
	bool BuildShaders(ID3D11Device* pDevice);
	bool BuildInputLayout(ID3D11Device* pDevice);

	/// <ref> Code for this function was from MSDN - compile shaders </ref>
	bool CompileShader(ID3DBlob** shader, string filename, string entry, string shaderModel);

	unsigned int m_particleAmount;
	float m_spacing;
	MassPoint m_massPoint;

	//Vector for all the particles / vertices
	vector<Particle> m_particleList;

	//Buffer to hold all the particles
	ID3D11Buffer* m_vBuffer;
	//Stream out buffer
	ID3D11Buffer* m_vStream;

	//Buffer to hold the mouse position and the current mass of the point
	ID3D11Buffer* m_pointBuffer;
	//Buffer to hold the camera view / proj matrix
	ID3D11Buffer* m_camWorldBuffer;
	ID3D11Buffer* m_camViewBuffer;
	ID3D11Buffer* m_camProjBuffer;

	//Vertex Shader handles the render of particles
	ID3D11VertexShader* m_vShader;
	ID3DBlob* m_vBlob;
	ID3D11PixelShader* m_pShader;

	//Vertex shader handles the position of the particles and streams out
	ID3D11VertexShader* m_vUpdate;
	ID3D11GeometryShader* m_gUpdate;

	//World mat
	XMMATRIX m_worldMat;

	//Input layout to determine the particles into vertices
	ID3D11InputLayout* m_inputLayout;
};

#endif

