// TestTiffLib
// Test harness for my build of libTiff.
//
// Baseline TIFF Tags
//		Code		Name				Short description
//		Dec	Hex
//		254	00FE	NewSubfileType		A general indication of the kind of data contained in this subfile.
//		255	00FF	SubfileType			A general indication of the kind of data contained in this subfile.
//		256	0100	ImageWidth			The number of columns in the image, i.e., the number of pixels per row.
//		257	0101	ImageLength			The number of rows of pixels in the image.
//		258	0102	BitsPerSample		Number of bits per component.
//		259	0103	Compression			Compression scheme used on the image data.
//		262	0106	PhotometricInterpretation	The color space of the image data.
//		263	0107	Threshholding		For black and white TIFF files that represent shades of gray, the technique used to convert from gray to black and white pixels.
//		264	0108	CellWidth			The width of the dithering or halftoning matrix used to create a dithered or halftoned bilevel file.
//		265	0109	CellLength			The length of the dithering or halftoning matrix used to create a dithered or halftoned bilevel file.
//		266	010A	FillOrder			The logical order of bits within a byte.
//		270	010E	ImageDescription	A string that describes the subject of the image.
//		271	010F	Make				The scanner manufacturer.
//		272	0110	Model				The scanner model name or number.
//		273	0111	StripOffsets		For each strip, the byte offset of that strip.
//		274	0112	Orientation			The orientation of the image with respect to the rows and columns.
//		277	0115	SamplesPerPixel		The number of components per pixel.
//		278	0116	RowsPerStrip		The number of rows per strip.
//		279	0117	StripByteCounts		For each strip, the number of bytes in the strip after compression.
//		280	0118	MinSampleValue		The minimum component value used.
//		281	0119	MaxSampleValue		The maximum component value used.
//		282	011A	XResolution			The number of pixels per ResolutionUnit in the ImageWidth direction.
//		283	011B	YResolution			The number of pixels per ResolutionUnit in the ImageLength direction.
//		284	011C	PlanarConfiguration	How the components of each pixel are stored.
//		288	0120	FreeOffsets			For each string of contiguous unused bytes in a TIFF file, the byte offset of the string.
//		289	0121	FreeByteCounts		For each string of contiguous unused bytes in a TIFF file, the number of bytes in the string.
//		290	0122	GrayResponseUnit	The precision of the information contained in the GrayResponseCurve.
//		291	0123	GrayResponseCurve	For grayscale data, the optical density of each possible pixel value.
//		296	0128	ResolutionUnit		The unit of measurement for XResolution and YResolution.
//		305	0131	Software			Name and version number of the software package(s) used to create the image.
//		306	0132	DateTime			Date and time of image creation.
//		315	013B	Artist				Person who created the image.
//		316	013C	HostComputer		The computer and / or operating system in use at the time of image creation.
//		320	0140	ColorMap			A color map for palette color images.
//		338	0152	ExtraSamples		Description of extra components.
//		33432 8298	Copyright			Copyright notice.


#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
#include <direct.h>
#endif

#include "CppUnitLite/TestHarness.h"
#include "../Common/UnitTest.h"			// unit test helpers
#include "MarkTypes.h"
#include "tiffio.h"

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	SetDataPath(argv[0]);

	TestResult tr;
	TestRegistry::runAllTests(tr);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}

