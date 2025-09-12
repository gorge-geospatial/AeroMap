// TestPngFile.cpp
// CppUnitLite test harness for PngFile class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers
#include "PngFile.h"					// interface to class under test

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
TEST(PngFile, BasicWrite)
{
	XString inFileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "toucan.png");
	XString outFileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "test-out.png");

	{
		PngFile pngFile(inFileNamePNG.c_str());

		LONGS_EQUAL(162, pngFile.GetWidth());
		LONGS_EQUAL(150, pngFile.GetHeight());

		int status = pngFile.Save(outFileNamePNG.c_str());
		CHECK(status == 0);
	}

	{
		// read back
		PngFile pngFile(outFileNamePNG.c_str());
		LONGS_EQUAL(162, pngFile.GetWidth());
		LONGS_EQUAL(150, pngFile.GetHeight());
		LONGS_EQUAL(8, pngFile.GetBitDepth());

		PixelType pix;
		for (int i = 0; i < 40; ++i)
		{
			pix = pngFile.GetPixel(i, i);
			LONGS_EQUAL(0, pix.R); LONGS_EQUAL(0, pix.G); LONGS_EQUAL(0, pix.B); LONGS_EQUAL(255, pix.A);
		}
		pix = pngFile.GetPixel(41, 41);
		LONGS_EQUAL(0, pix.R); LONGS_EQUAL(0, pix.G); LONGS_EQUAL(0, pix.B); LONGS_EQUAL(255, pix.A);
		pix = pngFile.GetPixel(42, 42);
		LONGS_EQUAL(61, pix.R); LONGS_EQUAL(21, pix.G); LONGS_EQUAL(21, pix.B); LONGS_EQUAL(255, pix.A);
		pix = pngFile.GetPixel(43, 43);
		LONGS_EQUAL(200, pix.R); LONGS_EQUAL(126, pix.G); LONGS_EQUAL(28, pix.B); LONGS_EQUAL(255, pix.A);
		pix = pngFile.GetPixel(44, 44);
		LONGS_EQUAL(251, pix.R); LONGS_EQUAL(203, pix.G); LONGS_EQUAL(28, pix.B); LONGS_EQUAL(255, pix.A);
		pix = pngFile.GetPixel(45, 45);
		LONGS_EQUAL(252, pix.R); LONGS_EQUAL(210, pix.G); LONGS_EQUAL(28, pix.B); LONGS_EQUAL(255, pix.A);
		pix = pngFile.GetPixel(46, 46);
		LONGS_EQUAL(254, pix.R); LONGS_EQUAL(225, pix.G); LONGS_EQUAL(34, pix.B); LONGS_EQUAL(255, pix.A);
		pix = pngFile.GetPixel(47, 47);
		LONGS_EQUAL(255, pix.R); LONGS_EQUAL(235, pix.G); LONGS_EQUAL(50, pix.B); LONGS_EQUAL(255, pix.A);
		pix = pngFile.GetPixel(48, 48);
		LONGS_EQUAL(254, pix.R); LONGS_EQUAL(230, pix.G); LONGS_EQUAL(34, pix.B); LONGS_EQUAL(255, pix.A);
		pix = pngFile.GetPixel(49, 49);
		LONGS_EQUAL(255, pix.R); LONGS_EQUAL(235, pix.G); LONGS_EQUAL(50, pix.B); LONGS_EQUAL(255, pix.A);
		pix = pngFile.GetPixel(50, 50);
		LONGS_EQUAL(252, pix.R); LONGS_EQUAL(228, pix.G); LONGS_EQUAL(34, pix.B); LONGS_EQUAL(255, pix.A);
	}
}

//----------------------------------------------------------------------------
TEST(PngFile, BasicRead)
{
	XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "toucan.png");

	{
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 162);
		CHECK(pngFile.GetHeight() == 150);
	}
}

