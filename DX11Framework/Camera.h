#pragma once

// Include{s}
#include "Structures.h"

class Camera
{
public:
#pragma region Camera Creation
	// Camera Constructor
	Camera();
	// Set the projection values
	void SetProjectionValues(float fov, float aspectRatio, float nearZ, float farZ);
#pragma endregion

#pragma region Getters
	// Get the view matrix
	// Returns XMMATRIX - The camera view matrix
	XMMATRIX GetViewMatrix() const;

	// Get the projection matrix
	// Returns XMMATRIX - The camera projection matrix
	XMMATRIX GetProjectionMatrix() const;

	// Get the position
	// Returns XMFLOAT3 - The camera position as a float3
	XMFLOAT3 GetPosition() const;

	// Get the rotation
	// Returns XMFLOAT3 - The camera rotation as a float3
	XMFLOAT3 GetRotation() const;

	// Get the forward vector
	// Returns XMVECTOR - The forward camera vector
	XMVECTOR GetForwardVector() const;

	// Get the left vector
	// Returns XMVECTOR - The left camera vector
	XMVECTOR GetLeftVector() const;

	// Get the right vector
	// Returns XMVECTOR - The right camera vector
	XMVECTOR GetRightVector() const;

	// Get the back vector
	// Returns XMVECTOR - The back camera vector
	XMVECTOR GetBackVector() const;
#pragma endregion

#pragma region Setters
	// Set the camera position
	void SetPosition(float x, float y, float z);

	// Set the camera rotation
	void SetRotation(float x, float y, float z);

	// Add to the camera position using an XMVECTOR
	void AddToPosition(XMVECTOR position);

	// Add to the camera position using x, y, z values
	void AddToPosition(float x, float y, float z);

	// Add to the camera rotation
	void AddToRotation(float x, float y, float z);
#pragma endregion

private:
#pragma region Private Methods
	// Update the view camera matrix
	void UpdateViewMatrix();

	// Load the starting camera vectors from a JSON file
	void LoadStartingVectors();
#pragma endregion

#pragma region Member Variables
	XMVECTOR m_PositionVector;
	XMVECTOR m_RotationVector;

	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;

	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;

	nlohmann::json m_cameraStartingVectors;

	XMVECTOR m_startforwardVector;
	XMVECTOR m_startupVector;
	XMVECTOR m_startrightVector;
	XMVECTOR m_startleftVector;
	XMVECTOR m_startbackVector;

	XMVECTOR m_forwardVector;
	XMVECTOR m_leftVector;
	XMVECTOR m_rightVector;
	XMVECTOR m_backVector;
#pragma endregion
};
