// Include{s}
#include "Camera.h"

#pragma region Constructor
Camera::Camera()
{
	// Multithreading this function is just too cool not to do.
	std::thread loadStartingVectorsThread(&Camera::LoadStartingVectors, this);

	// Set the base camera position and rotation
	m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_PositionVector = XMLoadFloat3(&m_position);
	m_RotationVector = XMLoadFloat3(&m_rotation);

	// Wait for the thread to finish
	loadStartingVectorsThread.join();

	// Apply the starting vectors to the camera
	UpdateViewMatrix();
}

void Camera::SetProjectionValues(float fov, float aspectRatio, float nearZ, float farZ)
{
	// Convert fov to radians
	float fovRadians = (fov / 360.0f) * XM_2PI;

	// Set the projection matrix
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
}
#pragma endregion

#pragma region Private Methods
void Camera::UpdateViewMatrix()
{
	// Calculate the rotation matrix
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y,
		m_rotation.z);

	// Transform the vectors with the rotation matrix
	XMVECTOR camTarget = XMVector3TransformCoord(m_startforwardVector, camRotationMatrix);

	// Apply the position to the camera target
	camTarget += m_PositionVector;

	// Calculate the up direction
	XMVECTOR upDirection = XMVector3TransformCoord(m_startupVector, camRotationMatrix);

	// Create the view matrix
	m_viewMatrix = XMMatrixLookAtLH(m_PositionVector, camTarget, upDirection);
	XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, m_rotation.y, 0.0f);

	// Transform the vectors with the rotation matrix
	m_forwardVector = XMVector3TransformCoord(m_startforwardVector, vecRotationMatrix);
	m_backVector = XMVector3TransformCoord(m_startbackVector, vecRotationMatrix);
	m_leftVector = XMVector3TransformCoord(m_startleftVector, vecRotationMatrix);
	m_rightVector = XMVector3TransformCoord(m_startrightVector, vecRotationMatrix);
}

void Camera::LoadStartingVectors()
{
	// Load the starting camera vectors from a JSON file
	std::ifstream file("JSON Files\\Starting Camera Vectors.json");

	// Check if the file is open
	if (!file.is_open())
	{
		std::cerr << "Failed to open JSON file." << std::endl;
	}

	file >> m_cameraStartingVectors;

	// Set the starting vectors using the JSON file values
	m_startforwardVector = XMVectorSet(
		m_cameraStartingVectors["StartingCameraVectors"]["ForwardVector"]["x"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["ForwardVector"]["y"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["ForwardVector"]["z"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["ForwardVector"]["w"].get<float>()
	);

	m_startupVector = XMVectorSet(
		m_cameraStartingVectors["StartingCameraVectors"]["UpVector"]["x"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["UpVector"]["y"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["UpVector"]["z"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["UpVector"]["w"].get<float>()
	);

	m_startrightVector = XMVectorSet(
		m_cameraStartingVectors["StartingCameraVectors"]["RightVector"]["x"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["RightVector"]["y"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["RightVector"]["z"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["RightVector"]["w"].get<float>()
	);

	m_startleftVector = XMVectorSet(
		m_cameraStartingVectors["StartingCameraVectors"]["LeftVector"]["x"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["LeftVector"]["y"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["LeftVector"]["z"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["LeftVector"]["w"].get<float>()
	);

	m_startbackVector = XMVectorSet(
		m_cameraStartingVectors["StartingCameraVectors"]["BackVector"]["x"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["BackVector"]["y"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["BackVector"]["z"].get<float>(),
		m_cameraStartingVectors["StartingCameraVectors"]["BackVector"]["w"].get<float>()
	);

	// Close the file
	file.close();
}
#pragma endregion

#pragma region Setters
void Camera::SetPosition(float x, float y, float z)
{
	// Set the camera position
	m_position = XMFLOAT3(x, y, z);
	m_PositionVector = XMLoadFloat3(&m_position);

	// Update the view matrix
	UpdateViewMatrix();
}

void Camera::AddToPosition(XMVECTOR position)
{
	// Add to the camera position using an XMVECTOR
	m_PositionVector += position;
	XMStoreFloat3(&m_position, m_PositionVector);

	// Update the view matrix
	UpdateViewMatrix();
}

void Camera::AddToPosition(float x, float y, float z)
{
	// Add to the camera position using x, y, z values
	m_position.x += x;
	m_position.y += y;
	m_position.z += z;

	// Update the position vector
	m_PositionVector = XMLoadFloat3(&m_position);

	// Update the view matrix
	UpdateViewMatrix();
}

void Camera::SetRotation(float x, float y, float z)
{
	// Set the camera rotation using x, y, z values
	m_rotation = XMFLOAT3(x, y, z);

	// Update the rotation vector
	m_RotationVector = XMLoadFloat3(&m_rotation);

	// Update the view matrix
	UpdateViewMatrix();
}

void Camera::AddToRotation(float x, float y, float z)
{
	// Add to the camera rotation using x, y, z values
	m_rotation.x += x;
	m_rotation.y += y;
	m_rotation.z += z;

	// Update the rotation vector
	m_RotationVector = XMLoadFloat3(&m_rotation);

	// Update the view matrix
	UpdateViewMatrix();
}
#pragma endregion

#pragma region Getters
// Getters for member variables

XMMATRIX Camera::GetViewMatrix() const
{
	return m_viewMatrix;
}

XMMATRIX Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}

XMFLOAT3 Camera::GetPosition() const
{
	return m_position;
}

XMFLOAT3 Camera::GetRotation() const
{
	return m_rotation;
}

XMVECTOR Camera::GetForwardVector() const
{
	return m_forwardVector;
}

XMVECTOR Camera::GetLeftVector() const
{
	return m_leftVector;
}

XMVECTOR Camera::GetRightVector() const
{
	return m_rightVector;
}

XMVECTOR Camera::GetBackVector() const
{
	return m_backVector;
}
#pragma endregion