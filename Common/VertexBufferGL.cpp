// VertexBufferGL.cpp
// Manager for renderable GLSL vertex buffers.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <string.h>
#include <assert.h>

#include "Logger.h"
#include "VertexBufferGL.h"

VertexBufferGL::VertexBufferGL()
	: m_PrimitiveType(0)
	, m_vaoHandle(0)
	, m_VertexCount(0)
	, m_IndexCount(0)
	, m_ibIndex(-1)
	, m_handleCount(0)
{
	memset(m_handle, 0, sizeof(m_handle));
}

VertexBufferGL::~VertexBufferGL()
{
	// release the vertex buffer 
	// & related resources
	Delete();
}

void VertexBufferGL::Render()
{
	assert(IsValid());

	// recall the VAO state info
	glBindVertexArray(m_vaoHandle);

	// render

	if (m_IndexCount > 0)
	{
		// indexed array
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle[m_ibIndex]);						// bind the index buffer
		glDrawElements(m_PrimitiveType, m_IndexCount, GL_UNSIGNED_INT, (GLubyte*)0);	// render
	}
	else
	{
		// non-indexed
		glDrawArrays(m_PrimitiveType, 0, m_VertexCount);
	}

	glBindVertexArray(0);

	// log gl errors
	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		Logger::Write(__FUNCTION__, "glError: %lu", glErr);
		glErr = glGetError();
	}
}

void VertexBufferGL::Delete()
{
	// Free all memory associated with this instance.
	//
	// Warning: Not sure if there's a bug in delete/recreate,
	//			but it's generally not a good idea to delete
	//			a vertex buffer with the intent of creating a
	//			new one it its place - better to create it once
	//			then modify it as needed.
	//

	if (m_vaoHandle != 0)
	{
		GLManager::CheckForOpenGLError(__FILE__, __LINE__);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDeleteBuffers(m_handleCount, m_handle);
		glDeleteVertexArrays(1, &m_vaoHandle);

		GLManager::CheckForOpenGLError(__FILE__, __LINE__);

		memset(m_handle, 0, sizeof(m_handle));
		m_vaoHandle = 0;
	}
	else
	{
		// should be impossible for handles to exist with
		// no vertex array object to be bound to
		int handleCount = sizeof(m_handle) / sizeof(m_handle[0]);
		for (int i = 0; i < handleCount; ++i)
			assert(m_handle[i] == 0);
	}

	m_VertexType = VertexType::NONE;
	m_PrimitiveType = 0;
	m_VertexCount = 0;
	m_IndexCount = 0;
}

bool VertexBufferGL::CreateWater(
	std::vector<VertexWater> vx,			// list of vertices
	std::vector<unsigned int> indexList)	// vertex indices, optional
{
	const int INDEX_P = 0;		// 2d position data
	const int INDEX_I = 1;		// indices

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	static_assert(sizeof(unsigned int) == sizeof(GLuint), "index parm wrong size");
	assert(vx.size() > 0);

	m_VertexCount = static_cast<unsigned int>(vx.size());
	m_IndexCount = static_cast<unsigned int>(indexList.size());

	// separate into separate buffers

	float* vertex = new float[m_VertexCount * 2];		// (x,y,z)

	for (unsigned int i = 0; i < m_VertexCount; ++i)
	{
		vertex[i * 2] = vx[i].x;
		vertex[i * 2 + 1] = vx[i].y;
	}

	// create and bind a Vertex Array Object - it will remember all of the
	// state information below so we can recall it with a single call to
	// glBindVertexArray( m_vaoHandle ) when rendering
	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	m_handleCount = 2;
	glGenBuffers(m_handleCount, m_handle);

	// 1 - load the vertex data

	// bind handle[0] to the GL_ARRAY_BUFFER point
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_P]);

	// load the vertex data
	glBufferData(GL_ARRAY_BUFFER,					// target buffer
		m_VertexCount * 2 * sizeof(float),			// size of buffer, bytes
		vertex,										// data
		GL_STATIC_DRAW);							// usage hint (STATIC = initialized, then only read; DRAW = used as src for rendering)			

	// specify the data format
	glVertexAttribPointer(INDEX_P, 			// index ("layout (location = 0) in vec3 VertexPosition;" in the shader)
		2, 									// # of components per attribute
		GL_FLOAT, 							// type of each component
		GL_FALSE, 							// normalize
		0, 									// stride (0 = tightly packed)
		(GLubyte*)0);						// offset of first component

	glEnableVertexAttribArray(INDEX_P);		// Vertex position ("layout (location = 0) in vec3 VertexPosition;" in the shader)

	// 2 - load the indices

	assert (m_IndexCount > 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle[INDEX_I]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndexCount * sizeof(GLuint), &indexList[0], GL_STATIC_DRAW);
	m_ibIndex = INDEX_I;

	// leave no index buffer bound
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// leave no array buffer bound
	glBindVertexArray(0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors

	m_PrimitiveType = GL_TRIANGLES;
	m_VertexType = VertexType::Water;

	delete [] vertex;

	return true;
}

