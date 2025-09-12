// TestTiffFile.cpp
// Test harness for TiffFile class.
//
// In the baseline TIFF spec, there are only 4 image types:
//		- bilevel
//		- grayscale
//		- palette-color
//		- full-color images
//
// Make sure there is at least one test for each of these 4 image types.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
#include <direct.h>
#endif

#include "CppUnitLite/TestHarness.h"
#include "../Common/UnitTest.h"			// unit test helpers

#include "TiffFile.h"			// interface to class under test

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
TEST(TiffFile, BaseLine_BiLevel)
{
	// circuit_bw.tif

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "circuit_bw.tif");

	TiffFile* pTiffFile = new TiffFile(fileName.c_str());

	LONGS_EQUAL(272, pTiffFile->GetWidth());
	LONGS_EQUAL(280, pTiffFile->GetHeight());

	LONGS_EQUAL(1, pTiffFile->GetSamplesPerPixel());
	LONGS_EQUAL(1, pTiffFile->GetBitsPerSample());

	LONGS_EQUAL(COMPRESSION_CCITTRLE, pTiffFile->GetCompression());
	LONGS_EQUAL(PHOTOMETRIC_MINISWHITE, pTiffFile->GetPhotometric());
	LONGS_EQUAL(PLANARCONFIG_CONTIG, pTiffFile->GetPlanarConfig());

	DOUBLES_EQUAL(72.0, pTiffFile->GetXResolution(), 0.0);
	DOUBLES_EQUAL(72.0, pTiffFile->GetYResolution(), 0.0);

	PixelType pix = pTiffFile->GetColor(0, 0);
	LONGS_EQUAL(255, pix.R);
	LONGS_EQUAL(255, pix.G);
	LONGS_EQUAL(255, pix.B);
	LONGS_EQUAL(255, pix.A);

	pix = pTiffFile->GetColor(60, 0);
	LONGS_EQUAL(255, pix.R);
	LONGS_EQUAL(255, pix.G);
	LONGS_EQUAL(255, pix.B);
	LONGS_EQUAL(255, pix.A);

	pix = pTiffFile->GetColor(64, 40);
	LONGS_EQUAL(0, pix.R);
	LONGS_EQUAL(0, pix.G);
	LONGS_EQUAL(0, pix.B);
	LONGS_EQUAL(255, pix.A);

	delete pTiffFile;
}

//----------------------------------------------------------------------------
TEST(TiffFile, BaseLine_GrayScale)
{
	// Test reading a gray scale TIFF file.
	//
	// cell.tif

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "cell.tif");

	TiffFile* pTiffFile = new TiffFile(fileName.c_str());

	LONGS_EQUAL(191, pTiffFile->GetWidth());
	LONGS_EQUAL(159, pTiffFile->GetHeight());

	LONGS_EQUAL(1, pTiffFile->GetSamplesPerPixel());
	LONGS_EQUAL(8, pTiffFile->GetBitsPerSample());

	LONGS_EQUAL(COMPRESSION_PACKBITS, pTiffFile->GetCompression());
	LONGS_EQUAL(PHOTOMETRIC_MINISBLACK, pTiffFile->GetPhotometric());
	LONGS_EQUAL(PLANARCONFIG_CONTIG, pTiffFile->GetPlanarConfig());

	DOUBLES_EQUAL(72.0, pTiffFile->GetXResolution(), 0.0);
	DOUBLES_EQUAL(72.0, pTiffFile->GetYResolution(), 0.0);

	PixelType pix = pTiffFile->GetColor(0, 0);
	LONGS_EQUAL(126, pix.R);
	LONGS_EQUAL(126, pix.G);
	LONGS_EQUAL(126, pix.B);
	LONGS_EQUAL(255, pix.A);

	pix = pTiffFile->GetColor(60, 0);
	LONGS_EQUAL(124, pix.R);
	LONGS_EQUAL(124, pix.G);
	LONGS_EQUAL(124, pix.B);
	LONGS_EQUAL(255, pix.A);

	pix = pTiffFile->GetColor(64, 40);
	LONGS_EQUAL(119, pix.R);
	LONGS_EQUAL(119, pix.G);
	LONGS_EQUAL(119, pix.B);
	LONGS_EQUAL(255, pix.A);

	delete pTiffFile;
}

//----------------------------------------------------------------------------
TEST(TiffFile, BaseLine_Palette)
{
	// jello.tif
	//		dimensions:		256x192
	//		compression:	PackBits
	//		8-bit RGB (lzw palette) Paul Heckbert "jello"

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "jello.tif");

	TiffFile* pTiffFile = new TiffFile(fileName.c_str());

	LONGS_EQUAL(256, pTiffFile->GetWidth());
	LONGS_EQUAL(192, pTiffFile->GetHeight());

	LONGS_EQUAL(1, pTiffFile->GetSamplesPerPixel());
	LONGS_EQUAL(8, pTiffFile->GetBitsPerSample());

	LONGS_EQUAL(COMPRESSION_PACKBITS, pTiffFile->GetCompression());
	LONGS_EQUAL(PHOTOMETRIC_PALETTE, pTiffFile->GetPhotometric());
	LONGS_EQUAL(PLANARCONFIG_CONTIG, pTiffFile->GetPlanarConfig());

	DOUBLES_EQUAL(0.0, pTiffFile->GetXResolution(), 0.0);
	DOUBLES_EQUAL(0.0, pTiffFile->GetYResolution(), 0.0);

	PixelType pix = pTiffFile->GetColor(0, 0);
	LONGS_EQUAL(126, pix.R);
	LONGS_EQUAL(94, pix.G);
	LONGS_EQUAL(190, pix.B);
	LONGS_EQUAL(255, pix.A);

	pix = pTiffFile->GetColor(60, 0);
	LONGS_EQUAL(126, pix.R);
	LONGS_EQUAL(94, pix.G);
	LONGS_EQUAL(190, pix.B);
	LONGS_EQUAL(255, pix.A);

	pix = pTiffFile->GetColor(64, 40);
	LONGS_EQUAL(254, pix.R);
	LONGS_EQUAL(0, pix.G);
	LONGS_EQUAL(0, pix.B);
	LONGS_EQUAL(255, pix.A);

	delete pTiffFile;
}

