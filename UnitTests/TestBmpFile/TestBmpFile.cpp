//	TestBmpFile.cpp
//
//	CppUnitLite test harness for BmpFile class
//
//	idea behind the 2x2, 3x3, 4x4, 5x5 tests is to exercise all "padding" values at the end of each pixel row
//
//	Example of a 2x2 Pixel Bitmap, with 24 bits/pixel encoding:
//
//	Offset 	Size 	Hex 			Value 	Description
//	-------------------------------------------------------------------------------------------
//	0 		2 		42 4D 			"BM" 		Magic Number (unsigned integer 66, 77)
//	2 		4 		46 00 00 00 	70 Bytes 	Size of Bitmap
//	6 		2 		00 00 			Unused 		Application Specific
//	8 		2 		00 00 			Unused 		Application Specific
//	10 		4 		36 00 00 00 	54 bytes 	The offset where the bitmap data (pixels) can be found
//	14 		4 		28 00 00 00 	40 bytes 	The number of bytes in the header (from this point)
//	18	 	4 		02 00 00 00 	2 pixels 	The width of the bitmap in pixels
//	22	 	4 		02 00 00 00 	2 pixels 	The height of the bitmap in pixels
//	26	 	2 		01 00 			1 plane 	Number of color planes being used
//	28	 	2 		18 00 			24 bits 	The number of bits/pixel.
//	30	 	4 		00 00 00 00 	0 			BI_RGB, No compression used
//	34	 	4 		10 00 00 00 	16 bytes 	The size of the raw BMP data (after this header)
//	38 		4 		13 0B 00 00 	2,835 pixels/meter 	The horizontal resolution of the image
//	42	 	4 		13 0B 00 00 	2,835 pixels/meter 	The vertical resolution of the image
//	46	 	4 		00 00 00 00 	0 colors 	Number of colors in the palette
//	50	 	4 		00 00 00 00 	0 important colors 	Means all colors are important
//	Start of Bitmap Data
//	54	 	3 		00 00 FF 		Red pixel
//	57	 	3 		FF FF FF 		White pixel
//	60	 	2 		00 00  			Padding for 4 bytes/row (Could be a value other than zero)
//	62	 	3 		FF 00 00 		Blue pixel
//	65	 	3 		00 FF 00 		Green pixel
//	68	 	2 		00 00  			Padding for 4 bytes/row (Could be a value other than zero)
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#include "CppUnitLite/TestHarness.h"
#include "../Common/UnitTest.h"			// unit test helpers
#include "BmpFile.h"					// interface to class under test

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	SetDataPath(argv[0]);
	_mkdir(gs_DataPath.c_str());

	TestResult tr;
	TestRegistry::runAllTests(tr);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}

//----------------------------------------------------------------------------
TEST(BmpFile, FileIO_2x2)
{
	// test access to object on disk - padding should be 2 bytes (rows must be multiple of 32 bits)
	//

	XString fileName = XString::CombinePath(gs_DataPath, "FileIO2x2.bmp");
	const int rowCount = 2;
	const int colCount = 2;

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Create(colCount, rowCount));

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				bmp.SetColor(row, col, 'A', 'B', 'C');
			}
		}

		CHECK(bmp.GetHeight() == rowCount);
		CHECK(bmp.GetWidth() == colCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}

		CHECK(bmp.Save(fileName.c_str()) == true);
	}

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Load(fileName.c_str()) == true);

		CHECK(bmp.GetWidth() == colCount);
		CHECK(bmp.GetHeight() == rowCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}
	}

}

//----------------------------------------------------------------------------
TEST(BmpFile, FileIO_3x3)
{
	// test access to object on disk - padding should be 1 byte (rows must be multiple of 32 bits)
	//

	XString fileName = XString::CombinePath(gs_DataPath, "FileIO3x3.bmp");
	const int rowCount = 3;
	const int colCount = 3;

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Create(colCount, rowCount));

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				bmp.SetColor(row, col, 'A', 'B', 'C');
			}
		}

		CHECK(bmp.GetHeight() == rowCount);
		CHECK(bmp.GetWidth() == colCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}

		CHECK(bmp.Save(fileName.c_str()) == true);
	}

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Load(fileName.c_str()) == true);

		CHECK(bmp.GetWidth() == colCount);
		CHECK(bmp.GetHeight() == rowCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}
	}

}