bool VertexBufferGL::CreateP(
	unsigned int primitiveType,
	float* vx,
	int count)
{
	// Create a position only vertex array.
	//

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	assert(count > 0);

	// create and bind a Vertex Array Object - it will remember all of the
	// state information below so we can recall it with a single call to
	// glBindVertexArray( m_vaoHandle ) when rendering
	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	{
		// generate 1 handle for position data
		m_handleCount = 1;
		glGenBuffers(m_handleCount, m_handle);

		// 1 - load the position data

		// bind handle[0] to the GL_ARRAY_BUFFER point
		glBindBuffer(GL_ARRAY_BUFFER, m_handle[0]);

		// load the vertex data
		glBufferData(GL_ARRAY_BUFFER,			// target buffer
			count * 3 * sizeof(float),			// size of buffer, bytes
			vx,									// data
			GL_STATIC_DRAW);					// usage hint (STATIC = initialized, then only read; DRAW = used as src for rendering)			

		// specify the data format
		glVertexAttribPointer(0, 				// index ("layout (location = 0) in vec3 VertexPosition;" in the shader)
			3, 									// # of components per attribute
			GL_FLOAT, 							// type of each component
			GL_FALSE, 							// normalize
			0, 									// stride (0 = tightly packed)
			(GLubyte*)0);						// offset of first component

		glEnableVertexAttribArray(0);			// Vertex position ("layout (location = 0) in vec3 VertexPosition;" in the shader)
	}

	// leave no array buffer bound
	glBindVertexArray(0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors

	m_PrimitiveType = primitiveType;
	m_VertexType = VertexType::P;
	m_VertexCount = count;
	m_IndexCount = 0;

	return true;
}

bool VertexBufferGL::CreatePC(
	unsigned int primitiveType,				// GL_TRIANGLES, GL_LINES, etc.
	std::vector<VertexPC> vx,				// list of vertices
	std::vector<unsigned int> indexList)	// vertex indices, optional
{
	// Create a position/color (PC) vertex buffer.
	//

	const int INDEX_P = 0;
	const int INDEX_C = 1;
	const int INDEX_I = 2;

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	static_assert(sizeof(unsigned int) == sizeof(GLuint), "index parm wrong size");
	assert(vx.size() > 0);

	m_VertexCount = static_cast<unsigned int>(vx.size());
	m_IndexCount = static_cast<unsigned int>(indexList.size());

	// separate into separate buffers

	float* vertex = new float[m_VertexCount * 3];		// (x,y,z)
	float* color = new float[m_VertexCount * 3];		// (r,g,b)

	for (unsigned int i = 0; i < m_VertexCount; ++i)
	{
		vertex[i * 3] = vx[i].x;
		vertex[i * 3 + 1] = vx[i].y;
		vertex[i * 3 + 2] = vx[i].z;

		color[i * 3] = vx[i].r;
		color[i * 3 + 1] = vx[i].g;
		color[i * 3 + 2] = vx[i].b;
	}

	// create and bind a Vertex Array Object - it will remember all of the
	// state information below so we can recall it with a single call to
	// glBindVertexArray( m_vaoHandle ) when rendering
	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	m_handleCount = 3;
	glGenBuffers(m_handleCount, m_handle);

	// 1 - load the vertex data

	// bind handle[0] to the GL_ARRAY_BUFFER point
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_P]);

	// load the vertex data
	glBufferData(GL_ARRAY_BUFFER,					// target buffer
		m_VertexCount * 3 * sizeof(float),			// size of buffer, bytes
		vertex,										// data
		GL_STATIC_DRAW);							// usage hint (STATIC = initialized, then only read; DRAW = used as src for rendering)			

	// specify the data format
	glVertexAttribPointer(INDEX_P, 			// index ("layout (location = 0) in vec3 VertexPosition;" in the shader)
		3, 									// # of components per attribute
		GL_FLOAT, 							// type of each component
		GL_FALSE, 							// normalize
		0, 									// stride (0 = tightly packed)
		(GLubyte*)0);						// offset of first component

	glEnableVertexAttribArray(INDEX_P);		// Vertex position ("layout (location = 0) in vec3 VertexPosition;" in the shader)

	// 2 - load the color data

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_C]);

	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * 3 * sizeof(float),
		color,
		GL_STATIC_DRAW);

	glVertexAttribPointer(INDEX_C, 			// index
		3, 									// components per attribute
		GL_FLOAT,
		GL_FALSE,
		0,
		(GLubyte*)0);

	glEnableVertexAttribArray(1);			// Vertex color

	// 3 - load the indices

	if (m_IndexCount > 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle[INDEX_I]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndexCount * sizeof(GLuint), &indexList[0], GL_STATIC_DRAW);
		m_ibIndex = INDEX_I;

		// leave no index buffer bound
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// leave no array buffer bound
	glBindVertexArray(0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors

	m_PrimitiveType = primitiveType;
	m_VertexType = VertexType::PC;

	delete [] vertex;
	delete [] color;

	return true;
}