//----------------------------------------------------------------------------
TEST(TiffFile, BaseLine_FullColor)
{
	// quad-lzw.tif

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "quad-lzw.tif");

	TiffFile* pTiffFile = new TiffFile(fileName.c_str());

	LONGS_EQUAL(512, pTiffFile->GetWidth());
	LONGS_EQUAL(384, pTiffFile->GetHeight());

	LONGS_EQUAL(3, pTiffFile->GetSamplesPerPixel());
	LONGS_EQUAL(8, pTiffFile->GetBitsPerSample());

	LONGS_EQUAL(COMPRESSION_LZW, pTiffFile->GetCompression());
	LONGS_EQUAL(PHOTOMETRIC_RGB, pTiffFile->GetPhotometric());
	LONGS_EQUAL(PLANARCONFIG_CONTIG, pTiffFile->GetPlanarConfig());

	DOUBLES_EQUAL(0.0, pTiffFile->GetXResolution(), 0.0);
	DOUBLES_EQUAL(0.0, pTiffFile->GetYResolution(), 0.0);

	PixelType pix = pTiffFile->GetColor(0, 0);
	LONGS_EQUAL(0, pix.R);
	LONGS_EQUAL(0, pix.G);
	LONGS_EQUAL(0, pix.B);
	LONGS_EQUAL(255, pix.A);

	pix = pTiffFile->GetColor(256, 192);
	LONGS_EQUAL(125, pix.R);
	LONGS_EQUAL(231, pix.G);
	LONGS_EQUAL(199, pix.B);
	LONGS_EQUAL(255, pix.A);

	pix = pTiffFile->GetColor(64, 40);
	LONGS_EQUAL(0, pix.R);
	LONGS_EQUAL(0, pix.G);
	LONGS_EQUAL(0, pix.B);
	LONGS_EQUAL(255, pix.A);

	delete pTiffFile;
}

//----------------------------------------------------------------------------
TEST(TiffFile, BasicWrite)
{
	// Basic test of creating a tiff file.

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "write_test.tif");

	TiffFile* pTiffFile = new TiffFile();
	bool createStatus = pTiffFile->Create(200, 100);
	CHECK(createStatus == true);

	UInt32 width = pTiffFile->GetWidth();
	UInt32 height = pTiffFile->GetHeight();

	for (UInt32 y = 0; y< height; ++y)
	{
		for (UInt32 x = 0; x < width; ++x)
		{
			PixelType pix;
			pix.R = 0;
			pix.G = 0;
			pix.B = x % 256;
			pTiffFile->SetColor(x, y, pix);
		}
	}

	pTiffFile->Save(fileName.c_str());

	delete pTiffFile;
}

//----------------------------------------------------------------------------
TEST(TiffFile, BasicRead)
{
	// oxford.tif
	//		601x81
	//		LZW compression

	XString fileName = XString::CombinePath(gs_DataPath.c_str(), "oxford.tif");

	TiffFile* pTiffFile = new TiffFile(fileName.c_str());

	LONGS_EQUAL(601, pTiffFile->GetWidth());
	LONGS_EQUAL(81, pTiffFile->GetHeight());
	LONGS_EQUAL(3, pTiffFile->GetSamplesPerPixel());
	LONGS_EQUAL(8, pTiffFile->GetBitsPerSample());

	DOUBLES_EQUAL(0.0, pTiffFile->GetXResolution(), 0.0);
	DOUBLES_EQUAL(0.0, pTiffFile->GetYResolution(), 0.0);

	PixelType pix = pTiffFile->GetColor(0, 0);
	LONGS_EQUAL(255, pix.R);
	LONGS_EQUAL(255, pix.G);
	LONGS_EQUAL(255, pix.B);
	LONGS_EQUAL(255, pix.A);

	pix = pTiffFile->GetColor(60, 0);
	LONGS_EQUAL(214, pix.R);
	LONGS_EQUAL(214, pix.G);
	LONGS_EQUAL(214, pix.B);
	LONGS_EQUAL(255, pix.A);

	pix = pTiffFile->GetColor(191, 7);
	LONGS_EQUAL(173, pix.R);
	LONGS_EQUAL(173, pix.G);
	LONGS_EQUAL(173, pix.B);
	LONGS_EQUAL(255, pix.A);

	delete pTiffFile;
}

//----------------------------------------------------------------------------
TEST(TiffFile, DataSize)
{
	CHECK(sizeof(UInt8) == 1);
	CHECK(sizeof(UInt16) == 2);		// 16 bits - unsigned
	CHECK(sizeof(UInt32) == 4);		// 32 bits - unsigned
	CHECK(sizeof(UInt64) == 8);		// 64 bits

	CHECK(sizeof(Int8) == 1);		// 8 bits - signed
	CHECK(sizeof(Int16) == 2);		// 16 bits - signed
	CHECK(sizeof(Int32) == 4);		// 32 bits - signed
	CHECK(sizeof(Int64) == 8);		// 64 bits - signed
}