//----------------------------------------------------------------------------
TEST(BmpFile, FileIO_4x4)
{
	// test access to object on disk - padding should be 0 bytes (rows must be multiple of 32 bits)
	//

	XString fileName = XString::CombinePath(gs_DataPath, "FileIO4x4.bmp");
	const int rowCount = 4;
	const int colCount = 4;

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Create(colCount, rowCount));

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				bmp.SetColor(row, col, 'A', 'B', 'C');
			}
		}

		CHECK(bmp.GetHeight() == rowCount);
		CHECK(bmp.GetWidth() == colCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}

		CHECK(bmp.Save(fileName.c_str()) == true);
	}

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Load(fileName.c_str()) == true);

		CHECK(bmp.GetWidth() == colCount);
		CHECK(bmp.GetHeight() == rowCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}
	}

}

//----------------------------------------------------------------------------
TEST(BmpFile, FileIO_5x5)
{
	// test access to object on disk - padding should be 3 bytes (rows must be multiple of 32 bits)
	//

	XString fileName = XString::CombinePath(gs_DataPath, "FileIO5x5.bmp");
	const int rowCount = 5;
	const int colCount = 5;

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Create(colCount, rowCount));

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				bmp.SetColor(row, col, 'A', 'B', 'C');
			}
		}

		CHECK(bmp.GetHeight() == rowCount);
		CHECK(bmp.GetWidth() == colCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}

		CHECK(bmp.Save(fileName.c_str()) == true);
	}

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Load(fileName.c_str()) == true);

		CHECK(bmp.GetWidth() == colCount);
		CHECK(bmp.GetHeight() == rowCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}
	}

}

//----------------------------------------------------------------------------
TEST(BmpFile, FileIO_256)
{
	// test access to object on disk - relatively large bitmap
	//

	XString fileName = XString::CombinePath(gs_DataPath, "FileIO_256.bmp");
	const int rowCount = 256;
	const int colCount = 256;

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Create(colCount, rowCount));

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				bmp.SetColor(row, col, 'A', 'B', 'C');
			}
		}

		CHECK(bmp.GetHeight() == rowCount);
		CHECK(bmp.GetWidth() == colCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}

		CHECK(bmp.Save(fileName.c_str()) == true);
	}

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Load(fileName.c_str()) == true);

		CHECK(bmp.GetWidth() == colCount);
		CHECK(bmp.GetHeight() == rowCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}
	}

}

//----------------------------------------------------------------------------
TEST(BmpFile, Memory)
{
	// 	test access to object in memory
	//
	{
		// static allocation

		const int rowCount = 10;
		const int colCount = 10;
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Create(colCount, rowCount));

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				bmp.SetColor(row, col, 'A', 'B', 'C');
			}
		}

		CHECK(bmp.GetHeight() == rowCount);
		CHECK(bmp.GetWidth() == colCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}
	}

	{
		// dynamic allocation

		const int rowCount = 10;
		const int colCount = 10;
		int row = 0;
		int col = 0;

		BmpFile* bmp = new BmpFile();
		CHECK(bmp->Create(colCount, rowCount));

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				bmp->SetColor(row, col, 'A', 'B', 'C');
			}
		}

		CHECK(bmp->GetHeight() == rowCount);
		CHECK(bmp->GetWidth() == colCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp->GetColor(row, col, &R, &G, &B);
				CHECK(R == 'A');
				CHECK(G == 'B');
				CHECK(B == 'C');
			}
		}

		delete bmp;
	}
}

//----------------------------------------------------------------------------
TEST(BmpFile, FileIO_Variable)
{
	// test read/write of variable data
	//

	XString fileName = XString::CombinePath(gs_DataPath, "FileIOVar.bmp");
	const int rowCount = 256;
	const int colCount = 256;

	// write file

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Create(colCount, rowCount));

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				bmp.SetColor(row, col, (UInt8)row, (UInt8)col, (UInt8)row);
			}
		}

		CHECK(bmp.GetHeight() == rowCount);
		CHECK(bmp.GetWidth() == colCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == (UInt8)row);
				CHECK(G == (UInt8)col);
				CHECK(B == (UInt8)row);
			}
		}

		CHECK(bmp.Save(fileName.c_str()) == true);
	}

	// read back file

	{
		int row = 0;
		int col = 0;

		BmpFile bmp;
		CHECK(bmp.Load(fileName.c_str()) == true);

		CHECK(bmp.GetWidth() == colCount);
		CHECK(bmp.GetHeight() == rowCount);

		for (row = 0; row < rowCount; row++)
		{
			for (col = 0; col < colCount; col++)
			{
				UInt8 R = 0;
				UInt8 G = 0;
				UInt8 B = 0;
				bmp.GetColor(row, col, &R, &G, &B);
				CHECK(R == (UInt8)row);
				CHECK(G == (UInt8)col);
				CHECK(B == (UInt8)row);
			}
		}
	}
}

//----------------------------------------------------------------------------
TEST(BmpFile, DataSize)
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