void VertexBufferGL::ModifyPC(float* vx, float* color)
{
	// Modify the vertex & color values of a PC buffer.
	//
	// Inputs:
	//		vx = array of xyz floats
	//		co = array of rgba floats
	//
	// Ideally, would be more general, but need to know 
	// that it (1) has vx & color and (2) are at positions
	// 0 & 1 respectively.

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	// modify vertex data
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[0]);
	glBufferData(GL_ARRAY_BUFFER, m_VertexCount * 3 * sizeof(float), vx, GL_STATIC_DRAW);

	// modify normals
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[1]);
	glBufferData(GL_ARRAY_BUFFER, m_VertexCount * 3 * sizeof(float), color, GL_STATIC_DRAW);

	// unbind so no indavertent "default" behavior
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors
}

bool VertexBufferGL::CreatePCT(
	unsigned int primitiveType,				// GL_TRIANGLES, GL_LINES, etc.
	std::vector<VertexPCT> vx,				// list of vertices
	std::vector<unsigned int> indexList)	// vertex indices, optional
{
	// Create a position/color/tex coordinate (PCT) vertex array.
	//

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	static_assert(sizeof(unsigned int) == sizeof(GLuint), "index parm wrong size");
	assert(vx.size() > 0);

	m_VertexCount = static_cast<unsigned int>(vx.size());
	m_IndexCount = static_cast<unsigned int>(indexList.size());

	// separate into separate buffers

	float* vertex = new float[m_VertexCount * 3];		// (x,y,z)
	float* color = new float[m_VertexCount * 3];		// (r,g,b)
	float* tex = new float[m_VertexCount * 2];			// (t,u)

	for (unsigned int i = 0; i < m_VertexCount; ++i)
	{
		vertex[i * 3] = vx[i].x;
		vertex[i * 3 + 1] = vx[i].y;
		vertex[i * 3 + 2] = vx[i].z;

		color[i * 3] = vx[i].r;
		color[i * 3 + 1] = vx[i].g;
		color[i * 3 + 2] = vx[i].b;

		tex[i * 2] = vx[i].u;
		tex[i * 2 + 1] = vx[i].v;
	}

	// create and bind a Vertex Array Object - it will remember all of the
	// state information below so we can recall it with a single call to
	// glBindVertexArray( m_vaoHandle ) when rendering
	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	{
		m_handleCount = 4;
		glGenBuffers(m_handleCount, m_handle);

		// 1 - position data

		// bind handle[0] to the GL_ARRAY_BUFFER point
		glBindBuffer(GL_ARRAY_BUFFER, m_handle[0]);

		// load the vertex data
		glBufferData(GL_ARRAY_BUFFER,					// target buffer
			m_VertexCount * 3 * sizeof(float),			// size of buffer, bytes
			vertex,										// data
			GL_STATIC_DRAW);							// usage hint (STATIC = initialized, then only read; DRAW = used as src for rendering)			

		// specify the data format
		glVertexAttribPointer(0, 				// index ("layout (location = 0) in vec3 VertexPosition;" in the shader)
			3, 									// # of components per attribute
			GL_FLOAT, 							// type of each component
			GL_FALSE, 							// normalize
			0, 									// stride (0 = tightly packed)
			(GLubyte*)0);						// offset of first component
		glEnableVertexAttribArray(0);			// P offset in shader ("layout (location = 0) in vec3 VertexPosition;")

		// 2 - load the color data

		glBindBuffer(GL_ARRAY_BUFFER, m_handle[1]);

		glBufferData(GL_ARRAY_BUFFER,
			m_VertexCount * 3 * sizeof(float),
			color,
			GL_STATIC_DRAW);

		glVertexAttribPointer(1, 				// index
			3, 									// components per attribute
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLubyte*)0);
		glEnableVertexAttribArray(1);			// C offset in shader ("layout (location = 1) in vec3 VertexColor;")

		// 3 - load the texture coordinates

		glBindBuffer(GL_ARRAY_BUFFER, m_handle[2]);
		glBufferData(GL_ARRAY_BUFFER,
			m_VertexCount * 2 * sizeof(float),
			tex,
			GL_STATIC_DRAW);
		glVertexAttribPointer(2,					// index 
			2, 										// components per attribute
			GL_FLOAT,
			GL_FALSE,
			0,
			(GLubyte*)0);
		glEnableVertexAttribArray(2);			// T offset in shader ("layout (location = 2) in vec2 TexCoord;")

		// 3 - load the indices, optional

		if (indexList.size() > 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle[3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexList.size() * sizeof(GLuint), &indexList[0], GL_STATIC_DRAW);
			m_ibIndex = 3;

			// leave no index buffer bound
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}

	delete [] vertex;
	delete [] color;
	delete [] tex;

	// leave no array buffer bound
	glBindVertexArray(0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors

	m_PrimitiveType = primitiveType;
	m_VertexType = VertexType::PCT;

	return true;
}

bool VertexBufferGL::CreatePNT(unsigned int primitiveType,				// GL_TRIANGLES, GL_LINES, etc.
								std::vector<VertexPNT> vxList,			// vertex data
								std::vector<unsigned int> indexList) 	// vertex indices
{
	// Create a PNT vertex buffer.
	//

	// vertex component offsets/slots (gl calls it 'index')
	const int INDEX_P = 0;
	const int INDEX_N = 1;
	const int INDEX_T = 2;
	const int INDEX_I = 3;			// optional index offset

	assert(m_vaoHandle == 0);		// make sure we didn't already create one

	m_VertexCount = static_cast<unsigned int>(vxList.size());
	assert(m_VertexCount > 0);

	m_IndexCount = static_cast<int>(indexList.size());
	m_ibIndex = -1;

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	// separate into separate buffers

	float* vertex = new float[m_VertexCount * 3];		// (x,y,z)
	float* normal = new float[m_VertexCount * 3];		// (x,y,z)
	float* tex0 = new float[m_VertexCount * 2];		// (t,u)

	for (unsigned int i = 0; i < m_VertexCount; ++i)
	{
		vertex[i * 3] = vxList[i].x;
		vertex[i * 3 + 1] = vxList[i].y;
		vertex[i * 3 + 2] = vxList[i].z;

		normal[i * 3] = vxList[i].nx;
		normal[i * 3 + 1] = vxList[i].ny;
		normal[i * 3 + 2] = vxList[i].nz;

		tex0[i * 2] = vxList[i].tu;
		tex0[i * 2 + 1] = vxList[i].tv;
	}

	// create and bind a Vertex Array Object - it will remember all of the
	// state information below so we can recall it with a single call to
	// glBindVertexArray( m_vaoHandle ) when rendering
	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	m_handleCount = 4;
	glGenBuffers(m_handleCount, m_handle);

	// 1 - load the vertex data

	// bind handle[0] to the GL_ARRAY_BUFFER point
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_P]);

	// load the vertex data
	glBufferData(GL_ARRAY_BUFFER,				// target buffer
		m_VertexCount * 3 * sizeof(float),		// size of buffer, bytes
		vertex,									// data
		GL_STATIC_DRAW);						// usage hint (STATIC = initialized, then only read; DRAW = used as src for rendering)			

	// specify the data format
	glVertexAttribPointer(INDEX_P, 				// index ("layout (location = 0) in vec3 VertexPosition;" in the shader)
		3, 										// # of components per attribute
		GL_FLOAT, 								// type of each component
		GL_FALSE, 								// normalize (false = do not map into -1.0 to 1.0 range)
		0, 										// stride (0 = tightly packed)
		(GLubyte*)0);							// offset of first component
	glEnableVertexAttribArray(INDEX_P);			// Vertex position ("layout (location = 0) in vec3 VertexPosition;" in the shader)

	// 2 - load the normal data

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_N]);
	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * 3 * sizeof(float),
		normal,
		GL_STATIC_DRAW);
	glVertexAttribPointer(INDEX_N, 				// index ("layout (location = 1) in vec3 VertexNormal;" in the shader)
		3, 										// components per attribute
		GL_FLOAT,
		GL_FALSE,
		0,
		(GLubyte*)0);
	glEnableVertexAttribArray(INDEX_N);			// Vertex normal

	// 3 - load the texture coordinates

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_T]);
	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * 2 * sizeof(float),
		tex0,
		GL_STATIC_DRAW);
	glVertexAttribPointer(INDEX_T,				// index 
		2, 										// components per attribute
		GL_FLOAT,
		GL_FALSE,
		0,
		(GLubyte*)0);
	glEnableVertexAttribArray(INDEX_T);			// texture coords

	// 3 - load the optional indices

	if (m_IndexCount > 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle[INDEX_I]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndexCount * sizeof(GLuint), &indexList[0], GL_STATIC_DRAW);
		m_ibIndex = INDEX_I;

		// leave no index buffer bound
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// leave no array buffer bound
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// leave no vertex array bound
	glBindVertexArray(0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors

	delete [] vertex;
	delete [] normal;
	delete [] tex0;

	m_PrimitiveType = primitiveType;
	m_VertexType = VertexType::PNT;

	return true;
}

