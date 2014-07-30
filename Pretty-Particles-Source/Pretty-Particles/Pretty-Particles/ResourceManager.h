#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "ModelManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	bool LoadContent(DxGraphics* dx);
	void UnloadContent();

	ModelManager& GetModels();
	ShaderManager& GetShaders();
	TextureManager& GetTextures();

private:

	ModelManager m_modelManager;
	ShaderManager m_shaderManager;
	TextureManager m_textureManager;
};

#endif