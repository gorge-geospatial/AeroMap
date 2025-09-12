#ifndef TIFFFILE_H
#define TIFFFILE_H

#include "tiffio.h"

#include "Calc.h"
#include "Logger.h"			// message logging facility

class TiffFile
{
public:

	// construction / destruction

	TiffFile(const char* fileName = nullptr);
	~TiffFile();

	bool Create(int width, int height);
	bool Load(const char* fileName);
	bool Save(const char* fileName);

	PixelType GetColor(UInt32 x, UInt32 y);
	void SetColor(UInt32 x, UInt32 y, UInt8 R, UInt8 G, UInt8 B, UInt8 A);
	void SetColor(UInt32 x, UInt32 y, PixelType pix);

	UInt32 GetWidth() const;
	UInt32 GetHeight() const;

	UInt16 GetSamplesPerPixel() const;		// 3 = RGB, 4 = RGBA, etc.
	UInt16 GetBitsPerSample() const;		// 8 = 8-bits per channel
	UInt16 GetCompression() const;			// type of compression, TIFFTAG_COMPRESSION
	UInt16 GetPhotometric() const;			// photometric interpretation, TIFFTAG_PHOTOMETRIC
	UInt16 GetPlanarConfig() const;			// planar configuration, TIFFTAG_PLANARCONFIG
	float  GetXResolution() const;			// number of pixels per ResolutionUnit in the ImageWidth direction
	float  GetYResolution() const;			// number of pixels per ResolutionUnit in the ImageLength direction

private:

	UInt32 m_Width;					// width of image, pixels
	UInt32 m_Height;
	UInt16 m_SamplesPerPixel;		// RGB = 3, RGBA = 4, etc.
	UInt16 m_BitsPerSample;			// 8 = 8-bits per channel
	UInt16 m_Compression;			// type of compression, TIFFTAG_COMPRESSION
	UInt16 m_Photometric;			// photometric interpretation, TIFFTAG_PHOTOMETRIC
	UInt16 m_PlanarConfig;			// planar configuration, TIFFTAG_PLANARCONFIG
	float  m_XResolution;			// TIFFTAG_XRESOLUTION
	float  m_YResolution;			// TIFFTAG_YRESOLUTION

	char ms_FileName[255];			// current file, "" = new file

	UInt32* mp_Raster;				// image data
};

#endif // #ifndef TIFFFILE_H