bool VertexBufferGL::CreatePNTF(unsigned int primitiveType,				// GL_TRIANGLES, GL_LINES, etc.
	std::vector<VertexPNTF> vxList,			// vertex data
	std::vector<unsigned int> indexList) 	// vertex indices
{
	// Create PNTF buffer.
	//

	// vertex component offsets/slots (gl calls it 'index')
	const int INDEX_P = 0;
	const int INDEX_N = 1;
	const int INDEX_T = 2;
	const int INDEX_F = 3;
	const int INDEX_I = 4;			// optional index offset

	assert(m_vaoHandle == 0);		// make sure we didn't already create one

	m_VertexCount = static_cast<unsigned int>(vxList.size());
	assert(m_VertexCount > 0);

	m_IndexCount = static_cast<unsigned int>(indexList.size());

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	// separate into separate buffers

	float* vertex = new float[m_VertexCount * 3];		// (x,y,z)
	float* normal = new float[m_VertexCount * 3];		// (x,y,z)
	float* tex0 = new float[m_VertexCount * 2];			// (t,u)
	UInt32* flags = new UInt32[m_VertexCount];			// flags

	for (unsigned int i = 0; i < m_VertexCount; ++i)
	{
		vertex[i * 3] = vxList[i].x;
		vertex[i * 3 + 1] = vxList[i].y;
		vertex[i * 3 + 2] = vxList[i].z;

		normal[i * 3] = vxList[i].nx;
		normal[i * 3 + 1] = vxList[i].ny;
		normal[i * 3 + 2] = vxList[i].nz;

		tex0[i * 2] = vxList[i].tu;
		tex0[i * 2 + 1] = vxList[i].tv;

		flags[i] = vxList[i].flags;
	}

	// create and bind a Vertex Array Object - it will remember all of the
	// state information below so we can recall it with a single call to
	// glBindVertexArray( m_vaoHandle ) when rendering
	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	m_handleCount = 5;
	glGenBuffers(m_handleCount, m_handle);

	// P - Position

	// bind handle[0] to the GL_ARRAY_BUFFER point
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_P]);

	// load the vertex data
	glBufferData(GL_ARRAY_BUFFER,				// target buffer
		m_VertexCount * 3 * sizeof(float),		// size of buffer, bytes
		vertex,									// data
		GL_STATIC_DRAW);						// usage hint (STATIC = initialized, then only read; DRAW = used as src for rendering)			

	// specify the data format
	glVertexAttribPointer(INDEX_P, 				// index ("layout (location = 0) in vec3 VertexPos;" in the shader)
		3, 										// # of components per attribute
		GL_FLOAT, 								// type of each component
		GL_FALSE,								// normalize = false (do not map values into -1.0 - 1.0 range)
		0, 										// stride (0 = tightly packed)
		(GLubyte*)0);							// offset of first component

	glEnableVertexAttribArray(INDEX_P);			// Vertex position ("layout (location = 0) in vec3 VertexPosition;" in the shader)

	// N - Normal

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_N]);
	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * 3 * sizeof(float),
		normal,
		GL_STATIC_DRAW);

	glVertexAttribPointer(INDEX_N, 				// index ("layout (location = 1) in vec3 VertexNormal;" in the shader)
		3, 										// components per attribute
		GL_FLOAT,
		GL_FALSE,								// normalize = false (do not map values into -1.0 - 1.0 range)
		0,
		(GLubyte*)0);

	glEnableVertexAttribArray(INDEX_N);			// index ("layout (location = 1) in vec3 VertexNormal;" in the shader)

	// T - Texture Coordinates

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_T]);
	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * 2 * sizeof(float),
		tex0,
		GL_STATIC_DRAW);
	glVertexAttribPointer(INDEX_T,				// index 
		2, 										// components per attribute
		GL_FLOAT,
		GL_FALSE,
		0,
		(GLubyte*)0);
	glEnableVertexAttribArray(INDEX_T);			// texture coords

	// F - Vertex Flags

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_F]);
	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * sizeof(UInt32),
		flags,
		GL_STATIC_DRAW);

	// note: using attribI not attrib here to pass
	// integral type (attrib will convert to float)
	glVertexAttribIPointer(INDEX_F,				// index 
		1, 										// components per attribute
		GL_UNSIGNED_INT,						// 32-bit unsigned int
		0,
		(GLubyte*)0);
	glEnableVertexAttribArray(INDEX_F);			// flags

	// load the indices, optional

	if (m_IndexCount > 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle[INDEX_I]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndexCount * sizeof(GLuint), &indexList[0], GL_STATIC_DRAW);
		m_ibIndex = INDEX_I;

		// leave no index buffer bound
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// leave no array buffer bound
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete [] vertex;
	delete [] normal;
	delete [] tex0;
	delete [] flags;

	// leave no vertex array bound
	glBindVertexArray(0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors

	m_PrimitiveType = primitiveType;
	m_VertexType = VertexType::PNTF;

	return true;
}

