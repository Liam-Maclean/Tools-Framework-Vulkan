#pragma once
#include "BaseModel.h"
class ImportedModel :
	public BaseModel
{
public:
	ImportedModel(std::string filePath)
		:BaseModel()
	{
		ImportedModel::LoadMeshFromFile(filePath);
	};

	~ImportedModel();

	//Method to load data into vertices datastructure
	void LoadMeshFromFile(std::string filePath);
};

