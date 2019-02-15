#pragma once
#include "BaseModel.h"
class ScreenTarget :
	public BaseMesh
{
public:
	ScreenTarget()
		:BaseMesh()
	{
		ScreenTarget::CreateModel();
	};

	~ScreenTarget();

	void CreateModel() override;
};

