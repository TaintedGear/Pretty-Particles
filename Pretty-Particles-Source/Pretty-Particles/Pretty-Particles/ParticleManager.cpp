#include "ParticleManager.h"


ParticleManager::ParticleManager() 
: m_particleAmount(3000000),
m_vBuffer(0),
m_vStream(0),
m_inputLayout(0),
m_pointBuffer(0),
m_camWorldBuffer(0),
m_camViewBuffer(0),
m_camProjBuffer(0),
m_vBlob(0),
m_vShader(0),
m_pShader(0),
m_vUpdate(0),
m_gUpdate(0),
m_spacing(1.0f)
{
}

ParticleManager::~ParticleManager()
{
}

bool ParticleManager::LoadParticles(DxGraphics* pDxGraphics)
{
	if (!BuildVertexList())
	{
		return false;
	}
	if (!BuildBuffers(pDxGraphics->GetDevice()))
	{
		return false;
	}
	if (!BuildShaders(pDxGraphics->GetDevice()))
	{
		return false;
	}
	if (!BuildInputLayout(pDxGraphics->GetDevice()))
	{
		return false;
	}

	return true;
}

void ParticleManager::UpdateParticles(float dt, DxGraphics* pDxGraphics, Camera& cam)
{
	// Update the world mat
	XMMATRIX transMat = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	XMMATRIX scaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	m_worldMat = rotMat * scaleMat * transMat;

	cam.CalculateViewMatrix();

	// stream out the particles which runs through the physics
	XMMATRIX tWorld = XMMatrixTranspose(m_worldMat);
	XMMATRIX tView = XMMatrixTranspose(cam.GetViewMatrix());
	XMMATRIX tProj = XMMatrixTranspose(cam.GetProjMatrix());

	m_massPoint.dt = dt;

	ID3D11DeviceContext* pCon = pDxGraphics->GetImmediateContext();

	// Set the stream out buffer
	UINT offsetSO[1] = { 0 };
	pCon->SOSetTargets(1, &m_vStream, offsetSO);

	UINT stride = sizeof(Particle);
	UINT offset = 0;

	pCon->IASetVertexBuffers(0, 1, &m_vBuffer, &stride, &offset);
	pCon->IASetInputLayout(m_inputLayout);
	pCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	pCon->VSSetShader(m_vUpdate, 0, 0);
	pCon->GSSetShader(m_gUpdate, 0, 0);
	pCon->PSSetShader(NULL, 0, 0);

	//// Update the cBuffer
	pCon->UpdateSubresource(m_pointBuffer, 0, NULL, &m_massPoint, 0, 0);

	pCon->VSSetConstantBuffers(3, 1, &m_pointBuffer);

	pCon->Draw(m_particleList.size(), 0);

	pCon->VSSetShader(NULL, 0, 0);
	pCon->GSSetShader(NULL, 0, 0);
	pCon->PSSetShader(NULL, 0, 0);

	// Unbind the SO
	ID3D11Buffer* bufferArray[1] = { 0 };
	pCon->SOSetTargets(1, bufferArray, offsetSO);
	
	std::swap(m_vStream, m_vBuffer);
}

void ParticleManager::RenderParticles(DxGraphics* pDxGraphics, Camera& cam)
{
	cam.CalculateViewMatrix();

	XMMATRIX tWorld = XMMatrixTranspose(m_worldMat);
	XMMATRIX tView = XMMatrixTranspose(cam.GetViewMatrix());
	XMMATRIX tProj = XMMatrixTranspose(cam.GetProjMatrix());

	ID3D11DeviceContext* pCon = pDxGraphics->GetImmediateContext();
	
	UINT stride = sizeof(Particle);
	UINT offset = 0;

	pCon->IASetVertexBuffers(0, 1, &m_vBuffer, &stride, &offset);
	pCon->IASetInputLayout(m_inputLayout);
	pCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	pCon->VSSetShader(m_vShader, 0, 0);
	pCon->PSSetShader(m_pShader, 0, 0);

	pCon->UpdateSubresource(m_camWorldBuffer, 0, NULL, &tWorld, 0, 0);
	pCon->UpdateSubresource(m_camViewBuffer, 0, NULL, &tView, 0, 0);
	pCon->UpdateSubresource(m_camProjBuffer, 0, NULL, &tProj, 0, 0);

	pCon->VSSetConstantBuffers(0, 1, &m_camWorldBuffer);
	pCon->VSSetConstantBuffers(1, 1, &m_camViewBuffer);
	pCon->VSSetConstantBuffers(2, 1, &m_camProjBuffer);

	pCon->Draw(m_particleList.size(), 0);

	pCon->VSSetShader(NULL, 0, 0);
	pCon->GSSetShader(NULL, 0, 0);
	pCon->PSSetShader(NULL, 0, 0);
}

