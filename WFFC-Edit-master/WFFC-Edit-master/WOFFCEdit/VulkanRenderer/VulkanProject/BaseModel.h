#pragma once
#include <string>
#include <vector>
#include "Window.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include <TinyObjLoader/tiny_obj_loader.h>
#include <unordered_map>

namespace std {
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec4>()(vertex.pos) ^ (hash<glm::vec4>()(vertex.color) << 1)) >> 1);
		}
	};
}

class BaseMesh
{
public:
	BaseMesh();
	~BaseMesh();

	//Create model function (where vertices and indices are created)
	virtual void CreateModel();

	//Getters
	size_t GetVertexCount() { return vertices.size(); };
	size_t GetIndexCount() { return indices.size(); };
	vk::wrappers::Buffer* GetVertexBuffer() { return &vertexBuffer; };
	vk::wrappers::Buffer* GetIndexBuffer() { return &indicesBuffer; };
	VkDeviceSize GetVertexBufferSize() { return vertices.size() * sizeof(Vertex); };
	VkDeviceSize GetIndexBufferSize() { return indices.size() * sizeof(uint32_t); };
	void* GetVertexData() { return vertices.data(); };
	void* GetIndexData() { return indices.data(); };

protected:
	//Data for mesh (vertices, indices and material)
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	//buffers for mesh
	vk::wrappers::Buffer vertexBuffer;
	vk::wrappers::Buffer indicesBuffer;
};

