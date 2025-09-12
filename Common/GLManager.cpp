// GLManager.cpp
// Global OpenGL rendering state manager
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//
// Single class that manages startup, shutdown,
// shaders, matrices, etc.
//
// for example, matrices kinda work w/in gl classes because i push/pop them, etc.
// but they have no knowledge of matrices in other classes or that might be active
// on entry, etc. this will provide global matrix state like the old fixed function
// gl pipeline (except w/o arbitrary stack size limit)
// 
// thinking of handling shaders in similar way; will also be able to manage 
// location of shaders, etc.
//
// Things I don't like about implementation:
//	- if you add GLManager to a project, need to add Light class
//  - management of light objects seems awkward
//

#include "Logger.h"
#include "GLManager.h"

bool GLManager::m_Init = false;

// definitions for class static declarations
std::vector<glm::mat4> GLManager::m_matModelStack;
std::vector<glm::mat4> GLManager::m_matViewStack;
std::vector<glm::mat4> GLManager::m_matProjStack;
glm::mat4 GLManager::m_matModel;
glm::mat4 GLManager::m_matView;
glm::mat4 GLManager::m_matProj;
bool GLManager::m_DefaultMat = false;

std::map<std::string, ShaderGL*> GLManager::m_ShaderMap;

std::map<int, Light*> GLManager::m_LightMap;
std::map<int, Material*> GLManager::m_MaterialMap;

std::vector<bool> GLManager::mv_Cull;
std::vector<GLint> GLManager::mv_Wind;
std::vector<GLint> GLManager::mv_Fill;
std::vector<GLManager::DepthType> GLManager::mv_Depth;


void GLManager::Initialize()
{
	Logger::Write(__FUNCTION__, "");

	GLManager::m_matModelStack.clear();
	GLManager::m_matViewStack.clear();
	GLManager::m_matProjStack.clear();

	GLManager::m_matModelStack.reserve(16);
	GLManager::m_matViewStack.reserve(16);
	GLManager::m_matProjStack.reserve(16);

	GLManager::mv_Cull.reserve(16);

	GLManager::m_Init = true;
	GLManager::m_DefaultMat = false;
}

void GLManager::Shutdown()
{
	Logger::Write(__FUNCTION__, "");

	assert(GLManager::m_matModelStack.size() == 0);
	assert(GLManager::m_matViewStack.size() == 0);
	assert(GLManager::m_matProjStack.size() == 0);

	assert(GLManager::mv_Cull.size() == 0);

	GLManager::m_ShaderMap.clear();
	GLManager::m_LightMap.clear();
	GLManager::m_MaterialMap.clear();

	GLManager::m_Init = false;
}

bool GLManager::IsInitialized()
{
	return GLManager::m_Init;
}

void GLManager::SetDefaultMatrices(glm::mat4& matModel, glm::mat4& matView, glm::mat4& matProj)
{
	// Since matrix stack generally empty outside of render context, this
	// allows for use of matrices outside that context.
	//

	assert(m_Init);

	GLManager::m_matModel = matModel;
	GLManager::m_matView = matView;
	GLManager::m_matProj = matProj;

	GLManager::m_DefaultMat = true;
}

void GLManager::PushMatrices(glm::mat4& matModel, glm::mat4& matView, glm::mat4& matProj)
{
	// Since you can't just say "Push()" like in fixed gl because there is no 
	// concept of a "current" matrix - each shader and/or code block may have
	// it's own matrices - we accept the matrices as parameters.

	assert(m_Init);

	GLManager::m_matModelStack.push_back(matModel);
	GLManager::m_matViewStack.push_back(matView);
	GLManager::m_matProjStack.push_back(matProj);
}

void GLManager::PopMatrices(glm::mat4& matModel, glm::mat4& matView, glm::mat4& matProj)
{
	assert(m_Init);
	assert(GLManager::m_matModelStack.size() > 0);

	matModel = GLManager::m_matModelStack.back();
	GLManager::m_matModelStack.pop_back();

	matView = GLManager::m_matViewStack.back();
	GLManager::m_matViewStack.pop_back();

	matProj = GLManager::m_matProjStack.back();
	GLManager::m_matProjStack.pop_back();
}

glm::mat4& GLManager::GetMatModel()
{
	// Return model matrix on top of stack without removing it.
	// If stack empty, returns default matrix.
	//

	assert(m_Init);
	if (GLManager::m_matModelStack.size() > 0)
		return GLManager::m_matModelStack.back();

	assert(m_DefaultMat);
	return m_matModel;
}

