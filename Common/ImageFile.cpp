// ImageFile.cpp
//
// Image file handler class. Thin container for various
// image file type handlers.
//
// Notes:
//		- internal data format is RGBA, regardless of source
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>

#include "BmpFile.h"		// interface to bitmap class
#include "JpegFile.h"		// interface to jpeg class
#include "PngFile.h"		// interface to png class
#include "ImageFile.h"

#include <io.h>
#define open _open
#define lseek _lseek
#define read _read
#define write _write
#define close _close
#define chmod _chmod

ImageFile::ImageFile()
	: mp_Image(nullptr)
{
}

ImageFile::ImageFile(const char* fileName)
	: mp_Image(nullptr)
{
	// constructor that accepts graphics file name

	LoadFile(fileName);
}

ImageFile::~ImageFile()
{
	DestroyImage(mp_Image);
}

bool ImageFile::LoadFile(const char* fileName)
{
	bool bReturn = false;

	XString sFileName = fileName;
	sFileName.Trim();

	// quick check for file existence before instantiating image file handlers
	int fd = open(sFileName.c_str(), O_RDONLY | O_BINARY);
	if (fd > 0)
	{
		close(fd);

		if (sFileName.EndsWithNoCase(".jpg") || sFileName.EndsWithNoCase(".jpeg"))
			LoadJpegFile(sFileName.c_str());
		else if (sFileName.EndsWithNoCase(".bmp"))
			LoadBmpFile(sFileName.c_str());
		else if (sFileName.EndsWithNoCase(".png"))
			LoadPngFile(sFileName.c_str());
		else
			assert(false);

		ms_FileName = sFileName;
		bReturn = true;
	}

	return bReturn;
}

void ImageFile::LoadPngFile(const char* fileName)
{
	// load PNG from disk - internally, all images regardless of source are stored
	// as 32-bit arrays
	//

	PngFile* pPng = new PngFile(fileName);

	if (pPng->GetWidth() > 0)
	{
		mp_Image = CreateImage(pPng->GetWidth(), pPng->GetHeight());

		for (UInt32 x = 0; x < pPng->GetWidth(); ++x)
		{
			for (UInt32 y = 0; y < pPng->GetHeight(); ++y)
			{
				PixelType pix = pPng->GetPixel(x, y);
				SetPixel(mp_Image, x, y, pix);
			}
		}
	}

	delete pPng;		// release it
}

void ImageFile::SavePngFile(const char* fileName)
{
	// Write PNG file.
	//
	// Inputs:
	//		fileName = name of .png file to write
	//

	PngFile* pPngFile = new PngFile();
	pPngFile->Create(GetWidth(), GetHeight(), 4);

	// block copy all data, assumes 4 bytes per pixel
	// and source/dest blocks are same size
	pPngFile->SetImageData(mp_Image->pData);

	int status = pPngFile->Save(fileName);
	delete pPngFile;

	if (status != 0)
		Logger::Write(__FUNCTION__, "Error writing PNG file '%s'.", fileName);
}

void ImageFile::LoadBmpFile(const char* fileName)
{
	// load bmp from disk - internally, all images regardless of source are stored
	// as 32-bit arrays
	//

	BmpFile* pBmp = new BmpFile();

	if (pBmp->Load(fileName))
	{
		// allocate our image

		mp_Image = CreateImage(pBmp->GetWidth(), pBmp->GetHeight());

		// populate our input/output images

		UInt8* pSrc = pBmp->GetData();
		int pixelCount = pBmp->GetHeight()*pBmp->GetWidth();

		UInt8* pDest = mp_Image->pData;

		for (int i = 0; i < pixelCount; ++i)
		{
			*pDest++ = *pSrc++;		// src is 3 bytes
			*pDest++ = *pSrc++;
			*pDest++ = *pSrc++;
			*pDest++ = 0;			// default alpha to 0
		}
	}

	delete pBmp;		// release it
}

