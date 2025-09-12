#ifndef SHADERGL_H
#define SHADERGL_H

#include <string>

#include <glm/glm.hpp>
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;

#include <GL/glew.h>

#include "XString.h"

class ShaderGL
{
public:

    enum class ShaderType 
	{
		None,
        Vertex, 
		Fragment, 
		Geometry,
        TessControl, 
		TessEval,
		Compute,
		Count
    };

    ShaderGL();
	~ShaderGL();

	bool Build(const char* path, const char* rootName);
	bool BuildShaderFromString(const char* shaderName, const std::string& vertexSource, const std::string& fragmentSource, 
							   const std::string& geometrySource = "", const std::string& tessControlSource = "", const std::string& tessEvalSource = "");
	bool CompileShaderFromFile(const char* fileName, ShaderType type, bool link = false);
	bool CompileShaderFromString(const std::string& source, ShaderType type, bool link = false);
	bool Link();
	void Activate();

    std::string GetLog();

    int  GetHandle();
    bool IsLinked();
	const char* GetName();

	void BindAttribLocation(GLuint location, const char* name);
	void BindFragDataLocation(GLuint location, const char* name);

	void SetUniform(const char* name, float x, float y, float z);
	void SetUniform(const char* name, float x, float y);
	void SetUniform(const char* name, const vec3& v);
	void SetUniform(const char* name, const vec4& v);
	void SetUniform(const char* name, const mat4& m);
	void SetUniform(const char* name, const mat3& m);
	void SetUniform(const char* name, float val);
	void SetUniform(const char* name, int val);
	void SetUniform(const char* name, bool val);
	void AssertOnNotFound(bool assert);

    void PrintActiveUniforms();
    void PrintActiveAttribs();

	static ShaderType GetShaderTypeFromExt(const char* fileName);

private:

	int mh_program;
	int mh_shaderVertex;
	int mh_shaderFragment;
	int mh_shaderGeometry;
	int mh_shaderTessControl;
	int mh_shaderTessEval;

	XString ms_ShaderName;		// human-readable shader name
	bool m_IsLinked;
	std::string m_LogString;

	bool mb_AssertOnNotFound;		// flag to assert when uniform variable not found

private:

	bool Validate();

	void ClearGLErrors();
	int GetUniformLocation(const char* name);
	XString GetRootName(XString fileName);
	static bool FileExists(const char* fileName);
};

#endif // SHADERGL_H
