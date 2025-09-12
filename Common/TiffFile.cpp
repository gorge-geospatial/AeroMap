// TiffFile.cpp
// Manager for TIFF files.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "TiffFile.h"

TiffFile::TiffFile(const char* fileName)
	: m_Width(0)
	, m_Height(0)
	, m_SamplesPerPixel(0)
	, m_BitsPerSample(0)
	, m_Compression(0)
	, m_Photometric(0)
	, m_PlanarConfig(0)
	, mp_Raster(nullptr)
	, m_XResolution(0.0)
	, m_YResolution(0.0)
{
	memset(ms_FileName, 0, sizeof(ms_FileName));

	if (fileName != nullptr)
		Load(fileName);
}

TiffFile::~TiffFile()
{
	if (mp_Raster != nullptr)
		_TIFFfree(mp_Raster);
}

bool TiffFile::Create(int width, int height)
{
	// Create a new bitmap in memory.

	memset(ms_FileName, 0, sizeof(ms_FileName));

	m_Width = width;
	m_Height = height;

	if (mp_Raster)		// delete existing data, if any
	{
		delete mp_Raster;
		mp_Raster = nullptr;
	}

	// currently, the only format supported
	m_SamplesPerPixel = 3;
	m_BitsPerSample = 8;

	// you should use the LIBTIFF library memory allocation functions to get space to store the image
	UInt32 npixels = width*height;
	mp_Raster = (UInt32 *)_TIFFmalloc(npixels * sizeof(UInt32));

	return (mp_Raster != nullptr);
}

bool TiffFile::Load(const char* fileName)
{
	// Load TIFF file.

	int status = 0;

	TIFF* pTiff = TIFFOpen(fileName, "r");
	if (pTiff == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open file: %s.", fileName);
		return false;
	}

	TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &m_Width);
	TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &m_Height);

	m_SamplesPerPixel = 0;
	status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &m_SamplesPerPixel);

	m_BitsPerSample = 0;
	status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &m_BitsPerSample);

	m_Compression = 0;
	status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &m_Compression);

	m_Photometric = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &m_Photometric);

	m_PlanarConfig = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &m_PlanarConfig);

	status = TIFFGetField(pTiff, TIFFTAG_XRESOLUTION, &m_XResolution);
	status = TIFFGetField(pTiff, TIFFTAG_YRESOLUTION, &m_YResolution);

	// use LIBTIFF library memory allocation functions
	UInt32 pixelCount = m_Width*m_Height;
	mp_Raster = (UInt32 *)_TIFFmalloc(pixelCount * sizeof(UInt32));
	assert(mp_Raster);

	status = TIFFReadRGBAImage(pTiff, m_Width, m_Height, mp_Raster, 0);
	if (status != 1)
	{
		Logger::Write(__FUNCTION__, "Unable to read RGBA image.");
		assert(false);
	}
	TIFFClose(pTiff);

	strncpy(ms_FileName, fileName, sizeof(ms_FileName));

	return (mp_Raster != nullptr);
}