glm::mat4& GLManager::GetMatView()
{
	// return view matrix on top of stack without removing it

	assert(m_Init);
	if(GLManager::m_matViewStack.size() > 0)
		return GLManager::m_matViewStack.back();

	assert(m_DefaultMat);
	return m_matView;
}

glm::mat4& GLManager::GetMatProj()
{
	// return projection matrix on top of stack without removing it

	assert(m_Init);
	if (GLManager::m_matProjStack.size() > 0)
		return GLManager::m_matProjStack.back();

	assert(m_DefaultMat);
	return m_matProj;
}

glm::mat4 GLManager::MatViewFromCamera(VEC3 camPos, VEC3 camDir, VEC3 camUp)
{
	// Create view matrix from camera.
	//

	vec3 eye(camPos.x, camPos.y, camPos.z);
	vec3 up(camUp.x, camUp.y, camUp.z);
	vec3 lookAt(camPos.x + camDir.x, camPos.y + camDir.y, camPos.z + camDir.z);
	mat4 matView = glm::lookAt(eye, lookAt, up);
	return matView;
}

bool GLManager::Project(VEC3 vPos, int& xp, int& yp)
{
	// Using current matrices and viewport, project point
	// onto screen
	//
	// Inputs:
	//      vPos = 3d world coordinates
	//
	// Outputs:
	//      xp, yp = 2d pixel coordinates
	//

	int x, y, w, h;
	GetViewPort(x, y, w, h);

	glm::vec3 pos;
	pos.x = static_cast<float>(vPos.x);
	pos.y = static_cast<float>(vPos.y);
	pos.z = static_cast<float>(vPos.z);

	glm::vec4 vpnew;
	vpnew.x = static_cast<float>(x);			// lower left corner
	vpnew.y = static_cast<float>(y);
	vpnew.z = static_cast<float>(w);			// width
	vpnew.w = static_cast<float>(h);			// height

	glm::vec3 newpos = glm::project(pos,		// object coordinates 
		GetMatView()*GetMatModel(), 			// current modelview matrix
		GetMatProj(), 							// current projection matrix
		vpnew);									// current viewport

	xp = (int)(newpos.x + 0.5);
	yp = (int)(newpos.y + 0.5);

	// using standard ortho projection, y is inverted
	yp = h - yp;

	if (xp < vpnew.x || xp > vpnew.x + vpnew.z)
		return false;
	if (yp < vpnew.y || yp > vpnew.y + vpnew.w)
		return false;

	return true;
}

bool GLManager::UnProject(int xp, int yp, VEC3& vPos)
{
	// Using current matrices and viewport, project point
	// onto screen
	//
	// Inputs:
	//      vPos = 3d world coordinates
	//
	// Outputs:
	//      xp, yp = 2d pixel coordinates
	//

//TODO:
//not yet working
	assert(false);

	int x, y, w, h;
	GetViewPort(x, y, w, h);

	glm::vec3 pos;
	pos.x = static_cast<float>(xp);
	pos.y = static_cast<float>(yp);
	pos.z = static_cast<float>(0);

	glm::vec4 vp;
	vp.x = static_cast<float>(x);			// lower left corner
	vp.y = static_cast<float>(y);
	vp.z = static_cast<float>(w);			// width
	vp.w = static_cast<float>(h);			// height

	glm::vec3 newpos = glm::unProject(pos,		// object coordinates 
		GetMatView()*GetMatModel(), 			// current modelview matrix
		GetMatProj(), 							// current projection matrix
		vp);									// current viewport

	vPos.x = newpos.x;
	vPos.y = newpos.y;
	vPos.z = newpos.z;

	//// using standard ortho projection, y is inverted
	//yp = h - yp;

	//if (xp < vpnew.x || xp > vpnew.x + vpnew.z)
	//	return false;
	//if (yp < vpnew.y || yp > vpnew.y + vpnew.w)
	//	return false;

	return true;
}

void GLManager::AddShader(std::string name, ShaderGL* pShader)
{
	// Add ptr to shader for easy access from other classes.
	//
	// Inputs:
	//		name	= human readable shader name
	//		pShader = ptr to the shader
	// Outputs:
	//		return = unique id of shader
	//

	assert(pShader);
	assert(pShader->IsLinked());

	m_ShaderMap.insert(std::make_pair(name, pShader));
}

ShaderGL* GLManager::GetShader(std::string shaderName)
{
	// Return a pointer to the requested shader.
	//
	// Inputs:
	//		shaderName = name of shader
	// Outputs:
	//		return = address of shader, nullptr if not found
	//

	std::map<std::string, ShaderGL*>::iterator it = m_ShaderMap.find(shaderName);
	if (it != m_ShaderMap.end())
		return it->second;

	return nullptr;
}

