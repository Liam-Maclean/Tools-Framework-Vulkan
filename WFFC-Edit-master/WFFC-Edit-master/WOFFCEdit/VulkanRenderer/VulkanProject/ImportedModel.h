#pragma once
#include "BaseModel.h"
class ImportedModel :
	public BaseMesh
{
public:
	ImportedModel(std::string filePath)
		:BaseMesh()
	{
		ImportedModel::LoadMeshFromFile(filePath);
	};

	~ImportedModel();

	//Method to load data into vertices datastructure
	void LoadMeshFromFile(std::string filePath);
};

