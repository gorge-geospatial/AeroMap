// ShaderGL.cpp
// OpenGL shader manager
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//
// A shader program consists of one or more individual shaders for each
// stage of the pipeline:
//
//		--------------------------------------------------------------------
//		component					member variable			functions
//		--------------------------------------------------------------------
//		program						mh_program				glLink()
//		vertex shader				m_handleVert			glCompile()
//		fragment shader				m_handleFrag			glCompile()
//		geometry shader				m_hangleGeom			glCompile()
//
// The "program" is created with glCreateProgram() and the shaders are created
// with glCreateShader(); they are then combined by "attaching" the shaders to
// the program 
//
// Compiling and linking is performed separately from the above create/attach
// steps; it appears shaders can be compiled before or after attaching - linking
// of course can only happen after compilation.
//

#include "MarkLib.h"
#include "ShaderGL.h"
#include "Logger.h"

#include <vector>

#include <fstream>
using std::ifstream;
using std::ios;

#include <sstream>
using std::ostringstream;

#include <sys/stat.h>

ShaderGL::ShaderGL()
	: mh_program(0)
	, mh_shaderVertex(0)
	, mh_shaderFragment(0)
	, mh_shaderGeometry(0)
	, mh_shaderTessControl(0)
	, mh_shaderTessEval(0)
	, m_IsLinked(false)
	, m_LogString("")
	, ms_ShaderName("")
	, mb_AssertOnNotFound(true)
{
}

ShaderGL::~ShaderGL()
{
	// detach all shaders from program

	if (mh_shaderVertex != 0)
	{
		glDetachShader(mh_program, mh_shaderVertex);
		glDeleteShader(mh_shaderVertex);
	}
	if (mh_shaderFragment != 0)
	{
		glDetachShader(mh_program, mh_shaderFragment);
		glDeleteShader(mh_shaderFragment);
	}
	if (mh_shaderGeometry != 0)
	{
		glDetachShader(mh_program, mh_shaderGeometry);
		glDeleteShader(mh_shaderGeometry);
	}
	if (mh_shaderTessControl != 0)
	{
		glDetachShader(mh_program, mh_shaderTessControl);
		glDeleteShader(mh_shaderTessControl);
	}
	if (mh_shaderTessEval != 0)
	{
		glDetachShader(mh_program, mh_shaderTessEval);
		glDeleteShader(mh_shaderTessEval);
	}

	// delete main program
	if (mh_program)
		glDeleteProgram(mh_program);
}

bool ShaderGL::Build(const char* path, const char* rootName)
{
	// Compile and link all shaders with specified root file name
	//

	ms_ShaderName = rootName;

	std::vector<XString> fileList = MarkLib::GetFileList(path);
	for (XString file : fileList)
	{
		XString fileName = file.GetFileName();
		fileName = GetRootName(fileName);

		if (fileName.CompareNoCase(rootName))
		{
			if (file.EndsWithNoCase(".vert") || file.EndsWithNoCase(".vs"))
			{
				if (!CompileShaderFromFile(file.c_str(), ShaderType::Vertex))
				{
					Logger::Write(__FUNCTION__, "Vertex shader failed to compile: %s", GetLog().c_str());
					return false;
				}
			}
			else if (file.EndsWithNoCase(".frag") || file.EndsWithNoCase(".fs"))
			{
				if (!CompileShaderFromFile(file.c_str(), ShaderType::Fragment))
				{
					Logger::Write(__FUNCTION__, "Fragment shader failed to compile: %s", GetLog().c_str());
					return false;
				}
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unsuported shader type: %s", file.c_str());
				assert(false);
			}
		}
	}

	if (!Link())
    {
		Logger::Write(__FUNCTION__, "Shader program failed to link: '%s'", XString::CombinePath(path, rootName).c_str());
		Logger::Write(__FUNCTION__, "    Error message: %s", GetLog().c_str());
		return false;
    }

	Activate();
	PrintActiveUniforms();
	PrintActiveAttribs();

	return true;
}

