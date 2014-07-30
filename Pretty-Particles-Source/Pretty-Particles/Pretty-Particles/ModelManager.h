/*
	Models Manager will Load and Unload all vertex buffers and index buffers
	NOT NEEDED IN GAMEOBJECT TO UNLOAD THE MODEL
*/

#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "DxGraphics.h"
#include "VertexTypes.h"
#include "Utilities.h"

using std::vector;
using std::getline;
using std::ifstream;
using std::ios;

struct Model
{
	string filename;

	ID3D11Buffer* vBuffer;
	ID3D11Buffer* iBuffer;

	UINT vertexCount,
		indexCount,
		stride,
		offset;

	int vertexType;

	vector<VertexPosTex> vertices;

	void Release()
	{
		ReleaseCOM(vBuffer);
		ReleaseCOM(iBuffer);
	};
};

class ModelManager
{
public:
	ModelManager();
	~ModelManager();

	bool LoadContent(DxGraphics* dx);
	void UnloadContent();

	Model& GetModel(string filename);

private:
	vector<Model> m_models;

	Model m_errorModel;

	bool LoadModels(DxGraphics* dx, string filename); // Load the models from the file
	void LoadVertex(int vertexType, string line); // load vertex
	bool LoadBuffers(DxGraphics* dx, Model& model);

	//Create vectors for diffrent vertexType due to DX not allowing use of template vectors
	vector<VertexPos> m_verticesPos;
	vector<VertexPosNor> m_verticesPosNor;
	vector<VertexPosNorTex> m_verticesPosNorTex;
	vector<VertexPosTex> m_verticesPosTex;
	vector<unsigned int> m_indicies;

	enum m_vType {VERTEX_POS = 1, VERTEX_POS_NOR = 2, VERTEX_POS_TEX = 3,
		VERTEX_POS_NOR_TEX = 4};
};

#endif