bool TiffFile::Save(const char* fileName)
{
	// no data to write
	if (mp_Raster == nullptr)
		return false;

	TIFF* pOutFile = TIFFOpen(fileName, "w");
	if (pOutFile == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open file: %s.", fileName);
		return false;
	}

	// Then format of the pixel information is store in the order RGBA, into the array, each channel occupies 1 byte(char).

	// Now we need to set the tags in the new image file, and the essential ones are the following :

	assert(m_SamplesPerPixel > 0);
	assert(m_BitsPerSample > 0);

	TIFFSetField(pOutFile, TIFFTAG_IMAGEWIDTH, m_Width);					// set the width of the image
	TIFFSetField(pOutFile, TIFFTAG_IMAGELENGTH, m_Height);					// set the height of the image
	TIFFSetField(pOutFile, TIFFTAG_SAMPLESPERPIXEL, m_SamplesPerPixel);		// set number of channels per pixel
	TIFFSetField(pOutFile, TIFFTAG_BITSPERSAMPLE, m_BitsPerSample);			// set the size of the channels
	TIFFSetField(pOutFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);		// set the origin of the image.
	TIFFSetField(pOutFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(pOutFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	// We will use most basic image data storing method provided by the library to write the data into the file,
	// this method uses strips, and we are storing a line(row) of pixel at a time. This following code writes 
	// the data from the char array image into the file :

	// length in memory of one row of pixel in the image
	tsize_t linebytes = m_SamplesPerPixel * m_Width;

	// Allocate memory to store the pixels of current row
	UInt8* pScanLine = nullptr;
	if (TIFFScanlineSize(pOutFile) == linebytes)
		pScanLine = (unsigned char *)_TIFFmalloc(linebytes);
	else
		pScanLine = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(pOutFile));

	// We set the strip size of the file to be size of one row of pixels
	TIFFSetField(pOutFile, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(pOutFile, m_Width * m_SamplesPerPixel));

	// support writing 24 & 32 bit images
	switch (m_SamplesPerPixel) {
	case 3:
		{
			// create 24 bit image buffer

			UInt8* pImage = new UInt8[m_Width*m_Height*m_SamplesPerPixel];
			for (UInt32 x = 0; x < m_Width; ++x)
			{
				for (UInt32 y = 0; y < m_Height; ++y)
				{
					int index = y * m_Width + x;
					pImage[index * 3] = TIFFGetR(mp_Raster[index]);
					pImage[index * 3 + 1] = TIFFGetG(mp_Raster[index]);
					pImage[index * 3 + 2] = TIFFGetB(mp_Raster[index]);
				}
			}

			// write image to the file one strip at a time
			for (UInt32 row = 0; row < m_Height; ++row)
			{
				memcpy(pScanLine, &pImage[(m_Height - row - 1)*linebytes], linebytes);
				if (TIFFWriteScanline(pOutFile, pScanLine, row, 0) < 0)
				{
					Logger::Write(__FUNCTION__, "TIFFWriteScanline() failed.");
					break;
				}
			}
			delete pImage;
		}
		break;
	case 4:
		// write image to the file one strip at a time
		for (UInt32 row = 0; row < m_Height; ++row)
		{
			memcpy(pScanLine, &mp_Raster[(m_Height - row - 1)*linebytes], linebytes);
			if (TIFFWriteScanline(pOutFile, pScanLine, row, 0) < 0)
			{
				Logger::Write(__FUNCTION__, "TIFFWriteScanline() failed.");
				break;
			}
		}
		break;
	default:
		Logger::Write(__FUNCTION__, "Unsupported image format, TIFFTAG_SAMPLESPERPIXEL = %u.", m_SamplesPerPixel);
		break;
	}

	// close output file and destroy the buffer
	TIFFClose(pOutFile);

	if (pScanLine)
		_TIFFfree(pScanLine);

	return true;
}

PixelType TiffFile::GetColor(UInt32 x, UInt32 y)
{
	PixelType pix;
	if (x < m_Width && y < m_Height)
	{
		int index = m_Width * y + x;

		pix.R = (UInt8)TIFFGetR(mp_Raster[index]);
		pix.G = (UInt8)TIFFGetG(mp_Raster[index]);
		pix.B = (UInt8)TIFFGetB(mp_Raster[index]);
		pix.A = (UInt8)TIFFGetA(mp_Raster[index]);

	}
	return pix;
}

void TiffFile::SetColor(UInt32 x, UInt32 y, PixelType pix)
{
	SetColor(x, y, pix.R, pix.G, pix.B, pix.A);
}

void TiffFile::SetColor(UInt32 x, UInt32 y, UInt8 R, UInt8 G, UInt8 B, UInt8 A)
{
	if (x < m_Width && y < m_Height)
	{
		// offset of 32-bit pixel

		int index = m_Width * y + x;

		mp_Raster[index] = (A << 24 | B << 16 | G << 8 | R);
	}
}

UInt32 TiffFile::GetWidth() const
{
	// Return width of image, pixels.

	return m_Width;
}

UInt32 TiffFile::GetHeight() const
{
	// Return height of image, pixels.

	return m_Height;
}

UInt16 TiffFile::GetSamplesPerPixel() const
{
	// 3 = RGB, 4 = RGBA, etc.
	return m_SamplesPerPixel;
}

UInt16 TiffFile::GetBitsPerSample() const
{
	// 8 = 8-bits per channel
	return m_BitsPerSample;
}

UInt16 TiffFile::GetCompression() const
{
	// type of compression, TIFFTAG_COMPRESSION
	return m_Compression;
}

UInt16 TiffFile::GetPhotometric() const
{
	// photometric interpretation, TIFFTAG_PHOTOMETRIC
	return m_Photometric;
}

UInt16 TiffFile::GetPlanarConfig() const
{
	// planar configuration, TIFFTAG_PLANARCONFIG
	return m_PlanarConfig;
}

float TiffFile::GetXResolution() const
{
	// number of pixels per ResolutionUnit in the ImageWidth direction
	return m_XResolution;
}

float TiffFile::GetYResolution() const
{
	// number of pixels per ResolutionUnit in the ImageLength direction
	return m_YResolution;
}
