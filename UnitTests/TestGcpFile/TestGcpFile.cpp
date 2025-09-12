// TestGcpFile.cpp
//
// CppUnitLite test harness for GcpFile class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CppUnitLite/TestHarness.h"
#include "../Common/UnitTest.h"			// unit test helpers
#include "GcpFile.h"			// interface to class under test

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
TEST(GcpFile, basic)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "gcp_list_epsg.txt");

		GcpFile gcpFile(file_name);
		LONGS_EQUAL(3, gcpFile.entry_count());
	}

	{
		XString file_name = XString::CombinePath(gs_DataPath, "gcp_list_proj.txt");

		GcpFile gcpFile(file_name);
		LONGS_EQUAL(3, gcpFile.entry_count());
	}

	{
		XString file_name = XString::CombinePath(gs_DataPath, "gcp_list_wgs84.txt");

		GcpFile gcpFile(file_name);
		LONGS_EQUAL(3, gcpFile.entry_count());
	}
}

//----------------------------------------------------------------------------
TEST(GcpFile, wgs84)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "gcp_list_epsg.txt");

		GcpFile gcpFile(file_name);
		CHECK(gcpFile.wgs84_utm_zone() == "WGS84 UTM 43N");
	}

	{
		XString file_name = XString::CombinePath(gs_DataPath, "gcp_list_proj.txt");

		GcpFile gcpFile(file_name);
		CHECK(gcpFile.wgs84_utm_zone() == "WGS84 UTM 10N");
	}

	{
		XString file_name = XString::CombinePath(gs_DataPath, "gcp_list_wgs84.txt");

		GcpFile gcpFile(file_name);
		CHECK(gcpFile.wgs84_utm_zone() == "WGS84 UTM 17N");
	}
}


//----------------------------------------------------------------------------
TEST(GcpFile, make_resized_copy)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "gcp_list_epsg.txt");

		GcpFile gcpFile(file_name);
		LONGS_EQUAL(3, gcpFile.entry_count());

		LONGS_EQUAL(3044, gcpFile.m_entry_list[0].px);
		LONGS_EQUAL(2622, gcpFile.m_entry_list[0].py);

		XString out_file = XString::CombinePath(gs_DataPath, "out_list_resized.txt");
		
		gcpFile.make_resized_copy(out_file, 0.5);

		{
			GcpFile gcpFile(out_file);

			LONGS_EQUAL(3, gcpFile.entry_count());

			LONGS_EQUAL(1522, gcpFile.m_entry_list[0].px);
			LONGS_EQUAL(1311, gcpFile.m_entry_list[0].py);
		}
	}
}

//----------------------------------------------------------------------------
TEST(GcpFile, make_filtered_copy)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "gcp_list_boruszyn.txt");

		GcpFile gcpFile(file_name);
		LONGS_EQUAL(18, gcpFile.entry_count());

		XString out_file = XString::CombinePath(gs_DataPath, "out_list_filtered.txt");
		XString image_path = XString::CombinePath(gs_DataPath, "images");
		
		gcpFile.make_filtered_copy(out_file, image_path, 3);

		{
			GcpFile gcpFile(out_file);

			LONGS_EQUAL(6, gcpFile.entry_count());
		}
	}
}


//----------------------------------------------------------------------------
TEST(GcpFile, make_utm_copy)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "gcp_list_epsg.txt");

		GcpFile gcpFile(file_name);
		LONGS_EQUAL(3, gcpFile.entry_count());

		XString out_file = XString::CombinePath(gs_DataPath, "out_list_utm.txt");

		gcpFile.make_utm_copy(out_file);

		{
			GcpFile gcpFile(out_file);
			LONGS_EQUAL(3, gcpFile.entry_count());
			//how to verify?
		}
	}
}