//----------------------------------------------------------------------------
TEST(LibTiff, BigTIFF)
{
	// Test reading BigTIFF files

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "BigTIFF.tif");

		TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

		UInt32 width;
		UInt32 height;
		TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
		LONGS_EQUAL(64, width);
		LONGS_EQUAL(64, height);

		UInt16 samplesPerPixel = 0;
		int status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(3, samplesPerPixel);

		UInt16 bitsPerSample = 0;
		status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(8, bitsPerSample);

		UInt16 compression = 0;
		status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(COMPRESSION_NONE, compression);

		UInt16 photoMetric = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

		UInt16 planar = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

		// you should use the LIBTIFF library memory allocation functions to get space to store the image
		UInt32 npixels = width*height;
		UInt32* pRaster = (UInt32 *)_TIFFmalloc(npixels * sizeof(UInt32));
		CHECK(pRaster != nullptr);

		//[Note:Though it works under Win32, you should not use the C - "malloc/free" and the C++ - "new/delete" functions / operators to do the memory management for the reading buffer]

		// Now you are able to read the image from the TIFF file using the following function:
		int n = TIFFReadRGBAImage(pTiff, width, height, pRaster, 0);
		LONGS_EQUAL(1, n);

		int x = 0;
		int y = 0;
		int pixelIndex = y * width + x;
		UInt8 R = (UInt8)TIFFGetR(pRaster[pixelIndex]);
		UInt8 G = (UInt8)TIFFGetG(pRaster[pixelIndex]);
		UInt8 B = (UInt8)TIFFGetB(pRaster[pixelIndex]);
		UInt8 A = (UInt8)TIFFGetA(pRaster[pixelIndex]);
		LONGS_EQUAL(0, R);
		LONGS_EQUAL(0, G);
		LONGS_EQUAL(255, B);
		LONGS_EQUAL(255, A);

		x = 32;
		y = 32;
		pixelIndex = y * width + x;
		R = (UInt8)TIFFGetR(pRaster[pixelIndex]);
		G = (UInt8)TIFFGetG(pRaster[pixelIndex]);
		B = (UInt8)TIFFGetB(pRaster[pixelIndex]);
		A = (UInt8)TIFFGetA(pRaster[pixelIndex]);
		LONGS_EQUAL(0, R);
		LONGS_EQUAL(128, G);
		LONGS_EQUAL(0, B);
		LONGS_EQUAL(255, A);

		if (pRaster)
			_TIFFfree(pRaster);
		if (pTiff)
			TIFFClose(pTiff);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "BigTIFFLong.tif");

		TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

		UInt32 width;
		UInt32 height;
		TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
		LONGS_EQUAL(64, width);
		LONGS_EQUAL(64, height);

		UInt16 samplesPerPixel = 0;
		int status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(3, samplesPerPixel);

		UInt16 bitsPerSample = 0;
		status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(8, bitsPerSample);

		UInt16 compression = 0;
		status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(COMPRESSION_NONE, compression);

		UInt16 photoMetric = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

		UInt16 planar = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

		TIFFClose(pTiff);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "BigTIFFLong.tif");

		TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

		UInt32 width;
		UInt32 height;
		TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
		LONGS_EQUAL(64, width);
		LONGS_EQUAL(64, height);

		UInt16 samplesPerPixel = 0;
		int status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(3, samplesPerPixel);

		UInt16 bitsPerSample = 0;
		status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(8, bitsPerSample);

		UInt16 compression = 0;
		status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(COMPRESSION_NONE, compression);

		UInt16 photoMetric = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

		UInt16 planar = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

		TIFFClose(pTiff);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "BigTIFFLong8.tif");

		TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

		UInt32 width;
		UInt32 height;
		TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
		LONGS_EQUAL(64, width);
		LONGS_EQUAL(64, height);

		UInt16 samplesPerPixel = 0;
		int status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(3, samplesPerPixel);

		UInt16 bitsPerSample = 0;
		status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(8, bitsPerSample);

		UInt16 compression = 0;
		status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(COMPRESSION_NONE, compression);

		UInt16 photoMetric = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

		UInt16 planar = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

		TIFFClose(pTiff);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "BigTIFFMotorola.tif");

		TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

		UInt32 width;
		UInt32 height;
		TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
		LONGS_EQUAL(64, width);
		LONGS_EQUAL(64, height);

		UInt16 samplesPerPixel = 0;
		int status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(3, samplesPerPixel);

		UInt16 bitsPerSample = 0;
		status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(8, bitsPerSample);

		UInt16 compression = 0;
		status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(COMPRESSION_NONE, compression);

		UInt16 photoMetric = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

		UInt16 planar = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

		TIFFClose(pTiff);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "BigTIFFMotorolaLongStrips.tif");

		TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

		UInt32 width;
		UInt32 height;
		TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
		LONGS_EQUAL(64, width);
		LONGS_EQUAL(64, height);

		UInt16 samplesPerPixel = 0;
		int status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(3, samplesPerPixel);

		UInt16 bitsPerSample = 0;
		status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(8, bitsPerSample);

		UInt16 compression = 0;
		status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(COMPRESSION_NONE, compression);

		UInt16 photoMetric = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

		UInt16 planar = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

		TIFFClose(pTiff);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "BigTIFFSubIFD4.tif");

		TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

		UInt32 width;
		UInt32 height;
		TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
		LONGS_EQUAL(64, width);
		LONGS_EQUAL(64, height);

		UInt16 samplesPerPixel = 0;
		int status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(3, samplesPerPixel);

		UInt16 bitsPerSample = 0;
		status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(8, bitsPerSample);

		UInt16 compression = 0;
		status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(COMPRESSION_NONE, compression);

		UInt16 photoMetric = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

		UInt16 planar = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

		TIFFClose(pTiff);
	}

	{
		XString fileName = XString::CombinePath(gs_DataPath.c_str(), "BigTIFFSubIFD8.tif");

		TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

		UInt32 width;
		UInt32 height;
		TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
		LONGS_EQUAL(64, width);
		LONGS_EQUAL(64, height);

		UInt16 samplesPerPixel = 0;
		int status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(3, samplesPerPixel);

		UInt16 bitsPerSample = 0;
		status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(8, bitsPerSample);

		UInt16 compression = 0;
		status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(COMPRESSION_NONE, compression);

		UInt16 photoMetric = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

		UInt16 planar = 0;
		status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

		TIFFClose(pTiff);
	}
}