bool VertexBufferGL::CreatePNC(
	unsigned int primitiveType,				// GL_TRIANGLES, GL_LINES, etc.
	std::vector<VertexPNC> vx,				// vertex data
	std::vector<unsigned int> indexList)	// vertex indices, optional
{
	// Create a PNC vertex array.
	//

	// vertex component offsets/slots (gl calls it 'index')
	const int INDEX_P = 0;
	const int INDEX_N = 1;
	const int INDEX_C = 2;
	const int INDEX_I = 3;			// optional index offset

	assert(m_vaoHandle == 0);		// make sure we didn't already create one

	m_VertexCount = static_cast<unsigned int>(vx.size());
	assert(m_VertexCount > 0);
	 
	m_IndexCount = static_cast<unsigned int>(indexList.size());
	m_ibIndex = -1;

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	// separate into separate buffers

	float* vertex = new float[m_VertexCount * 3];		// (x,y,z)
	float* normal = new float[m_VertexCount * 3];		// (x,y,z)
	float* color = new float[m_VertexCount * 3];		// (r,g,b)

	for (unsigned int i = 0; i < m_VertexCount; ++i)
	{
		vertex[i * 3] = vx[i].x;
		vertex[i * 3 + 1] = vx[i].y;
		vertex[i * 3 + 2] = vx[i].z;

		normal[i * 3] = vx[i].nx;
		normal[i * 3 + 1] = vx[i].ny;
		normal[i * 3 + 2] = vx[i].nz;

		color[i * 3] = vx[i].r;
		color[i * 3 + 1] = vx[i].g;
		color[i * 3 + 2] = vx[i].b;
	}

	// create and bind a Vertex Array Object - it will remember all of the
	// state information below so we can recall it with a single call to
	// glBindVertexArray( m_vaoHandle ) when rendering
	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	m_handleCount = 4;
	glGenBuffers(m_handleCount, m_handle);

	// 1 - load the vertex data

	// bind handle[0] to the GL_ARRAY_BUFFER point
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_P]);

	// load data
	glBufferData(GL_ARRAY_BUFFER,				// target buffer
		m_VertexCount * 3 * sizeof(float),		// size of buffer, bytes
		vertex,									// data
		GL_STATIC_DRAW);						// usage hint (STATIC = initialized, then only read; DRAW = used as src for rendering)			

	// specify data format
	glVertexAttribPointer(INDEX_P, 				// index ("layout (location = 0) in vec3 VertexPosition;" in the shader)
		3, 										// # of components per attribute
		GL_FLOAT, 								// type of each component
		GL_FALSE, 								// normalize (false = do not map into -1.0 to 1.0 range)
		0, 										// stride (0 = tightly packed)
		(GLubyte*)0);							// offset of first component
	glEnableVertexAttribArray(INDEX_P);			// Vertex position ("layout (location = 0) in vec3 VertexPosition;" in the shader)

	// 2 - load the normal data

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_N]);
	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * 3 * sizeof(float),
		normal,
		GL_STATIC_DRAW);
	glVertexAttribPointer(INDEX_N, 				// index ("layout (location = 1) in vec3 VertexNormal;" in the shader)
		3, 										// components per attribute
		GL_FLOAT,
		GL_FALSE,
		0,
		(GLubyte*)0);
	glEnableVertexAttribArray(INDEX_N);			// Vertex normal

	// 3 - load the color data

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_C]);
	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * 3 * sizeof(float),
		color,
		GL_STATIC_DRAW);
	glVertexAttribPointer(INDEX_C,				// index 
		3, 										// components per attribute
		GL_FLOAT,
		GL_FALSE,
		0,
		(GLubyte*)0);
	glEnableVertexAttribArray(INDEX_C);			// texture coords

	// 3 - load the indices

	if (m_IndexCount > 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_handle[INDEX_I]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_IndexCount * sizeof(GLuint), &indexList[0], GL_STATIC_DRAW);
		m_ibIndex = INDEX_I;

		// leave no index buffer bound
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// leave no array buffer bound
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete [] vertex;
	delete [] normal;
	delete [] color;

	// leave no vertex array bound
	glBindVertexArray(0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors

	m_PrimitiveType = primitiveType;
	m_VertexType = VertexType::PNC;

	return true;
}