//----------------------------------------------------------------------------
TEST(PngFile, Read_gray_1)
{
	// the "-1" indicates bit depth of 1

	{
		PixelType pix;

		// width = 2, height = 1, bit depth = 1
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-1.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 2);
		CHECK(pngFile.GetHeight() == 1);
		CHECK(pngFile.GetBitDepth() == 1);

		// just 1 black pixel and 1 white pixel
		pix = pngFile.GetPixel(0, 0);
		CHECK(pix.R == 0);
		CHECK(pix.G == 0);
		CHECK(pix.B == 0);

		pix = pngFile.GetPixel(1, 0);
		CHECK(pix.R == 255);
		CHECK(pix.G == 255);
		CHECK(pix.B == 255);
	}

	{
		PixelType pix;

		// width = 2, height = 1, bit depth = 1
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-1-1.8.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 2);
		CHECK(pngFile.GetHeight() == 1);
		CHECK(pngFile.GetBitDepth() == 1);

		// just 1 black pixel and 1 white pixel
		pix = pngFile.GetPixel(0, 0);
		CHECK(pix.R == 0);
		CHECK(pix.G == 0);
		CHECK(pix.B == 0);

		pix = pngFile.GetPixel(1, 0);
		CHECK(pix.R == 255);
		CHECK(pix.G == 255);
		CHECK(pix.B == 255);
	}

	{
		PixelType pix;

		// width = 2, height = 1, bit depth = 1
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-1-1.8-tRNS.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 2);
		CHECK(pngFile.GetHeight() == 1);
		CHECK(pngFile.GetBitDepth() == 1);

		// just 1 black pixel and 1 white pixel
		pix = pngFile.GetPixel(0, 0);
		CHECK(pix.R == 0);
		CHECK(pix.G == 0);
		CHECK(pix.B == 0);

		pix = pngFile.GetPixel(1, 0);
		CHECK(pix.R == 255);
		CHECK(pix.G == 255);
		CHECK(pix.B == 255);
	}

	{
		PixelType pix;

		// width = 2, height = 1, bit depth = 1
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-1-linear-tRNS.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 2);
		CHECK(pngFile.GetHeight() == 1);
		CHECK(pngFile.GetBitDepth() == 1);

		// just 1 black pixel and 1 white pixel
		pix = pngFile.GetPixel(0, 0);
		CHECK(pix.R == 0);
		CHECK(pix.G == 0);
		CHECK(pix.B == 0);

		pix = pngFile.GetPixel(1, 0);
		CHECK(pix.R == 255);
		CHECK(pix.G == 255);
		CHECK(pix.B == 255);
	}

	{
		PixelType pix;

		// width = 2, height = 1, bit depth = 1
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-1-sRGB.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 2);
		CHECK(pngFile.GetHeight() == 1);
		CHECK(pngFile.GetBitDepth() == 1);

		// just 1 black pixel and 1 white pixel
		pix = pngFile.GetPixel(0, 0);
		CHECK(pix.R == 0);
		CHECK(pix.G == 0);
		CHECK(pix.B == 0);

		pix = pngFile.GetPixel(1, 0);
		CHECK(pix.R == 255);
		CHECK(pix.G == 255);
		CHECK(pix.B == 255);
	}

	{
		PixelType pix;

		// width = 2, height = 1, bit depth = 1
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-1-sRGB-tRNS.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 2);
		CHECK(pngFile.GetHeight() == 1);
		CHECK(pngFile.GetBitDepth() == 1);

		// just 1 black pixel and 1 white pixel
		pix = pngFile.GetPixel(0, 0);
		CHECK(pix.R == 0);
		CHECK(pix.G == 0);
		CHECK(pix.B == 0);

		pix = pngFile.GetPixel(1, 0);
		CHECK(pix.R == 255);
		CHECK(pix.G == 255);
		CHECK(pix.B == 255);
	}

	{
		PixelType pix;

		// width = 2, height = 1, bit depth = 1
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-1-tRNS.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 2);
		CHECK(pngFile.GetHeight() == 1);
		CHECK(pngFile.GetBitDepth() == 1);

		// just 1 black pixel and 1 white pixel
		pix = pngFile.GetPixel(0, 0);
		CHECK(pix.R == 0);
		CHECK(pix.G == 0);
		CHECK(pix.B == 0);

		pix = pngFile.GetPixel(1, 0);
		CHECK(pix.R == 255);
		CHECK(pix.G == 255);
		CHECK(pix.B == 255);
	}
}

//----------------------------------------------------------------------------
TEST(PngFile, Read_gray_2)
{
	// the "-2" indicates bit depth of 2

	{
		PixelType pix;

		// width = 4, height = 1, bit depth = 2
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-2.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 4);
		CHECK(pngFile.GetHeight() == 1);
		CHECK(pngFile.GetBitDepth() == 2);

		pix = pngFile.GetPixel(0, 0);
		CHECK(pix.R == 0);
		CHECK(pix.G == 0);
		CHECK(pix.B == 0);

		pix = pngFile.GetPixel(1, 0);
		CHECK(pix.R == 85);
		CHECK(pix.G == 85);
		CHECK(pix.B == 85);

		pix = pngFile.GetPixel(2, 0);
		CHECK(pix.R == 170);
		CHECK(pix.G == 170);
		CHECK(pix.B == 170);

		pix = pngFile.GetPixel(3, 0);
		CHECK(pix.R == 255);
		CHECK(pix.G == 255);
		CHECK(pix.B == 255);
	}
}