//----------------------------------------------------------------------------
TEST(LibTiff, WriteTest)
{
	// Basic test of creating & writing a tif file.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "write_test.tif");

	{
		// Create file.

		TIFF* pOutFile = TIFFOpen(fileName.c_str(), "w");

		UInt32 width = 100;
		UInt32 height = 50;
		int samplesPerPixel = 3;    // RGB

		UInt8* pImage = new UInt8[width*height*samplesPerPixel];

		// Then format of the pixel information is store in the order RGBA, into the array, each channel occupies 1 byte(char).

		// Now we need to set the tags in the new image file, and the essential ones are the following :

		TIFFSetField(pOutFile, TIFFTAG_IMAGEWIDTH, width);					// set the width of the image
		TIFFSetField(pOutFile, TIFFTAG_IMAGELENGTH, height);				// set the height of the image
		TIFFSetField(pOutFile, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);	// set number of channels per pixel
		TIFFSetField(pOutFile, TIFFTAG_BITSPERSAMPLE, 8);					// set the size of the channels
		TIFFSetField(pOutFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);   // set the origin of the image.
		TIFFSetField(pOutFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(pOutFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

		// We will use most basic image data storing method provided by the library to write the data into the file,
		// this method uses strips, and we are storing a line(row) of pixel at a time. This following code writes 
		// the data from the char array image into the file :

		tsize_t linebytes = samplesPerPixel * width;     // length in memory of one row of pixel in the image.

		// Allocate memory to store the pixels of current row
		UInt8* pScanLine = nullptr;
		if (TIFFScanlineSize(pOutFile) == linebytes)
			pScanLine = (unsigned char *)_TIFFmalloc(linebytes);
		else
			pScanLine = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(pOutFile));

		// We set the strip size of the file to be size of one row of pixels
		TIFFSetField(pOutFile, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(pOutFile, width*samplesPerPixel));

		// put some visually verifiable data in the file
		for (UInt32 row = 0; row < height; ++row)
		{
			for (UInt32 col = 0; col < width; ++col)
			{
				pImage[row * linebytes + col*samplesPerPixel] = 0;
				pImage[row * linebytes + col*samplesPerPixel + 1] = col % 256;
				pImage[row * linebytes + col*samplesPerPixel + 2] = 0;
			}
		}

		// Now writing image to the file one strip at a time
		for (UInt32 row = 0; row < height; ++row)
		{
			memcpy(pScanLine, &pImage[(height - row - 1)*linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
			if (TIFFWriteScanline(pOutFile, pScanLine, row, 0) < 0)
				break;
		}

		// Finally we close the output file, and destroy the buffer

		(void)TIFFClose(pOutFile);

		if (pScanLine)
			_TIFFfree(pScanLine);
		if (pImage)
			delete pImage;
	}

	{
		// Read it back

		TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

		UInt32 width = 0;
		UInt32 height = 0;
		int status = TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
		status = TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
		LONGS_EQUAL(100, width);
		LONGS_EQUAL(50, height);

		UInt16 samplesPerPixel = 0;
		status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(3, samplesPerPixel);

		UInt16 bitsPerSample = 0;
		status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		LONGS_EQUAL(1, status);
		LONGS_EQUAL(8, bitsPerSample);

		// you should use the LIBTIFF library memory allocation functions to get space to store the image
		UInt32 npixels = width*height;
		UInt32* pRaster = (UInt32 *)_TIFFmalloc(npixels * sizeof(UInt32));
		CHECK(pRaster != nullptr);

		// Now you are able to read the image from the TIFF file using the following function:
		status = TIFFReadRGBAImage(pTiff, width, height, pRaster, 0);
		LONGS_EQUAL(1, status);

		// What you have now is an array(raster) of pixel values, one for each pixel in the image, 
		// and each pixel is a 4 - byte value.Each of the bytes represent a channel of the pixel 
		// value(RGBA). Each channel is represented by an integer value from 0 to 255.

		// To get each of the individual channel of a pixel use the function:
		// Important: Remember that the origin of the raster is at the lower left corner.
		// You should be able to figure out the how the image is stored in the raster 
		// given that the pixel information is stored a row at a time!

		for (UInt32 row = 0; row < height; ++row)
		{
			for (UInt32 col = 0; col < width; ++col)
			{
				int pixelIndex = row * width + col;
				UInt8 R = (UInt8)TIFFGetR(pRaster[pixelIndex]);
				UInt8 G = (UInt8)TIFFGetG(pRaster[pixelIndex]);
				UInt8 B = (UInt8)TIFFGetB(pRaster[pixelIndex]);

				// green channel is same as column offset, rest are 0
				LONGS_EQUAL(0, R);
				LONGS_EQUAL(col, G);
				LONGS_EQUAL(0, B);
			}
		}

		// After you are finished with the raster you should destroy it and free up the memory
		_TIFFfree(pRaster);
		TIFFClose(pTiff);
	}
}

//----------------------------------------------------------------------------
TEST(LibTiff, fax2d)
{
	// fax2d.tif
	//		dimensions:		1728 x 1082
	//		compression:	CCITT T.4
	//		1-bit b&w (G3/2D) facsimile

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "fax2d.tif");

	TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

	UInt32 width;
	UInt32 height;
	TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
	LONGS_EQUAL(1728, width);
	LONGS_EQUAL(1082, height);

	UInt16 samplesPerPixel = 0;
	int status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(1, samplesPerPixel);

	UInt16 bitsPerSample = 0;
	status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(1, bitsPerSample);

	UInt16 compression = 0;
	status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(COMPRESSION_CCITTFAX3, compression);
	LONGS_EQUAL(COMPRESSION_CCITT_T4, compression);

	UInt16 photoMetric = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PHOTOMETRIC_MINISWHITE, photoMetric);

	UInt16 planar = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

	TIFFClose(pTiff);
}

//----------------------------------------------------------------------------
TEST(LibTiff, Jello)
{
	// jello.tif
	//		dimensions:		256x192
	//		compression:	PackBits
	//		8-bit RGB (lzw palette) Paul Heckbert "jello"

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jello.tif");

	TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

	int status = 0;
	UInt32 width;
	UInt32 height;
	TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
	LONGS_EQUAL(256, width);
	LONGS_EQUAL(192, height);

	// 1 "sample" per pixel = 1 palette index per pixel
	UInt16 samplesPerPixel = 0;
	status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(1, samplesPerPixel);

	UInt16 bitsPerSample = 0;
	status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(8, bitsPerSample);

	UInt16 compression = 0;
	status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(COMPRESSION_PACKBITS, compression);

	UInt16 photoMetric = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PHOTOMETRIC_PALETTE, photoMetric);

	UInt16 planar = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

	// you should use the LIBTIFF library memory allocation functions to get space to store the image
	UInt32 npixels = width*height;
	UInt32* pRaster = (UInt32 *)_TIFFmalloc(npixels * sizeof(UInt32));
	CHECK(pRaster != nullptr);

	//[Note:Though it works under Win32, you should not use the C - "malloc/free" and the C++ - "new/delete" functions / operators to do the memory management for the reading buffer]

	// Now you are able to read the image from the TIFF file using the following function:
	int n = TIFFReadRGBAImage(pTiff, width, height, pRaster, 0);
	LONGS_EQUAL(1, n);

	// What you have now is an array(raster) of pixel values, one for each pixel in the image, 
	// and each pixel is a 4 - byte value.Each of the bytes represent a channel of the pixel 
	// value(RGBA).Each channel is represented by an integer value from 0 to 255.

	// To get each of the individual channel of a pixel use the function:
	//Important: Remember that the origin of the raster is at the lower left corner.You should be able to figure out the how the image is stored in the raster given that the pixel information is stored a row at a time!

	UInt8 R = (UInt8)TIFFGetR(pRaster[0]);	// i is the index of the pixel in the raster.
	UInt8 G = (UInt8)TIFFGetG(pRaster[0]);
	UInt8 B = (UInt8)TIFFGetB(pRaster[0]);
	UInt8 A = (UInt8)TIFFGetA(pRaster[0]);
	LONGS_EQUAL(126, R);
	LONGS_EQUAL(94, G);
	LONGS_EQUAL(190, B);
	LONGS_EQUAL(255, A);

	R = (UInt8)TIFFGetR(pRaster[60]);
	G = (UInt8)TIFFGetG(pRaster[60]);
	B = (UInt8)TIFFGetB(pRaster[60]);
	A = (UInt8)TIFFGetA(pRaster[60]);
	LONGS_EQUAL(126, R);
	LONGS_EQUAL(94, G);
	LONGS_EQUAL(190, B);
	LONGS_EQUAL(255, A);

	R = (UInt8)TIFFGetR(pRaster[10304]);
	G = (UInt8)TIFFGetG(pRaster[10304]);
	B = (UInt8)TIFFGetB(pRaster[10304]);
	A = (UInt8)TIFFGetA(pRaster[10304]);
	LONGS_EQUAL(254, R);
	LONGS_EQUAL(0, G);
	LONGS_EQUAL(0, B);
	LONGS_EQUAL(255, A);

	// After you are finished with the raster you should destroy it and free up the memory
	_TIFFfree(pRaster);
	TIFFClose(pTiff);
}

