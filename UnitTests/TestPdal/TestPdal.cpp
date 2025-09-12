// TestPdal.cpp
// General test suite for pdal.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "../Common/UnitTest.h"			// unit test helpers
#include "TextFile.h"

#include <pdal/util/Extractor.h>
#include <pdal/util/Inserter.h>

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
TEST(PDAL, ByteSwap)
{
	// test endian.h/ByteSwap()

	{
		// 16-bit
		uint16_t val = 0xABCD;
		LONGS_EQUAL(0xCDAB, ByteSwap(val));
	}
	{
		// 32-bit
		uint32_t val = 0x12345678;
		LONGS_EQUAL(0x78563412, ByteSwap(val));
	}
	{
		// 64-bit
		uint64_t val = 0x12345678ABCDEF01;
		CHECK(0x01EFCDAB78563412 == ByteSwap(val));
	}
}

//----------------------------------------------------------------------------
TEST(PDAL, Extractor)
{
	// The classes themselves are trivial, but they are based on the
	// core endiannes routines; this is really a test of endianness
	// handling which enables the removal of the winsock library
	
	// Little Endian

	{
		// 8-bit, no endian operations

		char buf[32] = {0x12,0x34};
		pdal::LeExtractor ex(buf, sizeof(buf));

		uint8_t byte0 = 0;
		int8_t byte1 = 0;

		ex >> byte0;
		ex >> byte1;

		LONGS_EQUAL(0x12, byte0);
		LONGS_EQUAL(0x34, byte1);
	}
	{
		// 16-bit, potential endian operations, but defined as
		// nop on this arch

		char buf[32] = {0x12,0x34,0x56,0x78};
		pdal::LeExtractor ex(buf, sizeof(buf));

		uint16_t word0 = 0;
		int16_t word1 = 0;

		ex >> word0;
		ex >> word1;

		LONGS_EQUAL(0x3412, word0);
		LONGS_EQUAL(0x7856, word1);
	}
	{
		// 32-bit

		char buf[32] = {0x12,0x34,0x56,0x78, 
						0x0A,0x0B,0x0C,0x0D};
		pdal::LeExtractor ex(buf, sizeof(buf));

		uint32_t dword0 = 0;
		int32_t dword1 = 0;

		ex >> dword0;
		ex >> dword1; 

		LONGS_EQUAL(0x78563412, dword0);
		LONGS_EQUAL(0x0D0C0B0A, dword1);
	}
	{
		// 64-bit

		char buf[32] = {0x12,0x34,0x56,0x78,0x0A,0x0B,0x0C,0x0D,
						0x21,0x43,0x65,0x0F,0x1A,0x1B,0x1C,0x1D};
		pdal::LeExtractor ex(buf, sizeof(buf));

		uint64_t dword0 = 0;
		int64_t dword1 = 0;

		ex >> dword0;
		ex >> dword1;

		CHECK(0x0D0C0B0A78563412 == dword0);
		CHECK(0x1D1C1B1A0F654321 == dword1);
	}
	{
		// float

		char buf[32] = {0x00,0x00,0x20,0x40,
						0x00,0x00,0x00,0x00,};
		pdal::LeExtractor ex(buf, sizeof(buf));

		float val0 = 0.0;
		float val1 = 0.0;

		ex >> val0;
		ex >> val1;

		DOUBLES_EQUAL(2.5, val0, 0.0);
		DOUBLES_EQUAL(0.0, val1, 0.0);
	}
	{
		// double
		char buf[32] = {0x00,0x00,0x00,0x00,0x00,0x48,0x55,0x40};

		pdal::LeExtractor ex(buf, sizeof(buf));

		double val0 = 0.0;

		ex >> val0;

		DOUBLES_EQUAL(85.125, val0, 0.0);
	}

	// Big Endian

	{
		// 8-bit, no endian operations

		char buf[32] = {0x12,0x34};
		pdal::BeExtractor ex(buf, sizeof(buf));

		uint8_t byte0 = 0;
		int8_t byte1 = 0;

		ex >> byte0;
		ex >> byte1;

		LONGS_EQUAL(0x12, byte0);
		LONGS_EQUAL(0x34, byte1);
	}
	{
		// 16-bit, potential endian operations, but defined as
		// nop on this arch

		char buf[32] = {0x12,0x34,0x56,0x78};
		pdal::BeExtractor ex(buf, sizeof(buf));

		uint16_t word0 = 0;
		int16_t word1 = 0;

		ex >> word0;
		ex >> word1;

		LONGS_EQUAL(0x1234, word0);
		LONGS_EQUAL(0x5678, word1);
	}
	{
		// 32-bit

		char buf[32] = {0x12,0x34,0x56,0x78,
						0x0A,0x0B,0x0C,0x0D};
		pdal::BeExtractor ex(buf, sizeof(buf));

		uint32_t dword0 = 0;
		int32_t dword1 = 0;

		ex >> dword0;
		ex >> dword1; 

		LONGS_EQUAL(0x12345678, dword0);
		LONGS_EQUAL(0x0A0B0C0D, dword1);
	}
	{
		// 64-bit

		char buf[32] = {0x12,0x34,0x56,0x78,0x0A,0x0B,0x0C,0x0D,
						0x21,0x43,0x65,0x0F,0x1A,0x1B,0x1C,0x1D};
		pdal::BeExtractor ex(buf, sizeof(buf));

		uint64_t dword0 = 0;
		int64_t dword1 = 0;

		ex >> dword0;
		ex >> dword1;

		CHECK(0x123456780A0B0C0D == dword0);
		CHECK(0x2143650F1A1B1C1D == dword1);
	}
	{
		// float

		char buf[32] = {0x40,0x20,0x00,0x00,
						0x00,0x00,0x00,0x00,};
		pdal::BeExtractor ex(buf, sizeof(buf));

		float val0 = 0.0;
		float val1 = 0.0;

		ex >> val0;
		ex >> val1;

		DOUBLES_EQUAL(2.5, val0, 0.0);
		DOUBLES_EQUAL(0.0, val1, 0.0);
	}
	{
		// double
		char buf[32] = {0x40,0x55,0x48,0x00,0x00,0x00,0x00,0x00};

		pdal::BeExtractor ex(buf, sizeof(buf));

		double val0 = 0.0;

		ex >> val0;

		DOUBLES_EQUAL(85.125, val0, 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(PDAL, Inserter)
{
	// Little Endian

	{
		// 8-bit, no endian operations

		char buf[16] = {0};
		pdal::LeInserter in(buf, sizeof(buf));

		uint8_t byte0 = 0x12;
		int8_t byte1 = 0x34;

		in << byte0;
		in << byte1;

		LONGS_EQUAL(0x12, buf[0]);
		LONGS_EQUAL(0x34, buf[1]);
	}
	{
		// 16-bit

		char buf[16] = {0};
		pdal::LeInserter in(buf, sizeof(buf));

		uint16_t word0 = 0x1234;
		int16_t word1 = 0x5678;

		in << word0;
		in << word1;

		LONGS_EQUAL(0x34, buf[0]);
		LONGS_EQUAL(0x12, buf[1]);

		LONGS_EQUAL(0x78, buf[2]);
		LONGS_EQUAL(0x56, buf[3]);
	}
	{
		// 32-bit

		char buf[16] = {0};
		pdal::LeInserter in(buf, sizeof(buf));

		uint32_t dword0 = 0x1A2B3C4D;
		int32_t dword1  = 0x01020304;

		in << dword0;
		in << dword1;

		LONGS_EQUAL(0x4D, buf[0]);
		LONGS_EQUAL(0x3C, buf[1]);
		LONGS_EQUAL(0x2B, buf[2]);
		LONGS_EQUAL(0x1A, buf[3]);

		LONGS_EQUAL(0x04, buf[4]);
		LONGS_EQUAL(0x03, buf[5]);
		LONGS_EQUAL(0x02, buf[6]);
		LONGS_EQUAL(0x01, buf[7]);
	}
	{
		// 64-bit

		char buf[16] = {0};
		pdal::LeInserter in(buf, sizeof(buf));

		uint64_t qword0 = 0x1A2B3C4D01020304;
		int64_t qword1  = 0x0102030405060708;

		in << qword0;
		in << qword1;

		LONGS_EQUAL(0x04, buf[0]);
		LONGS_EQUAL(0x03, buf[1]);
		LONGS_EQUAL(0x02, buf[2]);
		LONGS_EQUAL(0x01, buf[3]);
		LONGS_EQUAL(0x4D, buf[4]);
		LONGS_EQUAL(0x3C, buf[5]);
		LONGS_EQUAL(0x2B, buf[6]);
		LONGS_EQUAL(0x1A, buf[7]);

		LONGS_EQUAL(0x08, buf[8]);
		LONGS_EQUAL(0x07, buf[9]);
		LONGS_EQUAL(0x06, buf[10]);
		LONGS_EQUAL(0x05, buf[11]);
		LONGS_EQUAL(0x04, buf[12]);
		LONGS_EQUAL(0x03, buf[13]);
		LONGS_EQUAL(0x02, buf[14]);
		LONGS_EQUAL(0x01, buf[15]);
	}
	{
		// float

		char buf[32] = {0};

		pdal::LeInserter in(buf, sizeof(buf));

		float val0 = 2.5;
		float val1 = 0.0;

		in << val0;
		in << val1;

		LONGS_EQUAL(0x00, buf[0]);
		LONGS_EQUAL(0x00, buf[1]);
		LONGS_EQUAL(0x20, buf[2]);
		LONGS_EQUAL(0x40, buf[3]);

		LONGS_EQUAL(0x00, buf[4]);
		LONGS_EQUAL(0x00, buf[5]);
		LONGS_EQUAL(0x00, buf[6]);
		LONGS_EQUAL(0x00, buf[7]);
	}
	{
		// double
		char buf[32] = {0};

		pdal::LeInserter in(buf, sizeof(buf));

		double val0 = 85.125;

		in << val0;

		LONGS_EQUAL(0x00, buf[0]);
		LONGS_EQUAL(0x00, buf[1]);
		LONGS_EQUAL(0x00, buf[2]);
		LONGS_EQUAL(0x00, buf[3]);

		LONGS_EQUAL(0x00, buf[4]);
		LONGS_EQUAL(0x48, buf[5]);
		LONGS_EQUAL(0x55, buf[6]);
		LONGS_EQUAL(0x40, buf[7]);
	}

	// Big Endian

	{
		// 8-bit, no endian operations

		char buf[16] = {0};
		pdal::BeInserter in(buf, sizeof(buf));

		uint8_t byte0 = 0x12;
		int8_t byte1 = 0x34;

		in << byte0;
		in << byte1;

		LONGS_EQUAL(0x12, buf[0]);
		LONGS_EQUAL(0x34, buf[1]);
	}
	{
		// 16-bit

		char buf[16] = {0};
		pdal::BeInserter in(buf, sizeof(buf));

		uint16_t word0 = 0x1234;
		int16_t word1 = 0x5678;

		in << word0;
		in << word1;

		LONGS_EQUAL(0x12, buf[0]);
		LONGS_EQUAL(0x34, buf[1]);

		LONGS_EQUAL(0x56, buf[2]);
		LONGS_EQUAL(0x78, buf[3]);
	}
	{
		// 32-bit

		char buf[16] = {0};
		pdal::BeInserter in(buf, sizeof(buf));

		uint32_t dword0 = 0x1A2B3C4D;
		int32_t dword1  = 0x01020304;

		in << dword0;
		in << dword1;

		LONGS_EQUAL(0x1A, buf[0]);
		LONGS_EQUAL(0x2B, buf[1]);
		LONGS_EQUAL(0x3C, buf[2]);
		LONGS_EQUAL(0x4D, buf[3]);

		LONGS_EQUAL(0x01, buf[4]);
		LONGS_EQUAL(0x02, buf[5]);
		LONGS_EQUAL(0x03, buf[6]);
		LONGS_EQUAL(0x04, buf[7]);
	}
	{
		// 64-bit

		char buf[16] = {0};
		pdal::BeInserter in(buf, sizeof(buf));

		uint64_t qword0 = 0x1A2B3C4D01020304;
		int64_t qword1  = 0x0102030405060708;

		in << qword0;
		in << qword1;

		LONGS_EQUAL(0x1A, buf[0]);
		LONGS_EQUAL(0x2B, buf[1]);
		LONGS_EQUAL(0x3C, buf[2]);
		LONGS_EQUAL(0x4D, buf[3]);
		LONGS_EQUAL(0x01, buf[4]);
		LONGS_EQUAL(0x02, buf[5]);
		LONGS_EQUAL(0x03, buf[6]);
		LONGS_EQUAL(0x04, buf[7]);

		LONGS_EQUAL(0x01, buf[8]);
		LONGS_EQUAL(0x02, buf[9]);
		LONGS_EQUAL(0x03, buf[10]);
		LONGS_EQUAL(0x04, buf[11]);
		LONGS_EQUAL(0x05, buf[12]);
		LONGS_EQUAL(0x06, buf[13]);
		LONGS_EQUAL(0x07, buf[14]);
		LONGS_EQUAL(0x08, buf[15]);
	}
	{
		// float

		char buf[32] = {0};

		pdal::BeInserter in(buf, sizeof(buf));

		float val0 = 2.5;
		float val1 = 0.0;

		in << val0;
		in << val1;

		LONGS_EQUAL(0x40, buf[0]);
		LONGS_EQUAL(0x20, buf[1]);
		LONGS_EQUAL(0x00, buf[2]);
		LONGS_EQUAL(0x00, buf[3]);

		LONGS_EQUAL(0x00, buf[4]);
		LONGS_EQUAL(0x00, buf[5]);
		LONGS_EQUAL(0x00, buf[6]);
		LONGS_EQUAL(0x00, buf[7]);
	}
	{
		// double
		char buf[32] = {0};

		pdal::BeInserter in(buf, sizeof(buf));

		double val0 = 85.125;

		in << val0;

		LONGS_EQUAL(0x40, buf[0]);
		LONGS_EQUAL(0x55, buf[1]);
		LONGS_EQUAL(0x48, buf[2]);
		LONGS_EQUAL(0x00, buf[3]);

		LONGS_EQUAL(0x00, buf[4]);
		LONGS_EQUAL(0x00, buf[5]);
		LONGS_EQUAL(0x00, buf[6]);
		LONGS_EQUAL(0x00, buf[7]);
	}
}