void ParticleManager::ShutdownParticles()
{
	if (m_vBuffer) m_vBuffer->Release();
	if (m_vStream) m_vStream->Release();
	if (m_inputLayout) m_inputLayout->Release();
	if (m_vBlob) m_vBlob->Release();
	if (m_vShader) m_vShader->Release();
	if (m_pShader) m_pShader->Release();
	if (m_vUpdate) m_vUpdate->Release();
	if (m_gUpdate) m_gUpdate->Release();
	if (m_pointBuffer) m_pointBuffer->Release();
	if (m_camWorldBuffer) m_camWorldBuffer->Release();
	if (m_camViewBuffer) m_camViewBuffer->Release();
	if (m_camProjBuffer) m_camProjBuffer->Release();
}

bool ParticleManager::BuildVertexList()
{
	int amountCubed = 0;
	amountCubed = pow(m_particleAmount, 1.0f / 3.0f);

	for (unsigned int x = 0; x < amountCubed; x++)
	{
		for (unsigned int y = 0; y < amountCubed; y++)
		{
			for (unsigned int z = 0; z < amountCubed; z++)
			{
				XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
				XMFLOAT3 vel = XMFLOAT3(0.0f, 0.0f, 0.0f);
				XMFLOAT4 col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

				Particle newParticle;

				//Find the particle position along a plane in the X & Y
				pos.x = (float)(x + m_spacing);
				pos.y = (float)(y + m_spacing);
				pos.z = (float)(z + m_spacing);

				newParticle.pos = pos;
				newParticle.vel = vel;
				newParticle.colour = col;

				m_particleList.push_back(newParticle);
			}
		}
	}

	//TEMP INIT OF THE MASS POINT
	float midPoint = (amountCubed / 2) * m_spacing;
	m_massPoint.pos = XMFLOAT3(midPoint, midPoint, midPoint);
	m_massPoint.pointMass = 10.0f;
	m_massPoint.particleMass = 1.0f;
	m_massPoint.dragForce = 0.997f;
	m_massPoint.forceDistance = 500.0f;
	m_massPoint.gravityConst = 16.6738480f;
	m_massPoint.dt = 0.0f;

	return true;
}