//----------------------------------------------------------------------------
TEST(LibTiff, Oxford)
{
	// oxford.tif
	//		dimensions:		601 x 81
	//		compression:	LZW compression
	//		PlanarConfiguration = 2 (separated samples)
	//		8 - bit RGB(lzw) screendump off oxford


	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "oxford.tif");

	TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

	int status = 0;
	UInt32 width;
	UInt32 height;
	TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
	LONGS_EQUAL(601, width);
	LONGS_EQUAL(81, height);

	UInt16 samplesPerPixel = 0;
	status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(3, samplesPerPixel);

	UInt16 bitsPerSample = 0;
	status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(8, bitsPerSample);

	UInt16 compression = 0;
	status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(COMPRESSION_LZW, compression);

	UInt16 planar = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PLANARCONFIG_SEPARATE, planar);

	// you should use the LIBTIFF library memory allocation functions to get space to store the image
	UInt32 npixels = width*height;
	UInt32* pRaster = (UInt32 *)_TIFFmalloc(npixels * sizeof(UInt32));
	CHECK(pRaster != nullptr);

	UInt16 photoMetric = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

	//[Note:Though it works under Win32, you should not use the C - "malloc/free" and the C++ - "new/delete" functions / operators to do the memory management for the reading buffer]

	// Now you are able to read the image from the TIFF file using the following function:
	int n = TIFFReadRGBAImage(pTiff, width, height, pRaster, 0);
	LONGS_EQUAL(1, n);

	// What you have now is an array(raster) of pixel values, one for each pixel in the image, 
	// and each pixel is a 4 - byte value.Each of the bytes represent a channel of the pixel 
	// value(RGBA).Each channel is represented by an integer value from 0 to 255.

	// To get each of the individual channel of a pixel use the function:
	//Important: Remember that the origin of the raster is at the lower left corner.You should be able to figure out the how the image is stored in the raster given that the pixel information is stored a row at a time!

	UInt8 R = (UInt8)TIFFGetR(pRaster[0]);	// i is the index of the pixel in the raster.
	UInt8 G = (UInt8)TIFFGetG(pRaster[0]);
	UInt8 B = (UInt8)TIFFGetB(pRaster[0]);
	UInt8 A = (UInt8)TIFFGetA(pRaster[0]);
	LONGS_EQUAL(255, R);
	LONGS_EQUAL(255, G);
	LONGS_EQUAL(255, B);
	LONGS_EQUAL(255, A);

	R = (UInt8)TIFFGetR(pRaster[60]);
	G = (UInt8)TIFFGetG(pRaster[60]);
	B = (UInt8)TIFFGetB(pRaster[60]);
	A = (UInt8)TIFFGetA(pRaster[60]);
	LONGS_EQUAL(214, R);
	LONGS_EQUAL(214, G);
	LONGS_EQUAL(214, B);
	LONGS_EQUAL(255, A);

	R = (UInt8)TIFFGetR(pRaster[5000]);
	G = (UInt8)TIFFGetG(pRaster[5000]);
	B = (UInt8)TIFFGetB(pRaster[5000]);
	A = (UInt8)TIFFGetA(pRaster[5000]);
	LONGS_EQUAL(173, R);
	LONGS_EQUAL(173, G);
	LONGS_EQUAL(173, B);
	LONGS_EQUAL(255, A);

	// After you are finished with the raster you should destroy it and free up the memory
	_TIFFfree(pRaster);
	TIFFClose(pTiff);
}