void ImageFile::SaveBmpFile(const char* fileName)
{
	// write bmp file
	//

	ImageType* pImage = mp_Image;
	BmpFile* pBmp = new BmpFile();		// type-specific io embedded in BMP class

	pBmp->Create(pImage->Width, pImage->Height);
	for (int x = 0; x < pImage->Width; ++x)
	{
		for (int y = 0; y < pImage->Height; ++y)
		{
			PixelType pix = GetPixel(x, y);
			pBmp->SetColor(x, y, pix.R, pix.G, pix.B);
		}
	}

	pBmp->Save(fileName);

	delete pBmp;
}

void ImageFile::LoadJpegFile(const char* fileName)
{
	UInt8* pBuf = nullptr;			// pixel data buffer
	unsigned int Width = 0;
	unsigned int Height = 0;

	// read to buffer tmp
	pBuf = JpegFile::JpegFileToRGB(fileName, &Width, &Height);
	if (!pBuf)
	{
		Logger::Write(__FUNCTION__, "JpegFile::JpegFileToRGB() failed");
		return;
	}

	// allocate our image

	mp_Image = CreateImage(Width, Height);

	// now, copy it to internal pixel structure while mapping from 24 to 32 bit
	// pixel format

	UInt8* pSrc = pBuf;
	UInt8* pDest = mp_Image->pData;

	for (unsigned int y = 0; y < Height; ++y)
	{
		for (unsigned int x = 0; x < Width; ++x)
		{
			*pDest++ = *pSrc++;
			*pDest++ = *pSrc++;
			*pDest++ = *pSrc++;
			*pDest++ = 0;
		}
	}

	delete pBuf;
}

void ImageFile::SaveJpegFile(const char* fileName, int quality /* = 75 */)
{
	// write Jpeg file
	//

	ImageType* pImage = mp_Image;

	// copy to 24-bit buffer

	UInt8* pBuf = new UInt8[pImage->Width*pImage->Height * 3];

	UInt8* pDest = pBuf;
	UInt8* pSrc = pImage->pData;
	for (int y = 0; y < pImage->Height; ++y)
	{
		for (int x = 0; x < pImage->Width; ++x)
		{
			*pDest++ = *pSrc++;
			*pDest++ = *pSrc++;
			*pDest++ = *pSrc++;
			pSrc++;
		}
	}

	// write it
	bool bStatus = JpegFile::RGBToJpegFile(	fileName,
											pBuf,
											pImage->Width,
											pImage->Height,
											true,				// true = color, false = grayscale
											quality);			// 0..100
	if (!bStatus)
	{
		Logger::Write(__FUNCTION__, "Error writing Jpeg file '%s'.", fileName);
	}

	delete pBuf;
}

void ImageFile::RecreateImage(ImageType** ppImage, UInt16 width, UInt16 height)
{
	// Free and reallocate an image descriptor
	//
	// Inputs:
	//		pImage = address of ptr to image to be replaced
	//		Width = width of image, pixels
	//		Height = height of image, pixels
	//
	// Outputs:
	//		pImage = address of new image descriptor
	//

	DestroyImage(*ppImage);					// release existing image
	*ppImage = CreateImage(width, height);	// create new image
}

void ImageFile::Create(UInt16 width, UInt16 height)
{
	// Create a new image
	//
	// Inputs:
	//		width = width of image, pixels
	//		height = height of image, pixels
	//

	// recreate image with the new size
	RecreateImage(&mp_Image, width, height);

	// blank image

	UInt32* pDest = (UInt32*)mp_Image->pData;
	for (UInt32 pixelIdx = 0; pixelIdx < (UInt32)(width*height); ++pixelIdx)
	{
		*pDest++ = 0;
	}
}