void GLManager::AddLight(int index, Light* pLight)
{
	// Add light to list of managed lights.
	//
	// Lights are managed externally, they are not
	// created or destroyed here.
	//
	// Inputs:
	//		index = 0-based light index
	//		pLight = address of light object

	assert(m_Init);

	// if light already exists, replace it
	std::map<int, Light*>::iterator it = m_LightMap.find(index);
	if (it != m_LightMap.end())
		it->second = pLight;
	else
		GLManager::m_LightMap.insert(std::make_pair(index, pLight));

//TODO:
	// is this legal?
	//m_LightMap[index] = pLight;
}

void GLManager::RemoveLight(int index)
{
	// Nothing is deleted, light objects
	// are externally managed.

	assert(m_Init);

	std::map<int, Light*>::iterator it = m_LightMap.find(index);
	if (it != m_LightMap.end())
		GLManager::m_LightMap.erase(it);
}

void GLManager::RemoveAllLights()
{
	// Nothing is deleted, light objects
	// are externally managed.

	GLManager::m_LightMap.clear();
}

Light* GLManager::GetLight(int index)
{
	assert(m_Init);

	std::map<int, Light*>::iterator it = GLManager::m_LightMap.find(index);
	if (it != GLManager::m_LightMap.end())
		return it->second;

	return nullptr;
}

int GLManager::GetLightCount()
{
	return static_cast<int>(GLManager::m_LightMap.size());
}

void GLManager::AddMaterial(int index, Material* pMaterial)
{
	// Add material to list of managed materials.
	//
	// Materials are managed externally, they are not
	// created or destroyed here.
	//
	// Inputs:
	//		index = zero based material index
	//		pMaterial = address of material object

	assert(m_Init);

	// if material already exists, replace it
	std::map<int, Material*>::iterator it = m_MaterialMap.find(index);
	if (it != m_MaterialMap.end())
		it->second = pMaterial;
	else
		GLManager::m_MaterialMap.insert(std::make_pair(index, pMaterial));
}

void GLManager::RemoveMaterial(int index)
{
	// Nothing is deleted, light objects
	// are externally managed.

	assert(m_Init);

	std::map<int, Material*>::iterator it = m_MaterialMap.find(index);
	if (it != m_MaterialMap.end())
		GLManager::m_MaterialMap.erase(it);
}

void GLManager::RemoveAllMaterials()
{
	// Nothing is deleted, light objects
	// are externally managed.

	GLManager::m_MaterialMap.clear();
}

Material* GLManager::GetMaterial(int index)
{
	// Get material by index, uses key.

	assert(m_Init);

	std::map<int, Material*>::iterator it = m_MaterialMap.find(index);
	if (it != GLManager::m_MaterialMap.end())
		return it->second;

	return nullptr;
}

Material* GLManager::GetMaterial(const char* matName)
{
	// Get material by name, searches sequentially.

	assert(m_Init);

	XString strName = matName;
	for (auto it : GLManager::m_MaterialMap)
	{
		if (strName.CompareNoCase(it.second->GetName().c_str()))
			return it.second;
	}

	return nullptr;
}

int GLManager::GetMaterialCount()
{
	return static_cast<int>(GLManager::m_MaterialMap.size());
}

int GLManager::GetViewPort(int& x, int& y, int& width, int& height)
{
	// Get the current opengl viewport.
	//
	// Outputs:
	//		x, y = lower left of viewport
	//		width = width of viewport
	//		height = height of viewport
	//		return = 0 on success
	//

	CheckForOpenGLError(__FILE__, __LINE__);

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	GLenum glErr = glGetError();
	if (glErr != GL_NO_ERROR)
		return glErr;

	x = vp[0];			// lower left corner
	y = vp[1];
	width = vp[2];		// width
	height = vp[3];		// height

	return 0;
}

SizeType GLManager::GetViewPortSize()
{
	// Get the size only of the OpenGL viewport.
	//
	// Outputs:
	//		return = size of viewport
	//

	SizeType sz = { -1,-1 };

	CheckForOpenGLError(__FILE__, __LINE__);

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	GLenum glErr = glGetError();
	if (glErr == GL_NO_ERROR)
	{
		sz.cx = vp[2];		// width
		sz.cy = vp[3];		// height
	}

	return sz;
}

