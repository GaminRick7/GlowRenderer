#include "core/Camera.h"
#include <algorithm>

// Defines several possible options for camera movement
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// Constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) 
    : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
{
    this->position = position;
    this->worldUp = up;
    this->yaw = yaw;
    this->pitch = pitch;
    updateCameraVectors();
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::getViewMatrix() const
{
    // More efficient than glm::lookAt - directly construct the view matrix
    // View matrix = [right.x, up.x, -front.x, 0] [right.y, up.y, -front.y, 0] [right.z, up.z, -front.z, 0] [-dot(right,pos), -dot(up,pos), dot(front,pos), 1]
    return glm::mat4(
        right.x, up.x, -front.x, 0.0f,
        right.y, up.y, -front.y, 0.0f,
        right.z, up.z, -front.z, 0.0f,
        -glm::dot(right, position), -glm::dot(up, position), glm::dot(front, position), 1.0f
    );
}

// Returns the camera position
glm::vec3 Camera::getPosition() const
{
    return position;
}

// Processes input received from any keyboard-like input system
void Camera::processKeyboard(int direction, float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    if (direction == FORWARD)
        position += front * velocity;
    if (direction == BACKWARD)
        position -= front * velocity;
    if (direction == LEFT)
        position -= right * velocity;
    if (direction == RIGHT)
        position += right * velocity;
    if (direction == UP)
        position += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
    if (direction == DOWN)
        position -= glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
}

// Processes input received from a mouse input system
void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    // Update front, right and up vectors using the updated Euler angles
    updateCameraVectors();
}

// Processes input received from a mouse scroll-wheel event
void Camera::processMouseScroll(float yoffset)
{
    zoom -= yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

// Calculates the front vector from the Camera's (updated) Euler Angles
void Camera::updateCameraVectors()
{
    // Calculate the new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->front = glm::normalize(front);
    
    // Also re-calculate the right and up vector
    right = glm::normalize(glm::cross(this->front, worldUp));
    up = glm::normalize(glm::cross(right, this->front));
} 