void VertexBufferGL::ModifyPN(float* vx, float* n)
{
	// Modify the vertex & normal values of any PNx buffer.
	// Modifies entire buffer.
	//
	// If vertex buffer type contains P & N, they are always
	// the first 2 entries.
	//

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	// modify position data - P
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[0]);
	glBufferData(GL_ARRAY_BUFFER, m_VertexCount * 3 * sizeof(float), vx, GL_STATIC_DRAW);

	// modify normals - N
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[1]);
	glBufferData(GL_ARRAY_BUFFER, m_VertexCount * 3 * sizeof(float), n, GL_STATIC_DRAW);

	// unbind so no indavertent "default" behavior
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors
}

void VertexBufferGL::ModifyPN(float* vx, float* n, int offset)
{
	// Modify the vertex & normal values of any PNx buffer.
	// Modifies subset of buffer starting oat offset.
	//
	// If vertex buffer type contains P & N, they are always
	// the first 2 entries.
	//

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	// modify vertex data
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[0]);
	glBufferSubData(GL_ARRAY_BUFFER, offset, m_VertexCount * 3 * sizeof(float), vx);

	// modify normals
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[1]);
	glBufferSubData(GL_ARRAY_BUFFER, offset, m_VertexCount * 3 * sizeof(float), n);

	// unbind so no indavertent "default" behavior
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors
}

