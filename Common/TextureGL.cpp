// TextureGL.cpp
// OpenGL texture manager
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//
//TODO:
// - "texture unit" and how it relates to shader still a little vague
//

#include <assert.h>

#include <GL/glew.h>

#include "TextureGL.h"
#include "BmpFile.h"

TextureGL::TextureGL()
	: m_tid(0)
	, m_TextureUnit(0)
	, mb_IsDirty(false)
{
}

TextureGL::TextureGL(const char* fileName, int textureUnit)
	: m_tid(0)
	, m_TextureUnit(0)
{
	Load(fileName, textureUnit);
}

TextureGL::~TextureGL()
{
	if (m_tid > 0)
		glDeleteTextures(1, &m_tid);
}

bool TextureGL::Create(int width, int height, const UInt8* data, int textureUnit)
{
	// Create texture from data.
	// 
	// Inputs:
	//		width  = image width
	//		height = image height
	//		data   = RGBA pixel data
	// Outputs:
	//		return = true on success
	//

	if (textureUnit < 0 || textureUnit >= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
		return false;

	// Copy file to OpenGL
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glGenTextures(1, &m_tid);
	glBindTexture(GL_TEXTURE_2D, m_tid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	m_TextureUnit = textureUnit;

	mb_IsDirty = false;

	return true;
}

bool TextureGL::Load(const char* fileName, int textureUnit)
{
	// Load texture file

	if (textureUnit < 0 || textureUnit >= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
		return false;

	if (!m_ImageFile.LoadFile(fileName))
		return false;

	// convert to opengl texture compatible format
	//m_ImageFile.ConvertToOpenGL();

	// Copy file to OpenGL
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glGenTextures(1, &m_tid);
	glBindTexture(GL_TEXTURE_2D, m_tid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_ImageFile.GetWidth(), m_ImageFile.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageFile.GetData());
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	m_TextureUnit = textureUnit;

	mb_IsDirty = false;

	return true;
}

void TextureGL::Save()
{
	// Save texture.
	// 

	if (m_ImageFile.GetFileName().EndsWithNoCase(".png"))
	{
		// currently, only supporting .png save

		m_ImageFile.SavePngFile(m_ImageFile.GetFileName().c_str());
	}
	else
	{
		assert(false);
	}

	mb_IsDirty = false;
}

void TextureGL::Update()
{
	// Update bound gl texture.
	//

	Activate();

	// convert to opengl texture compatible format & blit to output image
	//m_ImageFile.ConvertToOpenGL();

	// download texture data
	//m_ImageFile.IdentityFilter();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_ImageFile.GetWidth(), m_ImageFile.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageFile.GetData());
}

void TextureGL::Activate()
{
	// activating the texture unit (i think) sets up for the associated
	// sampler2D slot in the shader
	glActiveTexture(GL_TEXTURE0 + m_TextureUnit);

	// make this texture active (specifically, make this the active
	// "2D" texture - ignoring all other bind targets)
	glBindTexture(GL_TEXTURE_2D, m_tid);
}

PixelType TextureGL::GetPixel(UInt16 x, UInt16 y)
{
	// Read a single pixel from the texture.
	//
	// Inputs:
	//		(x,y) = pixel coordinates
	//

	if (x >= m_ImageFile.GetWidth() || y >= m_ImageFile.GetHeight())
		return PixelType();

	return m_ImageFile.GetPixel(x, y);
}

PixelType TextureGL::GetPixel(double tu, double tv)
{
	// Read a single pixel from the texture.
	//
	// Inputs:
	//		(tu,tv) = relative image offsets
	//

	if ((tu < 0.0 || tu > 1.0) || (tv < 0.0 || tv > 1.0))
		return PixelType();

	UInt16 x = static_cast<UInt16>((m_ImageFile.GetWidth() - 1)*tu);
	UInt16 y = static_cast<UInt16>((m_ImageFile.GetHeight() - 1)*tv);

	return m_ImageFile.GetPixel(x, y);
}

void TextureGL::SetPixel(UInt16 x, UInt16 y, PixelType pix)
{
	// Write a single pixel to the texture.
	//
	// Inputs:
	//		(x,y) = pixel coordinates
	//

	if (x >= m_ImageFile.GetWidth() || y >= m_ImageFile.GetHeight())
		return;

	// always write to input image
	m_ImageFile.SetPixel(x, y, pix);

	mb_IsDirty = true;
}