bool ShaderGL::BuildShaderFromString(const char* shaderName,						// shader name
									 const std::string& vertexSource,				// vertex shader source
									 const std::string& fragmentSource,				// fragment shader source
									 const std::string& geometrySource,				// [opt] geometry shader source
									 const std::string& tessControlSource,			// [opt] tesselation control source
									 const std::string& tessEvalSource)				// [opt] tesselation evaluation source
{
	// Compile & link all shaders in pipeline.
	//

	ms_ShaderName = shaderName;

	if (vertexSource.length() > 0)
	{
		if (!CompileShaderFromString(vertexSource.c_str(), ShaderType::Vertex))
		{
			Logger::Write(__FUNCTION__, "Vertex shader failed to compile: %s", GetLog().c_str());
			return false;
		}
	}

	if (fragmentSource.length() > 0)
	{
		if (!CompileShaderFromString(fragmentSource.c_str(), ShaderType::Fragment))
		{
			Logger::Write(__FUNCTION__, "Fragment shader failed to compile: %s", GetLog().c_str());
			return false;
		}
	}

	if (geometrySource.length() > 0)
	{
		assert(false);
		return false;
	}

	if (tessControlSource.length() > 0)
	{
		assert(false);
		return false;
	}

	if (tessEvalSource.length() > 0)
	{
		assert(false);
		return false;
	}


	if (!Link())
    {
		Logger::Write(__FUNCTION__, "Shader failed to link: '%s'", shaderName);
		Logger::Write(__FUNCTION__, "    Error message: %s", GetLog().c_str());
		return false;
    }

	Activate();
	PrintActiveUniforms();
	PrintActiveAttribs();

	return true;
}

bool ShaderGL::CompileShaderFromFile(const char* fileName, ShaderType type, bool link)
{
	if (!FileExists(fileName))
	{
	    m_LogString = "File not found: ";
		m_LogString += fileName;
	    return false;
	}

	if (mh_program <= 0)
	{
		mh_program = glCreateProgram();
		if (mh_program == 0)
		{
			m_LogString = "Unable to create shader program.";
			return false;
		}
	}

	ifstream inFile(fileName, ios::in);
	if (!inFile)
	{
		return false;
	}

	ostringstream code;
	while (inFile.good())
	{
		int c = inFile.get();
		if (!inFile.eof())
			code << (char)c;
	}
	inFile.close();

	return CompileShaderFromString(code.str(), type, link);
}

bool ShaderGL::CompileShaderFromString(const std::string& source, ShaderType type, bool link)
{
	if (mh_program <= 0)
	{
		mh_program = glCreateProgram();
		if (mh_program == 0)
		{
			m_LogString = "Unable to create shader program.";
			return false;
		}
	}

	int shaderHandle = 0;
	switch (type)
	{
	case ShaderType::Vertex:
		{
			if (mh_shaderVertex != 0)
			{
				// remove the currently-attached vertex shader
				// (you could have more than one shader of the same type 
				//  attached, i'm just not supporting that at this time)
				glDetachShader(mh_program, mh_shaderVertex);
				glDeleteShader(mh_shaderVertex);
			}
			mh_shaderVertex = glCreateShader(GL_VERTEX_SHADER);
			glAttachShader(mh_program, mh_shaderVertex);
			shaderHandle = mh_shaderVertex;
		}
		break;
	case ShaderType::Fragment:
		{
			if (mh_shaderFragment != 0)
			{
				glDetachShader(mh_program, mh_shaderFragment);
				glDeleteShader(mh_shaderFragment);
			}
			mh_shaderFragment = glCreateShader(GL_FRAGMENT_SHADER);
			glAttachShader(mh_program, mh_shaderFragment);
			shaderHandle = mh_shaderFragment;
		}
		break;
	case ShaderType::Geometry:
		//shaderHandle = glCreateShader(GL_GEOMETRY_SHADER);
		assert(false);	// not implemented
		break;
	case ShaderType::TessControl:
		//shaderHandle = glCreateShader(GL_TESS_CONTROL_SHADER);
		assert(false);	// not implemented
		break;
	case ShaderType::TessEval:
		//shaderHandle = glCreateShader(GL_TESS_EVALUATION_SHADER);
		assert(false);	// not implemented
		break;
	default:
		return false;
	}

	const char* c_code = source.c_str();
	glShaderSource(shaderHandle, 1, &c_code, NULL);

	// Compile the shader
	glCompileShader(shaderHandle);

	// Check for errors
	int result;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
	if (GL_FALSE == result)
	{
		// Compile failed, store log and return false
		int length = 0;
		m_LogString = "";
		glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
		if (length > 0)
		{
			char* c_log = new char[length];
			int written = 0;
			glGetShaderInfoLog(shaderHandle, length, &written, c_log);
			m_LogString = c_log;
			delete[] c_log;
		}

		return false;
	}
	else
	{
		// any time a component shader is replaced, program
		// needs re-linking

		if (link)
		{
			Link();
		}
		else
		{
			m_IsLinked = false;
		}

		// compile succeeded
		return true;
	}
}

