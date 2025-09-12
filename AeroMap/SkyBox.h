#ifndef SKYBOX_H
#define SKYBOX_H

#include "GLManager.h"
#include "Sphere.h"
#include "TextureGL.h"
#include "VertexBufferGL.h"

class SkyBox
{
public:

	enum class Mode
	{
		None,		// do not render
		Sphere,		// render colored sphere with no texture
		Box			// render textured skybox
	};

public:

	SkyBox();
	~SkyBox();

	void Render();
	bool LoadCubeMap(const char* fileUp, const char* fileDn,
					 const char* fileLt, const char* fileRt,
					 const char* fileFt, const char* fileBk);

	void SetMode(Mode mode) { m_Mode = mode; }
	Mode GetMode() { return m_Mode; }

	unsigned int GetTextureUnit() { return m_textureID; }

private:

	Mode m_Mode;					// current rendering mode
	Sphere m_SphereSky;				// skydome sphere
	VertexBufferGL m_vbBox;

	ImageFile m_ImageUp;
	ImageFile m_ImageDn;
	ImageFile m_ImageLt;
	ImageFile m_ImageRt;
	ImageFile m_ImageFt;
	ImageFile m_ImageBk;

	unsigned int m_textureID;		// cubemap texture

	ShaderGL m_shader;				// linked shaders for skybox

private:

	void CreateSphere();
	bool LoadShaders();
};

#endif // #ifndef SKYBOX_H