//----------------------------------------------------------------------------
TEST(PngFile, Read_gray_4)
{
	// the "-4" indicates bit depth of 4

	{
		PixelType pix;

		// width = 16, height = 1, bit depth = 4
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-4.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 16);
		CHECK(pngFile.GetHeight() == 1);
		CHECK(pngFile.GetBitDepth() == 4);

		pix = pngFile.GetPixel(0, 0);
		CHECK((pix.R == 0) && (pix.G == 0) && (pix.B == 0));

		pix = pngFile.GetPixel(1, 0);
		CHECK((pix.R == 17) && (pix.G == 17) && (pix.B == 17));

		pix = pngFile.GetPixel(2, 0);
		CHECK((pix.R == 34) && (pix.G == 34) && (pix.B == 34));

		pix = pngFile.GetPixel(3, 0);
		CHECK((pix.R == 51) && (pix.G == 51) && (pix.B == 51));

		pix = pngFile.GetPixel(4, 0);
		CHECK((pix.R == 68) && (pix.G == 68) && (pix.B == 68));

		pix = pngFile.GetPixel(5, 0);
		CHECK((pix.R == 85) && (pix.G == 85) && (pix.B == 85));

		pix = pngFile.GetPixel(6, 0);
		CHECK((pix.R == 102) && (pix.G == 102) && (pix.B == 102));

		pix = pngFile.GetPixel(7, 0);
		CHECK((pix.R == 119) && (pix.G == 119) && (pix.B == 119));

		pix = pngFile.GetPixel(8, 0);
		CHECK((pix.R == 136) && (pix.G == 136) && (pix.B == 136));

		pix = pngFile.GetPixel(9, 0);
		CHECK((pix.R == 153) && (pix.G == 153) && (pix.B == 153));

		pix = pngFile.GetPixel(10, 0);
		CHECK((pix.R == 170) && (pix.G == 170) && (pix.B == 170));

		pix = pngFile.GetPixel(11, 0);
		CHECK((pix.R == 187) && (pix.G == 187) && (pix.B == 187));

		pix = pngFile.GetPixel(12, 0);
		CHECK((pix.R == 204) && (pix.G == 204) && (pix.B == 204));

		pix = pngFile.GetPixel(13, 0);
		CHECK((pix.R == 221) && (pix.G == 221) && (pix.B == 221));

		pix = pngFile.GetPixel(14, 0);
		CHECK((pix.R == 238) && (pix.G == 238) && (pix.B == 238));

		pix = pngFile.GetPixel(15, 0);
		CHECK((pix.R == 255) && (pix.G == 255) && (pix.B == 255));
	}
}

//----------------------------------------------------------------------------
TEST(PngFile, Read_gray_8)
{
	// the "-8" indicates bit depth of 8

	{
		PixelType pix;

		// width = 256, height = 1, bit depth = 8
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-8.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 256);
		CHECK(pngFile.GetHeight() == 1);
		CHECK(pngFile.GetBitDepth() == 8);

		pix = pngFile.GetPixel(0, 0);
		CHECK((pix.R == 1) && (pix.G == 1) && (pix.B == 1));

		pix = pngFile.GetPixel(254, 0);
		CHECK((pix.R == 255) && (pix.G == 255) && (pix.B == 255));

		pix = pngFile.GetPixel(255, 0);
		CHECK((pix.R == 0) && (pix.G == 0) && (pix.B == 0));
	}
}

//----------------------------------------------------------------------------
TEST(PngFile, Read_gray_16)
{
	// the "-16" indicates bit depth of 16

	{
		PixelType pix;

		// width = 256, height = 1, bit depth = 16
		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "gray-16.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 256);
		CHECK(pngFile.GetHeight() == 256);
		CHECK(pngFile.GetBitDepth() == 16);

		for (int x = 0; x < 256; ++x)
		{
			for (int y = 0; y < 256; ++y)
			{
				pix = pngFile.GetPixel(x, 0);
				CHECK((pix.R == x) && (pix.G == x) && (pix.B == x));
			}
		}
	}
}

//----------------------------------------------------------------------------
TEST(PngFile, Read_rgb_8)
{
	{
		PixelType pix;

		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "rgb-8.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 256);
		CHECK(pngFile.GetHeight() == 256);
		CHECK(pngFile.GetBitDepth() == 8);

		// top scan line
		for (UInt32 x = 0; x < pngFile.GetWidth(); ++x)
		{
			pix = pngFile.GetPixel(x, 0);
			CHECK(pix.R == x);
			CHECK(pix.G == x);
			CHECK(pix.B == x);
		}
	}
}

//----------------------------------------------------------------------------
TEST(PngFile, Read_rgb_16)
{
	{
		PixelType pix;

		XString fileNamePNG = XString::CombinePath(gs_DataPath.c_str(), "rgb-16.png");
		PngFile pngFile(fileNamePNG.c_str());

		CHECK(pngFile.GetWidth() == 256);
		CHECK(pngFile.GetHeight() == 256);
		CHECK(pngFile.GetBitDepth() == 16);

		// top scan line
		for (UInt32 x = 0; x < pngFile.GetWidth(); ++x)
		{
			pix = pngFile.GetPixel(x, 0);
			CHECK(pix.R == x);
			CHECK(pix.G == x);
			CHECK(pix.B == x);
		}
	}
}
