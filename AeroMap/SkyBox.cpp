// SkyBox.cpp
// Implementation of skybox class.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//
// Notes:
//		- This implementation is based on a few different tutorials. They
//		  all seem to have their own idea of coordinate systems, winding
//		  orders & image orientations. etc. One upshot is that I had to
//		  manually orient some of the images - would be nice if I could 
//		  use them all in their "upright, left to right" order that you
//		  typically see.
//		- For years i was unclear on skybox vs cubemap vs textured cube vs ...
//		  It's now clear that a cubemap is a completely different concept than
//		  a cube with a texture applied to it. For one, a cubemap, isn't a cube
//		  at all, it's a highly specialized texture that's used as a lookup table
//		  casting rays out from the center to the point where they intersect the 
//		  cube. (OK, there's a cube involved, but you have to have some geometry
//		  to render of course).
//		- Even with a cubemap - which is a texture, not a geometric object at all - 
//		  you still need a cube - even though many tutorials gloss over it (I suppose
//		  because it seems so basic). Also, it must have 6 separate sides for the 
//		  texture mapping to work.
//	    - Using a cubemap, there are no texture coordinates (and no colors, normals
//		  or anything else), just 3d cube vertices.
//		- It seems obvious, but this is a little more nuanced than I first realized:
//		  you need to keep the view point at the center of the skybox; of course you
//		  want the world rendered around the viewer, but also the cubemap texture
//		  lookup assumes rays cast from the origin. As soon as you start to move away
//		  from the center of the cubemap, the effect begins to break down & you can
//		  see that you are inside a cube.
//

#include "ShaderLib.h"
#include "Logger.h"
#include "SkyBox.h"

const float SKY_SIZE = 25000.0F;

SkyBox::SkyBox()
	: m_Mode(Mode::Box)
	, m_textureID(0)
{
	CreateSphere();
	
	LoadCubeMap(
		"D:/Media/Textures/skybox/sky1/sky512_up.jpg",			// up
		"D:/Media/Textures/skybox/sky1/sky512_dn.jpg",			// down
		"D:/Media/Textures/skybox/sky1/sky512_lt.jpg",			// left
		"D:/Media/Textures/skybox/sky1/sky512_rt.jpg",			// right
		"D:/Media/Textures/skybox/sky1/sky512_ft.jpg",			// front
		"D:/Media/Textures/skybox/sky1/sky512_bk.jpg");			// back
	//LoadCubeMap(
	//	"D:/Media/Textures/skybox/sky0/up.jpg",			// up
	//	"D:/Media/Textures/skybox/sky0/down.jpg",		// down
	//	"D:/Media/Textures/skybox/sky0/left.jpg",		// left
	//	"D:/Media/Textures/skybox/sky0/right.jpg",		// right
	//	"D:/Media/Textures/skybox/sky0/front.jpg",		// front
	//	"D:/Media/Textures/skybox/sky0/back.jpg");		// back
	//LoadCubeMap(
	//	"D:/Media/Textures/skybox/sky_tallinn/up.jpg",			// up
	//	"D:/Media/Textures/skybox/sky_tallinn/down.jpg",		// down
	//	"D:/Media/Textures/skybox/sky_tallinn/left.jpg",		// left
	//	"D:/Media/Textures/skybox/sky_tallinn/right.jpg",		// right
	//	"D:/Media/Textures/skybox/sky_tallinn/front.jpg",		// front
	//	"D:/Media/Textures/skybox/sky_tallinn/back.jpg");		// back

	LoadShaders();

	float skyboxVertices[] =
	{
		// bottom
		-1.0f*SKY_SIZE,  1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE, -1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE, -1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE, -1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE,  1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE,  1.0f*SKY_SIZE, -1.0f*SKY_SIZE,

		// left
		-1.0f*SKY_SIZE, -1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE,  1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE, -1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE,  1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE, -1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE,  1.0f*SKY_SIZE,  1.0f*SKY_SIZE,

		// right
		1.0f*SKY_SIZE, -1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		1.0f*SKY_SIZE,  1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		1.0f*SKY_SIZE, -1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		1.0f*SKY_SIZE,  1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		1.0f*SKY_SIZE, -1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		1.0f*SKY_SIZE,  1.0f*SKY_SIZE, -1.0f*SKY_SIZE,

		// top
		-1.0f*SKY_SIZE, -1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE,  1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE,  1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE,  1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE, -1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE, -1.0f*SKY_SIZE,  1.0f*SKY_SIZE,

		// front
		-1.0f*SKY_SIZE,  1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE,  1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE,  1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE,  1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE,  1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE,  1.0f*SKY_SIZE,  1.0f*SKY_SIZE,

		// back
		-1.0f*SKY_SIZE, -1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE, -1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE, -1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE, -1.0f*SKY_SIZE, -1.0f*SKY_SIZE,
		 1.0f*SKY_SIZE, -1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
		-1.0f*SKY_SIZE, -1.0f*SKY_SIZE,  1.0f*SKY_SIZE,
	};

	m_vbBox.CreateP(GL_TRIANGLES, skyboxVertices, _countof(skyboxVertices) / 3);
}

SkyBox::~SkyBox()
{

}

