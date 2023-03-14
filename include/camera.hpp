#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PITCH_LIMIT 89.0f

typedef enum _move_direction {
	FRONT,
	BACK,
	UP,
	DOWN,
	LEFT,
	RIGHT
}direction;

class camera
{
private:
	GLfloat moveSpeed;
	GLfloat viewSpeed;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat zoom;

	glm::vec3 up;
	glm::vec3 front;
	glm::vec3 right;

	glm::vec3 position;
	glm::vec3 worldUp;

	bool lookAtShouldUpdate;
	bool perspectiveShouldUpdate;

	glm::mat4 lookAtCache;
	glm::mat4 perspectiveCache;

	GLfloat lastRatio;

	void renewVector()
	{
		front.x = glm::cos(glm::radians(pitch)) * glm::cos(glm::radians(yaw));
		front.y = glm::sin(glm::radians(pitch));
		front.z = glm::cos(glm::radians(pitch)) * glm::sin(glm::radians(yaw));
		front = glm::normalize(front);
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
		lookAtShouldUpdate = true;
	}
public:
	camera(const glm::vec3 &position, GLfloat yaw = 0.0f, GLfloat pitch = 0.0f, GLfloat zoom = 45.0f, GLfloat moveSpeed = 4.0f, GLfloat viewSpeed = 0.02f):
	position(position),
	yaw(yaw),
	pitch(pitch),
	zoom(zoom),
	moveSpeed(moveSpeed),
	viewSpeed(viewSpeed),
	worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	lookAtShouldUpdate(true),
	perspectiveShouldUpdate(true),
	lastRatio(0.0f)
	{
		renewVector();
	}

	void move(direction d, GLfloat delta)
	{
		switch (d)
		{
			case FRONT:
			position += delta * moveSpeed * front;
			break;
			case BACK:
			position -= delta * moveSpeed * front;
			break;
			case LEFT:
			position -= delta * moveSpeed * right;
			break;
			case RIGHT:
			position += delta * moveSpeed * right;
			break;
			default:
			throw "Argument error.";
		}
		lookAtShouldUpdate = true;
	}

	void view(GLfloat xoffset, GLfloat yoffset)
	{
		xoffset *= viewSpeed;
		yoffset *= viewSpeed;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > PITCH_LIMIT)
			pitch = PITCH_LIMIT;
		if (pitch < -PITCH_LIMIT)
			pitch = -PITCH_LIMIT;

		if (yaw > 360.0f)
			yaw -= 360.0f;
		if (yaw < 0.0f)
			yaw += 360.0f;

		renewVector();
	}

	void zoomChange(GLfloat yoffset)
	{
		zoom += yoffset * viewSpeed;
		if (zoom > 60.0f)
			zoom = 60.0f;
		if (zoom < 5.0f)
			zoom = 5.0f;

		perspectiveShouldUpdate = true;
	}

	glm::mat4 getLookAt()
	{
		if (lookAtShouldUpdate)
		{
			lookAtCache = glm::lookAt(position, position + front, up);
			lookAtShouldUpdate = false;
		}
		return lookAtCache;
	}

	glm::mat4 getPerspective(GLfloat HWratio)
	{
		if (lastRatio == 0.0f || lastRatio != HWratio)
		{
			perspectiveShouldUpdate = true;
		}
		if (perspectiveShouldUpdate)
		{
			perspectiveCache = glm::perspective(glm::radians(zoom), HWratio, 0.1f, 100.0f);
			perspectiveShouldUpdate = false;
		}
		lastRatio = HWratio;
		return perspectiveCache;
	}
};
