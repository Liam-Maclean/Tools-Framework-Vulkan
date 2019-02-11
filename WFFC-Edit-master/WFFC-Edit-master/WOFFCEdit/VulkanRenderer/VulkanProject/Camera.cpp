#include "Camera.h"


//Constructor
Camera::Camera()
{
}

//Constructor 
Camera::Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up, float fov, glm::vec2 aspectRatio, float near, float far)
{
	m_view = glm::lookAt(eye, center, up);
	m_projection = glm::perspective(glm::radians(fov), (float)aspectRatio.x / (float)aspectRatio.y, near, far);
	//Usually made for OpenGL where Y coordinate of the clip coordinates are invertex, this flips back
	m_projection[1][1] *= -1;

	m_far = far;
	m_near = near;
	m_cameraEye = eye;
	m_aspectRatio = aspectRatio;
	m_fov = fov;
	m_screenWidth = aspectRatio.x;
	m_screenHeight = aspectRatio.y;
}

//Deconstructor
Camera::~Camera()
{
}

//Handles camera input and recalculates the view and projection matrices
void Camera::HandleInput(GLFWwindow* window)
{

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwSetCursorPos(window, m_screenWidth / 2, m_screenHeight / 2);
	//xpos = m_screenWidth / 2;
	//ypos = m_screenWidth / 2;
	//float xoffSet = (xpos - (m_screenWidth / 2)) * mouseSpeed * 0.016f;
	//float yoffSet = (ypos - (m_screenHeight / 2)) * mouseSpeed * 0.016f;

	
	//horizontalAngle += xoffSet;
	//verticalAngle += yoffSet;

	// Compute new orientation
	horizontalAngle += mouseSpeed * 0.016f * float(m_screenWidth / 2 - xpos);
	verticalAngle += mouseSpeed * 0.016f * float(m_screenHeight / 2 - ypos);

	//glm::vec3 direction(0.0f, 0.0f, -1.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 direction(
		cos(glm::radians(verticalAngle)) * sin(glm::radians(horizontalAngle)),
		sin(glm::radians(verticalAngle)),
		cos(glm::radians(verticalAngle)) * cos(glm::radians(horizontalAngle))
	);

	//
	//// Right vector
	glm::vec3 right = glm::cross(direction, up);
	//glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
	//glm::vec3 right(sin(horizontalAngle - 3.14f/2.0f),0,cos(horizontalAngle - 3.14f/2.0f));

	// Up vector : perpendicular to both direction and right
	
	//glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		m_cameraEye += direction * 0.016f * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		m_cameraEye -= direction * 0.016f * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		m_cameraEye += right * 0.016f * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		m_cameraEye -= right * 0.016f * speed;
	}

	//Recreate the view and projection matrices
	m_view = glm::lookAt(m_cameraEye, m_cameraEye + direction, up);
	m_projection = glm::perspective(glm::radians(m_fov), (float)m_aspectRatio.x / (float)m_aspectRatio.y, m_near, m_far);

	//Usually made for OpenGL where Y coordinate of the clip coordinates are invertex, this flips back
	m_projection[1][1] *= -1;
}