bool VertexBufferGL::CreateSSC(unsigned int primitiveType,		// GL_TRIANGLES, GL_LINES, etc.
							   std::vector<VertexSSC> vx)	 	// vertex data
{
	// Create a non-indexed SSC vertex buffer.
	//

	const int INDEX_P = 0;
	const int INDEX_C = 1;

	assert(m_vaoHandle == 0);		// make sure we didn't already create one

	m_VertexCount = static_cast<unsigned int>(vx.size());
	assert(m_VertexCount > 0);

	// separate into separate buffers

	float* vertex = new float[m_VertexCount * 2];		// (x,y)
	float* color = new float[m_VertexCount * 3];		// (r,g,b)

	for (unsigned int i = 0; i < m_VertexCount; ++i)
	{
		vertex[i * 2] = vx[i].x;
		vertex[i * 2 + 1] = vx[i].y;

		color[i * 3] = vx[i].r;
		color[i * 3 + 1] = vx[i].g;
		color[i * 3 + 2] = vx[i].b;
	}

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	// create and bind a Vertex Array Object - it will remember all of the
	// state information below so we can recall it with a single call to
	// glBindVertexArray( m_vaoHandle ) when rendering
	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	m_handleCount = 2;
	glGenBuffers(m_handleCount, m_handle);

	// 1 - load the vertex data

	// bind handle[0] to the GL_ARRAY_BUFFER point
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_P]);

	// load the vertex data
	glBufferData(GL_ARRAY_BUFFER,				// target buffer
		m_VertexCount * 2 * sizeof(float),		// size of buffer, bytes
		vertex,									// data
		GL_STATIC_DRAW);						// usage hint (STATIC = initialized, then only read; DRAW = used as src for rendering)			

	// specify the data format
	glVertexAttribPointer(INDEX_P, 				// index ("layout (location = 0) in vec3 VertexPosition;" in the shader)
		2, 										// # of components per attribute (2 = x,y)
		GL_FLOAT, 								// type of each component
		GL_FALSE, 								// normalize
		0, 										// stride (0 = tightly packed)
		(GLubyte*)0);							// offset of first component

	glEnableVertexAttribArray(INDEX_P);			// Vertex position ("layout (location = 0) in vec3 VertexPosition;" in the shader)

	// 2 - load the color data

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_C]);
	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * 3 * sizeof(float),
		color,
		GL_STATIC_DRAW);

	glVertexAttribPointer(INDEX_C, 				// index
		3, 										// components per attribute
		GL_FLOAT,
		GL_FALSE,
		0,
		(GLubyte*)0);
	glEnableVertexAttribArray(INDEX_C);			// Vertex color

	delete [] vertex;
	delete [] color;

	glBindVertexArray(0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors

	m_PrimitiveType = primitiveType;
	m_VertexType = VertexType::SSC;

	return true;
}

