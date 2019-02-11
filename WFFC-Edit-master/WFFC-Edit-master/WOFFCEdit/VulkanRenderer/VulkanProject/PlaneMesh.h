#pragma once
#include "BaseModel.h"
class PlaneMesh :
	public BaseModel
{
public:
	PlaneMesh()
		:BaseModel()
	{
		PlaneMesh::CreateModel();
	};
	~PlaneMesh();

	void CreateModel() override;
};

