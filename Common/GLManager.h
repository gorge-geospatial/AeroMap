#ifndef GLMANAGER_H
#define GLMANAGER_H

#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include <vector>
#include <map>

#include "MarkTypes.h"
#include "Light.h"
#include "Material.h"
#include "ShaderGL.h"			// GLSL shader manager

enum class FILL_MODE
{
	Solid,
	Wire,
	Point
};

class GLManager
{
public:

	GLManager() {}
	~GLManager() {}

	static void Initialize();
	static void Shutdown();
	static bool IsInitialized();

	static void SetDefaultMatrices(glm::mat4& matModel, glm::mat4& matView, glm::mat4& matProj);
	static void PushMatrices(glm::mat4& matModel, glm::mat4& matView, glm::mat4& matProj);
	static void PopMatrices(glm::mat4& matModel, glm::mat4& matView, glm::mat4& matProj);

	static bool Project(VEC3 vPos, int& xp, int& yp);			// project using current matrices
	static bool UnProject(int xp, int yp, VEC3& vPos);			// un-project using current matrices

	static glm::mat4& GetMatModel();
	static glm::mat4& GetMatView();
	static glm::mat4& GetMatProj();

	static glm::mat4 MatViewFromCamera(VEC3 camPos, VEC3 camDir, VEC3 camUp);

	// shaders
	static void AddShader(std::string shaderName, ShaderGL* pShader);
	static ShaderGL* GetShader(std::string shaderName);

	// lighting
	static void AddLight(int index, Light* pLight);
	static void RemoveLight(int index);
	static void RemoveAllLights();
	static int  GetLightCount();
	static Light* GetLight(int index);

	// materials
	static void AddMaterial(int index, Material* pMaterial);
	static void RemoveMaterial(int index);
	static void RemoveAllMaterials();
	static int  GetMaterialCount();
	static Material* GetMaterial(int index);
	static Material* GetMaterial(const char* matName);

	// manage backface culling
	static void PushCull(bool newValue);
	static void PopCull();
	static bool GetCull();

	// manage winding order
	static void  PushWind(GLint newValue);
	static void  PopWind();
	static GLint GetWind();

	// manage depth testing
	static void PushDepth(bool enable, GLint testType = 0);
	static void PopDepth();

	static int  GetViewPort(int& x, int& y, int& width, int& height);
	static SizeType GLManager::GetViewPortSize();
	static bool CheckForOpenGLError(const char* file, int line);
	static int  FlipY(int y);

private:

	static bool m_Init;		// class has been initialized

	// stacks for saved matrices
	static std::vector<glm::mat4> m_matModelStack;
	static std::vector<glm::mat4> m_matViewStack;
	static std::vector<glm::mat4> m_matProjStack;

	// default matrices (if stack empty)
	static glm::mat4 m_matModel;
	static glm::mat4 m_matView;
	static glm::mat4 m_matProj;
	static bool m_DefaultMat;		// true => default matrices set

	static std::map<std::string, ShaderGL*> m_ShaderMap;		// shader name, ptr to externally managed shader
	static std::map<int, Light*> m_LightMap;					// light index, ptr to externally managed light
	static std::map<int, Material*> m_MaterialMap;				// material name, ptr to externally managed material

	// state stacks
	static std::vector<bool> mv_Cull;		// backface culling stack
	static std::vector<GLint> mv_Wind;		// winding order stack
	static std::vector<GLint> mv_Fill;		// fill mode stack

	struct DepthType
	{
		bool enable;	// true => depth testing enabled
		GLint type;		// type of test, if enabled
	};
	static std::vector<DepthType> mv_Depth;	// depth testing type

};

#endif // #ifndef GLMANAGER_H
