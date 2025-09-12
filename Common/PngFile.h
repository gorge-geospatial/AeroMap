#ifndef PNGFILE_H
#define PNGFILE_H

#include "XString.h"
#include "MarkTypes.h"
#include "Calc.h"
#include "png.h"		// png lib header

class PngFile
{
public:

	PngFile(const char* fileName = nullptr);
	~PngFile();

	bool Create(UInt32 width, UInt32 height, int channels);
	int Save(const char* fileName = nullptr);

	UInt32 GetWidth() { return static_cast<UInt32>(m_image_width); }
	UInt32 GetHeight() { return static_cast<UInt32>(m_image_height); }
	int GetBitDepth() { return m_bit_depth; }

	PixelType GetPixel(UInt32 x, UInt32 y);
	void SetPixel(UInt32 x, UInt32 y, PixelType pix);

	void SetImageData(unsigned char* pData);

private:

	png_uint_32 m_image_width;			// image dimensions, pixels
	png_uint_32 m_image_height;
	unsigned long m_image_rowbytes;		// bytes per (transformed) row
	unsigned long m_ImageSize;			// size of image data, bytes

	png_structp m_png_ptr;
	png_infop m_info_ptr;
	int m_bit_depth;
	int m_color_type;
	int m_image_channels;
	unsigned char* mp_ImageData;		// ptr to image data

	XString ms_FileName;				// currently loaded png file

private:

	bool LoadFile(const char* fileName);
	int  ReadInit(FILE *infile);
	bool GetImage(double display_exponent);

};

#endif // PNGFILE_H
