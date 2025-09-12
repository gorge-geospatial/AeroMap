#ifndef TEXTUREGL_H
#define TEXTUREGL_H

#include <GL/glew.h>			// OpenGL

#include "MarkTypes.h"
#include "XString.h"
#include "ImageFile.h"		// image file manager

class TextureGL
{
public:

	TextureGL();
	TextureGL(const char* fileName, int textureUnit = 0);
	~TextureGL();

	bool Create(int width, int height, const UInt8* data, int textureUnit = 0);	// create from data
	bool Load(const char* fileName, int textureUnit = 0);		// load a texture from disk
	void Save();												// save modified texture
	void Update();												// update bound gl texture
	void Activate();											// make this texture active

	PixelType GetPixel(UInt16 x, UInt16 y);
	PixelType GetPixel(double tu, double tv);
	void SetPixel(UInt16 x, UInt16 y, PixelType pix);

	int GetWidth() { return m_ImageFile.GetWidth(); }
	int GetHeight() { return m_ImageFile.GetHeight(); }

	int GetTextureUnit() { return m_TextureUnit; }
	bool IsValid() { return m_tid > 0; }
	bool IsDirty() { return mb_IsDirty; }

private:

	ImageFile m_ImageFile;		// original image file
	GLuint m_tid;				// OpenGL texture ID
	int m_TextureUnit;			// texture unit for this texture, 0 to GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS-1
								// (think "texture unit" correlates to sampler in shader)
	bool mb_IsDirty;			// texture has been modified since save/load
};

#endif // #ifndef TEXTUREGL_H