bool ShaderGL::Link()
{
	if (mh_program <= 0)
		return false;

	m_IsLinked = false;

	glLinkProgram(mh_program);

	int status = 0;
	glGetProgramiv(mh_program, GL_LINK_STATUS, &status);
	if (GL_FALSE == status)
	{
		// Store log and return false
		int length = 0;
		m_LogString = "";

		glGetProgramiv(mh_program, GL_INFO_LOG_LENGTH, &length);

		if (length > 0)
		{
			char * c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(mh_program, length, &written, c_log);
			m_LogString = c_log;
			delete[] c_log;
		}
	}
	else
	{
		m_IsLinked = true;
	}

	return m_IsLinked;
}

void ShaderGL::Activate()
{
	if (mh_program <= 0 || (!m_IsLinked))
	{
		Logger::Write(__FUNCTION__, "Attempt to use invalid shader program: %s", ms_ShaderName.c_str());
		return;
	}
	glUseProgram(mh_program);
}

XString ShaderGL::GetRootName(XString fileName)
{
	// Return root portion of file name.
	//

	// ensure no path
	fileName.NormalizePath();
	int pos = fileName.ReverseFind('/');
	if (pos > -1)
		fileName.DeleteLeft(pos + 1);

	// remove ext
	pos = fileName.ReverseFind('.');
	if (pos > -1)
		fileName = fileName.Left(pos);

	fileName.Trim();

	return fileName;
}

std::string ShaderGL::GetLog()
{
    return m_LogString;
}

int ShaderGL::GetHandle()
{
    return mh_program;
}

bool ShaderGL::IsLinked()
{
    return m_IsLinked;
}

const char* ShaderGL::GetName()
{
	return ms_ShaderName.c_str();
}

void ShaderGL::BindAttribLocation(GLuint location, const char* name)
{
	glBindAttribLocation(mh_program, location, name);
}

void ShaderGL::BindFragDataLocation(GLuint location, const char* name)
{
	glBindFragDataLocation(mh_program, location, name);
}

void ShaderGL::SetUniform(const char* name, float x, float y, float z)
{
	// ensure this shader active before attempting to set uniform
	Activate();

	int loc = GetUniformLocation(name);
	if (loc >= 0)
	{
		ClearGLErrors();

		glUniform3f(loc, x, y, z);

		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR)
		{
			Logger::Write(__FUNCTION__, "Unable to set uniform '%s': %s", name, gluErrorString(glErr));
			assert(false);
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "Uniform: %s not found.", name);
		if (mb_AssertOnNotFound)
		{
			assert(false);
		}
	}
}

void ShaderGL::SetUniform(const char* name, float x, float y)
{
	// ensure this shader active before attempting to set uniform
	Activate();

	int loc = GetUniformLocation(name);
	if (loc >= 0)
	{
		ClearGLErrors();

		glUniform2f(loc, x, y);

		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR)
		{
			Logger::Write(__FUNCTION__, "Unable to set uniform '%s': %s", name, gluErrorString(glErr));
			assert(false);
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "Uniform: %s not found.", name);
		if (mb_AssertOnNotFound)
		{
			assert(false);
		}
	}
}

