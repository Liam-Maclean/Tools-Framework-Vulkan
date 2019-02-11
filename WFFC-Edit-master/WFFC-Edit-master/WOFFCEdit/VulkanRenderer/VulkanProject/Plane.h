#pragma once
#include "BaseModel.h"
class ScreenTarget :
	public BaseModel
{
public:
	ScreenTarget()
		:BaseModel()
	{
		ScreenTarget::CreateModel();
	};

	~ScreenTarget();

	void CreateModel() override;
};

