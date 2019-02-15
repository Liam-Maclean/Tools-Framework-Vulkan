#pragma once
#include "BaseModel.h"
class PlaneMesh :
	public BaseMesh
{
public:
	PlaneMesh()
		:BaseMesh()
	{
		PlaneMesh::CreateModel();
	};
	~PlaneMesh();

	void CreateModel() override;
};