bool VertexBufferGL::CreateSSCT(unsigned int primitiveType,		// GL_TRIANGLES, GL_LINES, etc.
							    std::vector<VertexSSCT> vx)	 	// vertex data
{
	// Create a non-indexed SST vertex buffer.
	//

	const int INDEX_P = 0;
	const int INDEX_C = 1;
	const int INDEX_T = 2;

	assert(m_vaoHandle == 0);		// make sure we didn't already create one

	m_VertexCount = (unsigned int)vx.size();
	assert(m_VertexCount > 0);

	// separate into separate buffers

	float* vertex = new float[m_VertexCount * 2];		// (x,y)
	float* color = new float[m_VertexCount * 3];		// (r,g,b)
	float* tex0 = new float[m_VertexCount * 2];			// (t,u)

	for (unsigned int i = 0; i < m_VertexCount; ++i)
	{
		vertex[i * 2] = vx[i].x;
		vertex[i * 2 + 1] = vx[i].y;

		color[i * 3] = vx[i].r;
		color[i * 3 + 1] = vx[i].g;
		color[i * 3 + 2] = vx[i].b;

		tex0[i * 2] = vx[i].tu;
		tex0[i * 2 + 1] = vx[i].tv;
	}

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);

	// create and bind a Vertex Array Object - it will remember all of the
	// state information below so we can recall it with a single call to
	// glBindVertexArray( m_vaoHandle ) when rendering
	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	m_handleCount = 3;
	glGenBuffers(m_handleCount, m_handle);

	// 1 - load the vertex data

	// bind handle[0] to the GL_ARRAY_BUFFER point
	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_P]);

	// load the 2d position data
	glBufferData(GL_ARRAY_BUFFER,				// target buffer
		m_VertexCount * 2 * sizeof(float),		// size of buffer, bytes
		vertex,									// data
		GL_STATIC_DRAW);						// usage hint (STATIC = initialized, then only read; DRAW = used as src for rendering)			

	// specify the data format
	glVertexAttribPointer(INDEX_P, 				// index ("layout (location = 0) in vec3 VertexPosition;" in the shader)
		2, 										// # of components per attribute (2 = x,y)
		GL_FLOAT,					 			// type of each component
		GL_FALSE,								// normalize
		0,					 					// stride (0 = tightly packed)
		(GLubyte*)0);							// offset of first component
	glEnableVertexAttribArray(INDEX_P);			// Vertex position ("layout (location = 0) in vec3 VertexPosition;" in the shader)

	// 2 - load the color data

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_C]);
	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * 3 * sizeof(float),
		color,
		GL_STATIC_DRAW);
	glVertexAttribPointer(INDEX_C, 				// index
		3,										// components per attribute
		GL_FLOAT,
		GL_FALSE,
		0,
		(GLubyte*)0);
	glEnableVertexAttribArray(INDEX_C);			// Vertex color

	// 3 - load the texture coordinates

	glBindBuffer(GL_ARRAY_BUFFER, m_handle[INDEX_T]);
	glBufferData(GL_ARRAY_BUFFER,
		m_VertexCount * 2 * sizeof(float),
		tex0,
		GL_STATIC_DRAW);
	glVertexAttribPointer(INDEX_T,				// index 
		2, 										// components per attribute
		GL_FLOAT,
		GL_FALSE,
		0,
		(GLubyte*)0);
	glEnableVertexAttribArray(INDEX_T);			// texture coords

	delete [] vertex;
	delete [] color;
	delete [] tex0;

	glBindVertexArray(0);

	GLManager::CheckForOpenGLError(__FILE__, __LINE__);	// log gl errors

	m_PrimitiveType = primitiveType;
	m_VertexType = VertexType::SSCT;

	return true;
}
