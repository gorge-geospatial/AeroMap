#ifndef IMAGEFILE_H
#define IMAGEFILE_H

#include "MarkTypes.h"		// global type definitions
#include "XString.h"		// string class
#include "Calc.h"

class ImageFile
{
public:

	// construction / destruction

	ImageFile();
	ImageFile(const char* fileName);
	virtual ~ImageFile();

	void Create(UInt16 width, UInt16 height);

	// image access

	PixelType GetPixel(UInt16 x, UInt16 y);
	void SetPixel(UInt16 x, UInt16 y, PixelType pix);

	const UInt8* GetData();		// return ptr to image's raw RGBA data

	// in-lines

	UInt32 GetHeight() { return (mp_Image ? mp_Image->Height : 0); }
	UInt32 GetWidth()  { return (mp_Image ? mp_Image->Width : 0); }

	bool IsValid(int x, int y) { return (x > -1 && x < mp_Image->Width) && (y > -1 && y < mp_Image->Height); }
	bool IsValid(PointType pt) { return (pt.x > -1 && pt.x < mp_Image->Width) && (pt.y > -1 && pt.y < mp_Image->Height); }

	XString GetFileName() { return ms_FileName; }

	// file io

	bool LoadFile(const char* fileName);
	void SaveJpegFile(const char* fileName, int quality = 75);
	void SaveBmpFile(const char* fileName);
	void SavePngFile(const char* fileName);

	// transformations

	void ConvertToOpenGL();
	void FlipVert();

private:

	struct ImageType		// image descriptor
	{
		int Height;				// height, pixels
		int Width;				// width, pixels
		UInt8* pData;			// image data
	};
	ImageType* mp_Image;

	XString ms_FileName;		// graphics file name

private:

	ImageType* CreateImage(int width, int height);
	void RecreateImage(ImageType** ppImage, UInt16 width, UInt16 height);
	void DestroyImage(ImageType* pImage);

	void LoadBmpFile(const char* fileName);
	void LoadJpegFile(const char* fileName);
	void LoadPngFile(const char* fileName);

	PixelType GetPixel(ImageType* pImage, UInt16 x, UInt16 y);
	void SetPixel(ImageType* pImage, UInt16 x, UInt16 y, PixelType pix);

	// in-lines

	// calculate the byte offset into internal data buffer (forward or reverse) for pixel at (x,y)
	inline UInt32 CalcOffset(ImageType* pImage, int x, int y);
};

#endif // #ifndef IMAGEFILE_H
