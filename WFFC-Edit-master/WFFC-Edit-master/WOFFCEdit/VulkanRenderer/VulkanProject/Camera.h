#include <glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"
#define GLM_FORCE_RADIANS
#pragma once
class Camera
{
public:
	Camera();
	Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up, float fov, glm::vec2 aspectRatio, float near, float far);
	~Camera();

	void HandleInput(GLFWwindow* window);

	glm::mat4 GetViewMatrix() { return m_view; }
	glm::mat4 GetProjectionMatrix() { return m_projection; }
	glm::vec3 GetCameraEye() { return m_cameraEye; }
	 


private:

	float speed = 3.0f; // 3 units / second
	float mouseSpeed = 2.0f;
	// horizontal angle : toward -Z
	float horizontalAngle = -90.0f;
	// vertical angle : 0, look at the horizon
	float verticalAngle = 0.0f;


	glm::mat4 m_view;
	glm::mat4 m_projection;
	glm::vec3 m_cameraEye;
	glm::vec2 m_aspectRatio;
	float m_fov;
	float m_near, m_far;
	float m_screenWidth, m_screenHeight;

};