bool ParticleManager::BuildBuffers(ID3D11Device* pDevice)
{
	//Create the vertex buffer
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(D3D11_BUFFER_DESC));
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	vbd.ByteWidth = sizeof(Particle) * m_particleList.size();
	vbd.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vInit;
	ZeroMemory(&vInit, sizeof(D3D11_SUBRESOURCE_DATA));
	vInit.pSysMem = &m_particleList[0];

	HRESULT hr = 0;
	hr = pDevice->CreateBuffer(&vbd, &vInit, &m_vBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	//Create the stream out buffer
	hr = pDevice->CreateBuffer(&vbd, NULL, &m_vStream);
	if (FAILED(hr))
	{
		return false;
	}

	//Create the constant buffer for the mouse mass point
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(D3D11_BUFFER_DESC));
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.ByteWidth = sizeof(MassPoint);
	cbd.Usage = D3D11_USAGE_DEFAULT;

	hr = pDevice->CreateBuffer(&cbd, NULL, &m_pointBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	//Create the constant buffer for the wvp;
	cbd.ByteWidth = sizeof(XMMATRIX);
	hr = pDevice->CreateBuffer(&cbd, NULL, &m_camWorldBuffer);
	if (FAILED(hr))
	{
		return false;
	}
	hr = pDevice->CreateBuffer(&cbd, NULL, &m_camViewBuffer);
	if (FAILED(hr))
	{
		return false;
	}
	hr = pDevice->CreateBuffer(&cbd, NULL, &m_camProjBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool ParticleManager::BuildShaders(ID3D11Device* pDevice)
{
#pragma region COMPILE_RENDER_SHADER
	if (!CompileShader(&m_vBlob, "Data/particles.fx", "VS_MAIN", "vs_5_0"))
	{
		return false;
	}

	ID3DBlob* pBlob = 0;

	if (!CompileShader(&pBlob, "Data/particles.fx", "PS_MAIN", "ps_5_0"))
	{
		if (pBlob)
		{
			pBlob->Release();
		}

		return false;
	}
#pragma endregion

#pragma region COMPILE_UPDATE_SHADER

	ID3DBlob* vBlob = 0;

	if (!CompileShader(&vBlob, "Data/particles.fx", "VS_MAIN_SO", "vs_5_0"))
	{
		if (vBlob)
		{
			vBlob->Release();
		}

		return false;
	}

	ID3DBlob* gBlob = 0;

	if (!CompileShader(&gBlob, "Data/particles.fx", "GS_MAIN_SO", "gs_5_0"))
	{
		if (gBlob)
		{
			gBlob->Release();
		}

		return false;
	}

#pragma endregion

#pragma region CREATE_RENDER_SHADER
	
	HRESULT hr = 0;
	hr = pDevice->CreateVertexShader(m_vBlob->GetBufferPointer(), m_vBlob->GetBufferSize(),
		NULL, &m_vShader);
	if (FAILED(hr))
	{
		return false;
	}
	hr = pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
		NULL, &m_pShader);
	if (FAILED(hr))
	{
		pBlob->Release();
		return false;
	}

	pBlob->Release();

#pragma endregion

	hr = pDevice->CreateVertexShader(vBlob->GetBufferPointer(), vBlob->GetBufferSize(),
		NULL, &m_vUpdate);
	if (FAILED(hr))
	{
		vBlob->Release();
		return false;
	}

	//Stream out decl
	D3D11_SO_DECLARATION_ENTRY dso[] = {
		{ 0, "SV_POSITION", 0, 0, 3, 0 },
		{ 0, "VELOCITY", 0, 0, 3, 0 },
		{ 0, "COLOR", 0, 0, 4, 0 }
	};

	UINT stride = sizeof(Particle);
	UINT elems = sizeof(dso) / sizeof(D3D11_SO_DECLARATION_ENTRY);

	hr = pDevice->CreateGeometryShaderWithStreamOutput(
		gBlob->GetBufferPointer(), gBlob->GetBufferSize(),
		dso, elems, &stride, 1, 0, NULL, &m_gUpdate);
	if (FAILED(hr))
	{
		gBlob->Release();

		return false;
	}

	vBlob->Release();
	gBlob->Release();

	return true;
}

bool ParticleManager::BuildInputLayout(ID3D11Device* pDevice)
{
	//Make the particles a vertex for the pipeline
	D3D11_INPUT_ELEMENT_DESC idp[] =
	{
		{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HRESULT hr = 0;
	hr = pDevice->CreateInputLayout(idp, 3, m_vBlob->GetBufferPointer(), m_vBlob->GetBufferSize(),
		&m_inputLayout);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool ParticleManager::CompileShader(ID3DBlob** shader, string filename, string entry, string shaderModel)
{
	DWORD shaderFlags = 0;
	ID3DBlob* errorBuffer = 0;
	HRESULT hResult;

	//If debugging then compile shader in debug mode for error messages
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	//Compile the shader using the determined filename, entry point, shader model
	hResult = D3DX11CompileFromFile(filename.c_str(), 0, 0, entry.c_str(), shaderModel.c_str(),
		shaderFlags, 0, 0, shader, &errorBuffer, 0);

	//If the returned shader has errored then see what line in the .fx file
	if (errorBuffer != 0)
	{
		MessageBoxA(NULL, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		errorBuffer->Release();
		errorBuffer = 0;
	}

	//If the compile failed completely then return a DXTRACE msg to link back to this function call
	if (FAILED(hResult))
	{
		DXTRACE_MSG(__FILE__, (DWORD)__LINE__, hResult, "D3DX11CompileFromFile", true);
		return false;
	}

	return true;
}

MassPoint ParticleManager::GetMassPoint()
{
	return m_massPoint;
}
void ParticleManager::SetMassPoint(MassPoint massPoint)
{
	m_massPoint = massPoint;
}
float ParticleManager::GetParticleAmount()
{
	return m_particleList.size();
}
// Only refers to the center pos at its starting point
XMFLOAT3 ParticleManager::GetParticleCenterPos()
{
	int amountCubed = 0;
	amountCubed = pow(m_particleAmount, 1.0f / 3.0f);

	XMFLOAT3 centerPos = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float midPoint = (amountCubed / 2) * m_spacing;

	centerPos = XMFLOAT3(midPoint, midPoint, midPoint);

	return centerPos;
}