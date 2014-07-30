#include "ResourceManager.h"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

bool ResourceManager::LoadContent(DxGraphics* dx)
{
	if(!m_modelManager.LoadContent(dx))
	{
		MessageBoxA(NULL, "Failed to load model Assets!", "ERROR!", MB_OK | MB_ICONERROR);
		return false;
	}
	if(!m_shaderManager.LoadContent(dx))
	{
		MessageBoxA(NULL, "Failed to load shader Assets!", "ERROR!", MB_OK | MB_ICONERROR);
		return false;
	}
	if(!m_textureManager.LoadContent(dx))
	{
		MessageBoxA(NULL, "Failed to load texture Assets!", "ERROR!", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

void ResourceManager::UnloadContent()
{
	m_modelManager.UnloadContent();
	m_shaderManager.UnloadContent();
	m_textureManager.UnloadContent();
}

ModelManager& ResourceManager::GetModels()
{
	return m_modelManager;
}
ShaderManager& ResourceManager::GetShaders()
{
	return m_shaderManager;
}
TextureManager& ResourceManager::GetTextures()
{
	return m_textureManager;
}