//----------------------------------------------------------------------------
TEST(LibTiff, pc260001)
{
	// pc260001.tif
	//		640 x 480
	//		Uncompressed
	//		8 - bit RGB digital camera image.Contains EXIF SubIFD.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "pc260001.tif");

	TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

	UInt32 width;
	UInt32 height;
	TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
	LONGS_EQUAL(640, width);
	LONGS_EQUAL(480, height);

	UInt16 samplesPerPixel = 0;
	int status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(3, samplesPerPixel);

	UInt16 bitsPerSample = 0;
	status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(8, bitsPerSample);

	UInt16 compression = 0;
	status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(COMPRESSION_NONE, compression);

	UInt16 photoMetric = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

	UInt16 planar = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

	// you should use the LIBTIFF library memory allocation functions to get space to store the image
	UInt32 npixels = width*height;
	UInt32* pRaster = (UInt32 *)_TIFFmalloc(npixels * sizeof(UInt32));
	CHECK(pRaster != nullptr);

	// Now you are able to read the image from the TIFF file using the following function:
	int n = TIFFReadRGBAImage(pTiff, width, height, pRaster, 0);
	LONGS_EQUAL(1, n);

	// What you have now is an array(raster) of pixel values, one for each pixel in the image, 
	// and each pixel is a 4 - byte value.Each of the bytes represent a channel of the pixel 
	// value(RGBA).Each channel is represented by an integer value from 0 to 255.

	// To get each of the individual channel of a pixel use the function:
	//Important: Remember that the origin of the raster is at the lower left corner.You should be able to figure out the how the image is stored in the raster given that the pixel information is stored a row at a time!

	UInt8 R = (UInt8)TIFFGetR(pRaster[0]);	// i is the index of the pixel in the raster.
	UInt8 G = (UInt8)TIFFGetG(pRaster[0]);
	UInt8 B = (UInt8)TIFFGetB(pRaster[0]);
	UInt8 A = (UInt8)TIFFGetA(pRaster[0]);
	LONGS_EQUAL(128, R);
	LONGS_EQUAL(137, G);
	LONGS_EQUAL(142, B);
	LONGS_EQUAL(255, A);

	R = (UInt8)TIFFGetR(pRaster[64]);
	G = (UInt8)TIFFGetG(pRaster[64]);
	B = (UInt8)TIFFGetB(pRaster[64]);
	A = (UInt8)TIFFGetA(pRaster[64]);
	LONGS_EQUAL(108, R);
	LONGS_EQUAL(103, G);
	LONGS_EQUAL(97, B);
	LONGS_EQUAL(255, A);

	// test indexing to row,col - remember, 0,0 is lower left corner of a TIFF
	UInt32 row = 475;
	UInt32 col = 48;
	UInt32 index = width * (height - row - 1) + col;
	R = (UInt8)TIFFGetR(pRaster[index]);
	G = (UInt8)TIFFGetG(pRaster[index]);
	B = (UInt8)TIFFGetB(pRaster[index]);
	A = (UInt8)TIFFGetA(pRaster[index]);
	LONGS_EQUAL(68, R);
	LONGS_EQUAL(59, G);
	LONGS_EQUAL(52, B);
	LONGS_EQUAL(255, A);

	// After you are finished with the raster you should destroy it and free up the memory
	_TIFFfree(pRaster);
	TIFFClose(pTiff);
}