void SkyBox::Render()
{
	if (m_Mode == Mode::None)
		return;

	glm::mat4 matMVP = GLManager::GetMatProj() * GLManager::GetMatView() * GLManager::GetMatModel();

	GLManager::PushWind(GL_CW);

	switch (m_Mode)
	{
	case Mode::Sphere:
		{
			GLManager::GetShader("PC")->Activate();
			GLManager::GetShader("PC")->SetUniform("MVP", matMVP);

			m_SphereSky.Render();
		}
		break;
	case Mode::Box:
		{
			m_shader.Activate();
			m_shader.SetUniform("projection", GLManager::GetMatProj());

			glm::mat4 view = GLManager::GetMatView(); // remove translation from the view matrix
			// clear the translation so viewer always at center
			// of skybox
			view[3][0] = 0.0;		// x
			view[3][1] = 0.0;		// y
			view[3][2] = 0.0;		// z
			m_shader.SetUniform("view", view);

			// draw skybox last

			// change depth function so depth test passes when values are equal to depth buffer's content
			glDepthFunc(GL_LEQUAL);

			// skybox cube
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
			m_vbBox.Render();
			glDepthFunc(GL_LESS); // set depth function back to default
		}
		break;
	}

	GLManager::PopWind();
}

void SkyBox::CreateSphere()
{
	// Create the sky sphere vertex buffer.
	//

	// use SphereGL to generate our geometry
	m_SphereSky.Create(VertexBufferGL::VertexType::PC, SKY_SIZE, 2);

	// now apply fixed colors
	for (int i = 0; i < m_SphereSky.GetVertexCount(); ++i)
	{
		VertexBufferGL::VertexPC* pVx = m_SphereSky.GetVertexPC(i);
		if (pVx)
		{
			// "dark" sky from neahkanie flyby: RGB=(63,78,97)=(0.247,0.306,0.380)
			// "light sky (i can see): RGB=(103,131,155)=(0.404,0.514,0.608)
			//base color on distance from horizon (just pVx->y value i believe)

			double sf = 1.0 - pVx->y / m_SphereSky.GetRadius();
			pVx->r = static_cast<float>(Lerp(sf, 0.14, 0.30));
			pVx->g = static_cast<float>(Lerp(sf, 0.20, 0.41));
			pVx->b = static_cast<float>(Lerp(sf, 0.28, 0.50));

			// since we have access to the vertices, let's go ahead and translate it
			// so it's centered at the center of the terrain
			//pVx->x += static_cast<float>(SKY_SIZE * 0.5);
			//pVx->z -= static_cast<float>(SKY_SIZE * 0.5);
		}
	}
	m_SphereSky.ApplyChanges();
}

bool SkyBox::LoadCubeMap(const char* fileUp, const char* fileDn,
	const char* fileLt, const char* fileRt,
	const char* fileFt, const char* fileBk)
{
	// Load 6 individual cube map textures.
	//
	// See: https://learnopengl.com/Advanced-OpenGL/Cubemaps
	//
	// Inputs:
	//		fileUp/Dn = top/bottom textures
	//		fileLt/Rt = right/left textures
	//		fileFt/Bk = front/back textures
	//

	// generate the cube map texture (even though it uses
	// 6 images, it's still a single texture)

	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

	if (!m_ImageUp.LoadFile(fileUp))
	{
		Logger::Write(__FUNCTION__, "Unable to load '%s'", fileUp);
		return false;
	}
	if (!m_ImageDn.LoadFile(fileDn))
	{
		Logger::Write(__FUNCTION__, "Unable to load '%s'", fileDn);
		return false;
	}
	if (!m_ImageLt.LoadFile(fileLt))
	{
		Logger::Write(__FUNCTION__, "Unable to load '%s'", fileLt);
		return false;
	}
	if (!m_ImageRt.LoadFile(fileRt))
	{
		Logger::Write(__FUNCTION__, "Unable to load '%s'", fileRt);
		return false;
	}
	if (!m_ImageFt.LoadFile(fileFt))
	{
		Logger::Write(__FUNCTION__, "Unable to load '%s'", fileFt);
		return false;
	}
	if (!m_ImageBk.LoadFile(fileBk))
	{
		Logger::Write(__FUNCTION__, "Unable to load '%s'", fileBk);
		return false;
	}

	// convert to opengl texture compatible format
	//m_ImageUp.ConvertToOpenGL();
	//m_ImageDn.ConvertToOpenGL();
	//m_ImageLt.ConvertToOpenGL();
	//m_ImageRt.ConvertToOpenGL();
	//m_ImageFt.ConvertToOpenGL();
	//m_ImageBk.ConvertToOpenGL();

	int w = m_ImageUp.GetWidth();
	int h = m_ImageUp.GetHeight();

	// assert square
	assert(m_ImageUp.GetWidth() == m_ImageUp.GetHeight());
	assert(m_ImageDn.GetWidth() == m_ImageDn.GetHeight());
	assert(m_ImageLt.GetWidth() == m_ImageLt.GetHeight());
	assert(m_ImageRt.GetWidth() == m_ImageRt.GetHeight());
	assert(m_ImageFt.GetWidth() == m_ImageFt.GetHeight());
	assert(m_ImageBk.GetWidth() == m_ImageBk.GetHeight());

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageRt.GetData());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageLt.GetData());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageFt.GetData());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageBk.GetData());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageUp.GetData());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageDn.GetData());

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return true;
}

bool SkyBox::LoadShaders()
{
	bool status = m_shader.BuildShaderFromString("skybox", ShaderLib::GetVertexShaderSkyBox(), ShaderLib::GetFragmentShaderSkyBox());
	assert(status);
	return status;
}