ImageFile::ImageType* ImageFile::CreateImage(int Width, int Height)
{
	// Allocate an image descriptor.
	//
	// Inputs:
	//		Width = width of image, pixels
	//		Height = height of image, pixels
	// Outputs:
	//		return = new image descriptor, with data allocated
	//

	ImageType* pReturn = new ImageType();

	pReturn->Width = Width;
	pReturn->Height = Height;
	pReturn->pData = new UInt8[Width * Height * 4];

	return pReturn;
}

void ImageFile::DestroyImage(ImageType* pImage)
{
	// free an image descriptor
	//

	if (!pImage)
		return;

	delete pImage->pData;		// delete image data
	delete pImage;				// delete descriptor
}

void ImageFile::SetPixel(UInt16 x, UInt16 y, PixelType pix)
{
	// Set a single pixel.
	// 
	// Inputs:
	//		x,y      	= coordinates of pixel
	//		pix		   	= 32 bit pixel color
	//

	SetPixel(mp_Image, x, y, pix);
}

void ImageFile::SetPixel(ImageType* pImage, UInt16 x, UInt16 y, PixelType pix)
{
	// Set a single pixel, overload that accepts image ptr
	//
	// Inputs:
	// 		x,y	= coordinates of pixel
	// 		pix	= 32 bit pixel color
	//

	if (!pImage)
		return;

	if (x > pImage->Width - 1 || y > pImage->Height - 1)
		return;

	// calculate pixel offset
	UInt32 offset = CalcOffset(pImage, x, y);

	pImage->pData[offset] = pix.R;
	pImage->pData[offset + 1] = pix.G;
	pImage->pData[offset + 2] = pix.B;
	pImage->pData[offset + 3] = pix.A;
}

PixelType ImageFile::GetPixel(UInt16 x, UInt16 y)
{
	// Get a single pixel
	//
	// Inputs:
	//		x,y = coordinates of pixel
	//
	// Outputs:
	//		return = 32 bit pixel color
	//

	PixelType pix = GetPixel(mp_Image, x, y);

	return pix;
}

PixelType ImageFile::GetPixel(ImageType* pImage, UInt16 x, UInt16 y)
{
	// get a single pixel, overload that accepts an image
	//
	// inputs:
	//		pImage = image descriptor
	//		x,y    = coordinates of pixel
	//
	// output:
	//		return = 32 bit pixel color

	PixelType pix;

	if (!pImage)
		return pix;

	if (x > pImage->Width - 1 || y > pImage->Height - 1)
		return pix;

	// calculate pixel offset
	UInt32 offset = CalcOffset(pImage, x, y);

	pix.R = pImage->pData[offset];
	pix.G = pImage->pData[offset + 1];
	pix.B = pImage->pData[offset + 2];
	pix.A = pImage->pData[offset + 3];

	return pix;
}

const UInt8* ImageFile::GetData()
{
	// Return ptr to image's raw ARGB data.
	//

	return mp_Image->pData;
}

UInt32 ImageFile::CalcOffset(ImageType* pImage, int x, int y)
{
	return (y * pImage->Width + x) * 4;
}

void ImageFile::FlipVert()
{
	// Flip image vertically.
	//

	int rowCount = GetHeight();
	int stride = GetWidth() * 4;		// bytes in row

	UInt8* pSrc = mp_Image->pData;							// row 0, src image
	ImageType* pImage = CreateImage(GetWidth(), GetHeight());
	UInt8* pDst = pImage->pData + (rowCount-1)*stride;		// last row, dst image

	for (int i = 0; i < rowCount; ++i)
	{
		memcpy(pDst, pSrc, stride);
		pDst -= stride;
		pSrc += stride;
	}

	memcpy(mp_Image, pImage, GetWidth() * GetHeight() * 4);
}

void ImageFile::ConvertToOpenGL()
{
	// Convert data buffer to RGBA format for 
	// use in OpenGL RGBA buffers.
	//
	// Since internal format currently RGBA,
	// does nothing.
	//

}