//----------------------------------------------------------------------------
TEST(LibTiff, Strike)
{
	// strike.tif
	//		dimensions:		256 x 200
	//		compression:	LZW
	//		bit depth:		32
	//		8-bit RGBA (lzw) "bowling pins" from Pixar

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "strike.tif");

	TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

	int status = 0;
	UInt32 width;
	UInt32 height;
	TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
	LONGS_EQUAL(256, width);
	LONGS_EQUAL(200, height);

	UInt16 samplesPerPixel = 0;
	status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(4, samplesPerPixel);

	UInt16 bitsPerSample = 0;
	status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(8, bitsPerSample);

	UInt16 planar = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

	// old jpeg compressoin
	UInt16 compression = 0;
	status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(COMPRESSION_LZW, compression);

	UInt16 photoMetric = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PHOTOMETRIC_RGB, photoMetric);

	TIFFClose(pTiff);
}

//----------------------------------------------------------------------------
TEST(LibTiff, zackthecat)
{
	// zackthecat.tif
	//		dimensions:		234 x 213
	//		compression:	JPEG
	//		bit depth:		24
	//		8-bit YCbCr (OLD jpeg) tiled "ZackTheCat" from NeXT**

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "zackthecat.tif");

	TIFF* pTiff = TIFFOpen(fileName.c_str(), "r");

	int status = 0;
	UInt32 width;
	UInt32 height;
	TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height);
	LONGS_EQUAL(234, width);
	LONGS_EQUAL(213, height);

	UInt16 samplesPerPixel = 0;
	status = TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(3, samplesPerPixel);

	UInt16 bitsPerSample = 0;
	status = TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(8, bitsPerSample);

	// old jpeg compressoin
	UInt16 compression = 0;
	status = TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(COMPRESSION_OJPEG, compression);

	UInt16 photoMetric = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PHOTOMETRIC, &photoMetric);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PHOTOMETRIC_YCBCR, photoMetric);

	UInt16 planar = 0;
	status = TIFFGetField(pTiff, TIFFTAG_PLANARCONFIG, &planar);
	LONGS_EQUAL(1, status);
	LONGS_EQUAL(PLANARCONFIG_CONTIG, planar);

	TIFFClose(pTiff);
}

//----------------------------------------------------------------------------
TEST(LibTiff, DataSize)
{
	CHECK(sizeof(UInt8) == 1);
	CHECK(sizeof(UInt16) == 2);		// 16 bits - unsigned
	CHECK(sizeof(UInt32) == 4);		// 32 bits - unsigned
	CHECK(sizeof(UInt64) == 8);		// 64 bits

	CHECK(sizeof(Int8) == 1);			// 8 bits - signed
	CHECK(sizeof(Int16) == 2);		// 16 bits - signed
	CHECK(sizeof(Int32) == 4);		// 32 bits - signed
	CHECK(sizeof(Int64) == 8);		// 64 bits - signed
}