void ShaderGL::SetUniform(const char *name, const vec3& v)
{
	// ensure this shader active before attempting to set uniform
	Activate();

	SetUniform(name, v.x, v.y, v.z);
}

void ShaderGL::SetUniform(const char* name, const vec4& v)
{
	// ensure this shader active before attempting to set uniform
	Activate();

	int loc = GetUniformLocation(name);
	if (loc >= 0)
	{
		ClearGLErrors();

		glUniform4f(loc, v.x, v.y, v.z, v.w);

		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR)
		{
			Logger::Write(__FUNCTION__, "Unable to set uniform '%s': %s", name, gluErrorString(glErr));
			assert(false);
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "%s: uniform '%s' not found.", ms_ShaderName.c_str(), name);
		if (mb_AssertOnNotFound)
		{
			assert(false);
		}
	}
}

void ShaderGL::SetUniform(const char* name, const mat4& m)
{
	// ensure this shader active before attempting to set uniform
	Activate();

	int loc = GetUniformLocation(name);
	if (loc >= 0)
	{
		ClearGLErrors();

		glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);

		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR)
		{
			Logger::Write(__FUNCTION__, "Unable to set uniform '%s': %s", name, gluErrorString(glErr));
			assert(false);
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "%s: uniform '%s' not found.", ms_ShaderName.c_str(), name);
		if (mb_AssertOnNotFound)
		{
			assert(false);
		}
	}
}

void ShaderGL::SetUniform(const char* name, const mat3& m)
{
	// ensure this shader active before attempting to set uniform
	Activate();

	int loc = GetUniformLocation(name);
	if (loc >= 0)
	{
		ClearGLErrors();

		glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);

		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR)
		{
			Logger::Write(__FUNCTION__, "Unable to set uniform '%s': %s", name, gluErrorString(glErr));
			assert(false);
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "%s: uniform '%s' not found.", ms_ShaderName.c_str(), name);
		if (mb_AssertOnNotFound)
		{
			assert(false);
		}
	}
}

void ShaderGL::SetUniform(const char *name, float val)
{
	// ensure this shader active before attempting to set uniform
	Activate();

	int loc = GetUniformLocation(name);
	if (loc >= 0)
	{
		ClearGLErrors();
		
		glUniform1f(loc, val);

		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR)
		{
			Logger::Write(__FUNCTION__, "Unable to set uniform '%s': %s", name, gluErrorString(glErr));
			assert(false);
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "%s: uniform '%s' not found.", ms_ShaderName.c_str(), name);
		if (mb_AssertOnNotFound)
		{
			assert(false);
		}
	}
}

void ShaderGL::SetUniform(const char* name, int val)
{
	// ensure this shader active before attempting to set uniform
	Activate();

	int loc = GetUniformLocation(name);
	if (loc >= 0)
	{
		ClearGLErrors();

		glUniform1i(loc, val);

		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR)
		{
			Logger::Write(__FUNCTION__, "Unable to set uniform '%s': %s", name, gluErrorString(glErr));
			assert(false);
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "%s: uniform '%s' not found.", ms_ShaderName.c_str(), name);
		if (mb_AssertOnNotFound)
		{
			assert(false);
		}
	}
}

void ShaderGL::SetUniform(const char *name, bool val)
{
	// ensure this shader active before attempting to set uniform
	Activate();

	int loc = GetUniformLocation(name);
	if (loc >= 0)
	{
		ClearGLErrors();

		glUniform1i(loc, val);

		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR)
		{
			Logger::Write(__FUNCTION__, "Unable to set uniform '%s': %s", name, gluErrorString(glErr));
			assert(false);
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "%s: uniform '%s' not found.", ms_ShaderName.c_str(), name);
		if (mb_AssertOnNotFound)
		{
			assert(false);
		}
	}
}

