#include "PlaneMesh.h"


PlaneMesh::~PlaneMesh()
{
}

void PlaneMesh::CreateModel()
{
	//Creates a Plane Mesh
	vertices.push_back({ {-1.0f,  0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } });  //top left
	vertices.push_back({ { 1.0f,  0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } });  //top right
	vertices.push_back({ { 1.0f,  0.0f,-1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } });  //bottom right
	vertices.push_back({ {-1.0f,  0.0f,-1.0f, 1.0f },{ 1.0f, 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } });  //bottom left
																				
	indices = { 0,3,2, 2,1,0 }; // Setup indices
}
