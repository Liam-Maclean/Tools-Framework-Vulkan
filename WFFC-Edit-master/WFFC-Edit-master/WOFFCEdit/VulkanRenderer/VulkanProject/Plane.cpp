#include "Plane.h"


ScreenTarget::~ScreenTarget()
{
}

void ScreenTarget::CreateModel()
{
	//Creates a Plane Mesh
	vertices.push_back({ { -1.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f }, {0.0f, -1.0f, 0.0f, 1.0f} });  //bottom left
	vertices.push_back({ { 1.0f, 1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, -1.0, 0.0f, 1.0f } });   //bottom  right
	vertices.push_back({ { 1.0f, -1.0f,0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f, 0.0f },{ 0.0f, -1.0, 0.0f, 1.0f } });   //top right
	vertices.push_back({ { -1.0f,-1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 0.0f },{ 0.0f, -1.0, 0.0f, 1.0f } });  //top left
	// Setup indices
	indices = { 0,1,2, 2,1,3 };
}