void ShaderGL::PrintActiveUniforms()
{
	GLint nUniforms, size, location, maxLen;
	GLchar* name;
	GLsizei written;
	GLenum type;

	glGetProgramiv(mh_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
	glGetProgramiv(mh_program, GL_ACTIVE_UNIFORMS, &nUniforms);

	name = (GLchar *)malloc(maxLen);

	Logger::Write(__FUNCTION__, "------------------------------------------------");
	Logger::Write(__FUNCTION__, " Shader: %s", ms_ShaderName.c_str());
	Logger::Write(__FUNCTION__, " Location | Name");
	Logger::Write(__FUNCTION__, "------------------------------------------------");
	for (int i = 0; i < nUniforms; ++i)
	{
		glGetActiveUniform(mh_program, i, maxLen, &written, &size, &type, name);
		location = glGetUniformLocation(mh_program, name);
		Logger::Write(__FUNCTION__, " %-8d | %s", location, name);
	}

	free(name);
}

void ShaderGL::PrintActiveAttribs()
{
	GLint written, size, location, maxLength, nAttribs;
	GLenum type;
	GLchar * name;

	glGetProgramiv(mh_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
	glGetProgramiv(mh_program, GL_ACTIVE_ATTRIBUTES, &nAttribs);

	name = (GLchar *)malloc(maxLength);

	Logger::Write(__FUNCTION__, "------------------------------------------------");
	Logger::Write(__FUNCTION__, " Shader: %s", ms_ShaderName.c_str());
	Logger::Write(__FUNCTION__, " Index | Name");
	Logger::Write(__FUNCTION__, "------------------------------------------------");
	for (int i = 0; i < nAttribs; i++)
	{
		glGetActiveAttrib(mh_program, i, maxLength, &written, &size, &type, name);
		location = glGetAttribLocation(mh_program, name);
		Logger::Write(__FUNCTION__, " %-5d | %s", location, name);
	}

	free(name);
}

bool ShaderGL::Validate()
{
	if (!IsLinked())
		return false;

	GLint status;
	glValidateProgram(mh_program);
	glGetProgramiv(mh_program, GL_VALIDATE_STATUS, &status);

	if (GL_FALSE == status)
	{
		// Store log and return false
		int length = 0;
		m_LogString = "";

		glGetProgramiv(mh_program, GL_INFO_LOG_LENGTH, &length);

		if (length > 0)
		{
			char * c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(mh_program, length, &written, c_log);
			m_LogString = c_log;
			delete[] c_log;
		}

		return false;
	}
	else
	{
		return true;
	}
}

int ShaderGL::GetUniformLocation(const char* name)
{
	return glGetUniformLocation(mh_program, name);
}

bool ShaderGL::FileExists(const char* fileName)
{
	struct stat info;
	int ret = -1;

	ret = stat(fileName, &info);
	return 0 == ret;
}

void ShaderGL::ClearGLErrors()
{
	// clear all pending opengl errors

	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		glErr = glGetError();
	}
}

void ShaderGL::AssertOnNotFound(bool assert)
{
	// Flag to assert when uniform variable
	// not found.
	//

	mb_AssertOnNotFound = assert;
}

/* static */ ShaderGL::ShaderType ShaderGL::GetShaderTypeFromExt(const char* fileName)
{
	// There are no official file extensions for glsl shaders, but these
	// are the ones I use:
	// 
	//		.vert - vertex shader
	//		.frag - fragment shader
	//		.tesc - tessellation control shader
	//		.tese - tessellation evaluation shader
	//		.geom - geometry shader
	//		.comp - compute shader

	ShaderGL::ShaderType shaderType = ShaderGL::ShaderType::None;

	XString str(fileName);
	if (str.EndsWithNoCase(".vert") || str.EndsWithNoCase(".vs"))
		shaderType = ShaderGL::ShaderType::Vertex;
	else if (str.EndsWithNoCase(".frag") || str.EndsWithNoCase(".fs"))
		shaderType = ShaderGL::ShaderType::Fragment;
	else if (str.EndsWithNoCase(".geom"))
		shaderType = ShaderGL::ShaderType::Geometry;
	else if (str.EndsWithNoCase(".tessc"))
		shaderType = ShaderGL::ShaderType::TessControl;
	else if (str.EndsWithNoCase(".tesse"))
		shaderType = ShaderGL::ShaderType::TessEval;
	else
		assert(false);

	return shaderType;
}