int GLManager::FlipY(int y)
{
	// convenience function to invert y axis values
	//

	int vpx, vpy, vpw, vph;
	if (GetViewPort(vpx, vpy, vpw, vph) == 0)
	{
		y = vph - y;
	}

	return y;
}

bool GLManager::CheckForOpenGLError(const char* file, int line)
{
	// returns true if an OpenGL error occurred and
	// clears all pending errors

	bool retCode = false;
	
	// loop to get them all
	int errCtr = 0;
	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		Logger::Write(__FUNCTION__, "glError in file %s @ line %d: %s", file, line, gluErrorString(glErr));
		retCode = true;
		glErr = glGetError();

		// this has to be limited - i've seen cases where the "get" doesn't clear
		// them (or something is happening in another thread to add them while 
		// this loop is removing them)
		if (++errCtr > 20)
		{
			Logger::Write(__FUNCTION__, "Aborted loop, max # of errors exceeded.");
			break;
		}
	}

	return retCode;
}

void GLManager::PushCull(bool newValue)
{
	// set backface culling and push current value

	// true => backface culling is enabled

	// get current mode 
	bool isEnabled = (glIsEnabled(GL_CULL_FACE) == GL_TRUE);

	// "backface" is the only culling type supported
	glCullFace(GL_BACK);

	if (newValue)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	// push the current value, not the passed new value
	mv_Cull.push_back(isEnabled);
}

void GLManager::PopCull()
{
	// pop and set backface culling state

	if (mv_Cull.size() == 0)
		return;

	// "backface" is the only culling type supported
	glCullFace(GL_BACK);

	if (mv_Cull.back() == true)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	mv_Cull.pop_back();
}

bool GLManager::GetCull()
{
	// get current backface culling state

	// may or may not be on the stack, so just
	// retrieve from current gl state

	GLboolean cullMode = false;
	glGetBooleanv(GL_CULL_FACE, &cullMode);

	return (cullMode == GL_TRUE);
}

void GLManager::PushWind(GLint newValue)
{
	// set winding order and push current value

	assert(newValue == GL_CW || newValue == GL_CCW);

	GLint windOrder = 0;
	glGetIntegerv(GL_FRONT_FACE, &windOrder);
	glFrontFace(newValue);

	// push the current value, not the passed new value
	mv_Wind.push_back(windOrder);
}

void GLManager::PopWind()
{
	// pop and set winding order

	if (mv_Wind.size() == 0)
		return;

	glFrontFace(mv_Wind.back());
	mv_Wind.pop_back();
}

GLint GLManager::GetWind()
{
	// get current winding order

	// may or may not be on the stack, so just
	// retrieve from current gl state

	GLint windOrder = 0;
	glGetIntegerv(GL_FRONT_FACE, &windOrder);

	return windOrder;
}

void GLManager::PushDepth(bool enable, GLint testType)
{
	// set & push depth testing
	//
	// inputs:
	//		enable = enable/disable depth testing
	//		testType = type of depth testing, if enable == true
	//			GL_NEVER	- Never passes.
	//			GL_LESS		- Passes if the incoming depth value is less than the stored depth value.
	//			GL_EQUAL	- Passes if the incoming depth value is equal to the stored depth value.
	//			GL_LEQUAL	- Passes if the incoming depth value is less than or equal to	the stored depth value.
	//			GL_GREATER	- Passes if the incoming depth value is greater than the stored depth value.
	//			GL_NOTEQUAL - Passes if the incoming depth value is not equal to the stored depth value.
	//			GL_GEQUAL	- Passes if the incoming depth value is greater than or equal to the stored depth value.
	//			GL_ALWAYS	- Always passes.
	//

	DepthType dt;
	dt.enable = enable;

	if (enable)
	{
		assert(testType == GL_NEVER || testType == GL_LESS || testType == GL_EQUAL || testType == GL_LEQUAL 
			|| testType == GL_GREATER || testType == GL_NOTEQUAL || testType == GL_GEQUAL || testType == GL_ALWAYS);

		glEnable(GL_DEPTH_TEST);		// Enables Depth Testing
		glDepthFunc(testType);			// The Type Of Depth Testing To Do

		dt.type = testType;
	}
	else
	{
		glDisable(GL_DEPTH_TEST);		// Disables Depth Testing

		dt.type = GL_ALWAYS;
	}

	mv_Depth.push_back(dt);
}

void GLManager::PopDepth()
{
	// restore previous depth testing settings
	//

	if (mv_Depth.size() == 0)
		return;

	DepthType dt = mv_Depth.back();

	if (dt.enable)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(dt.type);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	mv_Depth.pop_back();
}
