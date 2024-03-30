/*------------------------------
Author: Christian Henshaw
Organization: SNHU
Version: 1.0
------------------------------*/

#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>

class Meshes
{
	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nVertices;   // Number of vertices for the mesh
		GLuint nIndices;    // Number of indices for the mesh
	};

public:
	GLMesh gCylinderMesh;
	GLMesh gPlaneMesh;
	GLMesh gConeMesh;
	GLMesh gSphereMesh;
	GLMesh gCubeMesh;
	GLMesh gHexagonMesh;

public:
	void CreateMeshes();
	void DestroyMeshes();

private:
	void UCreateCylinderMesh(GLMesh& mesh);
	void UCreateConeMesh(GLMesh& mesh);
	void UCreatePlaneMesh(GLMesh& mesh);
	void UCreateSphereMesh(GLMesh& mesh);
	void UCreateCubeMesh(GLMesh& mesh);
	void UCreateHexagonMesh(GLMesh& mesh);

	void UDestroyMesh(GLMesh& mesh);
};