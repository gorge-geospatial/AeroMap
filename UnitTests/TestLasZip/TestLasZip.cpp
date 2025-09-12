// TestLasZip.cpp
// CppUnitLite test harness for LasZip library
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#include "CppUnitLite/TestHarness.h"
#include "../Common/UnitTest.h"			// unit test helpers

#include "MarkTypes.h"
#include "XString.h"

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "laswritercompatible.hpp"
#include "laswaveform13reader.hpp"
#include "laswaveform13writer.hpp"
#include "bytestreamin.hpp"
#include "bytestreamout_array.hpp"
#include "bytestreamin_array.hpp"
//#include "geoprojectionconverter.hpp"
#include "lasindex.hpp"
#include "lasquadtree.hpp"

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
TEST(LasFile, RW_Test_10_0)
{
	// Test writing & reading back a .laz file, version 1.0, point format 0
	// 

	XString fileName = XString::CombinePath(gs_DataPath, "temp.laz");
	const int POINT_COUNT = 100;

	{
		// write

		LASwriteOpener lasWriteOpener;
		lasWriteOpener.set_file_name(fileName.c_str());
		if (!lasWriteOpener.active())
		{
			fprintf(stderr, "ERROR: no output specified\n");
		}

		// init header

		LASheader lasheader;
		lasheader.version_major = 1;
		lasheader.version_minor = 0;
		lasheader.point_data_format = 0;
		lasheader.point_data_record_length = 20;
		lasheader.file_creation_day = 123;
		lasheader.file_creation_year = 2018;
		lasheader.x_scale_factor = 0.1;
		lasheader.y_scale_factor = 0.01;
		lasheader.z_scale_factor = 0.001;
		lasheader.x_offset = 1000.0;
		lasheader.y_offset = 2000.0;
		lasheader.z_offset = 0.0;
		strcpy(lasheader.system_identifier, "System Identifier");
		strcpy(lasheader.generating_software, "Generating Software");

		// init point 

		LASpoint laspoint;
		laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);

		// open laswriter

		LASwriter* pLasWriter = lasWriteOpener.open(&lasheader);
		CHECK(pLasWriter != nullptr);
		if (pLasWriter == nullptr)
		{
			fprintf(stderr, "ERROR: could not open laswriter\n");
			CHECK(false);
		}

		// write points

		for (int i = 0; i < POINT_COUNT; i++)
		{
			// populate the point

			laspoint.set_X(i);
			laspoint.set_Y(i);
			laspoint.set_Z(i);
			laspoint.set_intensity((UInt16)i);

			// write the point

			pLasWriter->write_point(&laspoint);

			// add it to the inventory

			pLasWriter->update_inventory(&laspoint);
		}

		// update the header

		pLasWriter->update_header(&lasheader, true);

		// close the writer

		I64 total_bytes = pLasWriter->close();

		delete pLasWriter;
	}

	{
		// read it back

		LASreadOpener lasreadopener;
		lasreadopener.set_file_name(fileName.c_str());

		LASreader* pLasReader = lasreadopener.open();
		CHECK(pLasReader != nullptr);

		CHECK(XString(pLasReader->header.file_signature).Compare("LASF"));
		CHECK(pLasReader->header.is_compressed() == true);
		LONGS_EQUAL(1, pLasReader->header.version_major);
		LONGS_EQUAL(0, pLasReader->header.version_minor);
		LONGS_EQUAL(0, pLasReader->header.point_data_format);
		LONGS_EQUAL(20, pLasReader->header.point_data_record_length);
		LONGS_EQUAL(123, pLasReader->header.file_creation_day);
		LONGS_EQUAL(2018, pLasReader->header.file_creation_year);
		LONGS_EQUAL(0, pLasReader->header.file_source_ID);
		LONGS_EQUAL(0, pLasReader->header.global_encoding);
		LONGS_EQUAL(1, pLasReader->header.version_major);
		LONGS_EQUAL(0, pLasReader->header.version_minor);
		DOUBLES_EQUAL(0.1, pLasReader->header.x_scale_factor, 0.0);
		DOUBLES_EQUAL(0.01, pLasReader->header.y_scale_factor, 0.0);
		DOUBLES_EQUAL(0.001, pLasReader->header.z_scale_factor, 0.0);
		DOUBLES_EQUAL(1000.0, pLasReader->header.x_offset, 0.0);
		DOUBLES_EQUAL(2000.0, pLasReader->header.y_offset, 0.0);
		DOUBLES_EQUAL(0.0, pLasReader->header.z_offset, 0.0);
		CHECK(XString(pLasReader->header.system_identifier).Compare("System Identifier"));
		CHECK(XString(pLasReader->header.generating_software).Compare("Generating Software"));

		for (int i = 0; i < POINT_COUNT; i++)
		{
			pLasReader->read_point();
			LONGS_EQUAL(i, pLasReader->point.X);
			LONGS_EQUAL(i, pLasReader->point.Y);
			LONGS_EQUAL(i, pLasReader->point.Z);
			LONGS_EQUAL(i, pLasReader->point.intensity);
		}

		pLasReader->close();
		delete pLasReader;
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, RW_Test_10_1)
{
	// Test writing & reading back a .laz file, version 1.0, point format 1
	// 

	XString fileName = XString::CombinePath(gs_DataPath, "temp.laz");
	const int POINT_COUNT = 100;

	{
		LASwriteOpener lasWriteOpener;

		lasWriteOpener.set_file_name(fileName.c_str());
		if (!lasWriteOpener.active())
		{
			fprintf(stderr, "ERROR: no output specified\n");
		}

		// init header

		LASheader lasheader;
		lasheader.version_major = 1;
		lasheader.version_minor = 0;
		lasheader.point_data_format = 1;
		lasheader.point_data_record_length = 28;
		lasheader.file_creation_day = 123;
		lasheader.file_creation_year = 2018;
		lasheader.x_scale_factor = 0.1;
		lasheader.y_scale_factor = 0.01;
		lasheader.z_scale_factor = 0.001;
		lasheader.x_offset = 1000.0;
		lasheader.y_offset = 2000.0;
		lasheader.z_offset = 0.0;
		strcpy(lasheader.system_identifier, "System Identifier");
		strcpy(lasheader.generating_software, "Generating Software");

		// init point 

		LASpoint laspoint;
		laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);

		// open laswriter

		LASwriter* pLasWriter = lasWriteOpener.open(&lasheader);
		CHECK(pLasWriter != nullptr);
		if (pLasWriter == nullptr)
		{
			fprintf(stderr, "ERROR: could not open laswriter\n");
		}

		fprintf(stderr, "writing 100 points to '%s'.\n", lasWriteOpener.get_file_name());

		// write points

		for (int i = 0; i < POINT_COUNT; i++)
		{
			// populate the point

			laspoint.set_X(i);
			laspoint.set_Y(i);
			laspoint.set_Z(i);
			laspoint.set_intensity((UInt16)i);
			laspoint.set_gps_time(0.0006*i);

			// write the point

			pLasWriter->write_point(&laspoint);

			// add it to the inventory

			pLasWriter->update_inventory(&laspoint);
		}

		// update the header

		pLasWriter->update_header(&lasheader, true);

		// close the writer

		I64 total_bytes = pLasWriter->close();

		delete pLasWriter;
	}

	{
		// read it back

		LASreadOpener lasreadopener;
		lasreadopener.set_file_name(fileName.c_str());

		LASreader* pLasReader = lasreadopener.open();
		CHECK(pLasReader != nullptr);

		CHECK(XString(pLasReader->header.file_signature).Compare("LASF"));
		CHECK(pLasReader->header.is_compressed() == true);
		LONGS_EQUAL(1, pLasReader->header.version_major);
		LONGS_EQUAL(0, pLasReader->header.version_minor);
		LONGS_EQUAL(1, pLasReader->header.point_data_format);
		LONGS_EQUAL(28, pLasReader->header.point_data_record_length);
		LONGS_EQUAL(123, pLasReader->header.file_creation_day);
		LONGS_EQUAL(2018, pLasReader->header.file_creation_year);
		LONGS_EQUAL(0, pLasReader->header.file_source_ID);
		LONGS_EQUAL(0, pLasReader->header.global_encoding);
		LONGS_EQUAL(1, pLasReader->header.version_major);
		LONGS_EQUAL(0, pLasReader->header.version_minor);
		DOUBLES_EQUAL(0.1, pLasReader->header.x_scale_factor, 0.0);
		DOUBLES_EQUAL(0.01, pLasReader->header.y_scale_factor, 0.0);
		DOUBLES_EQUAL(0.001, pLasReader->header.z_scale_factor, 0.0);
		DOUBLES_EQUAL(1000.0, pLasReader->header.x_offset, 0.0);
		DOUBLES_EQUAL(2000.0, pLasReader->header.y_offset, 0.0);
		DOUBLES_EQUAL(0.0, pLasReader->header.z_offset, 0.0);
		CHECK(XString(pLasReader->header.system_identifier).Compare("System Identifier"));
		CHECK(XString(pLasReader->header.generating_software).Compare("Generating Software"));

		for (int i = 0; i < POINT_COUNT; i++)
		{
			pLasReader->read_point();
			LONGS_EQUAL(i, pLasReader->point.X);
			LONGS_EQUAL(i, pLasReader->point.Y);
			LONGS_EQUAL(i, pLasReader->point.Z);
			LONGS_EQUAL(i, pLasReader->point.intensity);
			DOUBLES_EQUAL(0.0006*i, pLasReader->point.gps_time, 0.0);
		}

		pLasReader->close();
		delete pLasReader;
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, RW_Test_12_2)
{
	// Test writing & reading back a .laz file, version 1.2, point format 2
	// 

	XString fileName = XString::CombinePath(gs_DataPath, "temp.laz");
	const int POINT_COUNT = 1000;

	{
		LASwriteOpener lasWriteOpener;

		lasWriteOpener.set_file_name(fileName.c_str());
		if (!lasWriteOpener.active())
		{
			fprintf(stderr, "ERROR: no output specified\n");
		}

		// init header

		LASheader lasheader;
		lasheader.version_major = 1;
		lasheader.version_minor = 2;
		lasheader.point_data_format = 2;
		lasheader.point_data_record_length = 26;
		lasheader.file_creation_day = 123;
		lasheader.file_creation_year = 2018;
		lasheader.x_scale_factor = 0.1;
		lasheader.y_scale_factor = 0.01;
		lasheader.z_scale_factor = 0.001;
		lasheader.x_offset = 1000.0;
		lasheader.y_offset = 2000.0;
		lasheader.z_offset = 0.0;
		strcpy(lasheader.system_identifier, "System Identifier");
		strcpy(lasheader.generating_software, "Generating Software");

		// init point 

		LASpoint laspoint;
		laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);

		// open laswriter

		LASwriter* pLasWriter = lasWriteOpener.open(&lasheader);
		CHECK(pLasWriter != nullptr);
		if (pLasWriter == nullptr)
		{
			fprintf(stderr, "ERROR: could not open laswriter\n");
		}

		fprintf(stderr, "writing 100 points to '%s'.\n", lasWriteOpener.get_file_name());

		// write points

		for (int i = 0; i < POINT_COUNT; i++)
		{
			// populate the point

			laspoint.set_X(i);
			laspoint.set_Y(i);
			laspoint.set_Z(i);
			laspoint.set_intensity((UInt16)i);
			laspoint.set_scan_angle_rank(-18);
			laspoint.set_user_data(0xAB);
			laspoint.set_point_source_ID(0xABCD);
			laspoint.set_R(10 + i % 255);
			laspoint.set_G(20 + i % 255);
			laspoint.set_B(30 + i % 255);

			// write the point

			pLasWriter->write_point(&laspoint);

			// add it to the inventory

			pLasWriter->update_inventory(&laspoint);
		}

		// update the header

		pLasWriter->update_header(&lasheader, true);

		// close the writer

		I64 total_bytes = pLasWriter->close();

		delete pLasWriter;
	}

	{
		// read it back

		LASreadOpener lasreadopener;
		lasreadopener.set_file_name(fileName.c_str());

		LASreader* pLasReader = lasreadopener.open();
		CHECK(pLasReader != nullptr);

		CHECK(XString(pLasReader->header.file_signature).Compare("LASF"));
		CHECK(pLasReader->header.is_compressed() == true);
		LONGS_EQUAL(1, pLasReader->header.version_major);
		LONGS_EQUAL(2, pLasReader->header.version_minor);
		LONGS_EQUAL(2, pLasReader->header.point_data_format);
		LONGS_EQUAL(26, pLasReader->header.point_data_record_length);
		LONGS_EQUAL(123, pLasReader->header.file_creation_day);
		LONGS_EQUAL(2018, pLasReader->header.file_creation_year);
		LONGS_EQUAL(0, pLasReader->header.file_source_ID);
		LONGS_EQUAL(0, pLasReader->header.global_encoding);
		LONGS_EQUAL(1, pLasReader->header.version_major);
		LONGS_EQUAL(2, pLasReader->header.version_minor);
		DOUBLES_EQUAL(0.1, pLasReader->header.x_scale_factor, 0.0);
		DOUBLES_EQUAL(0.01, pLasReader->header.y_scale_factor, 0.0);
		DOUBLES_EQUAL(0.001, pLasReader->header.z_scale_factor, 0.0);
		DOUBLES_EQUAL(1000.0, pLasReader->header.x_offset, 0.0);
		DOUBLES_EQUAL(2000.0, pLasReader->header.y_offset, 0.0);
		DOUBLES_EQUAL(0.0, pLasReader->header.z_offset, 0.0);
		CHECK(XString(pLasReader->header.system_identifier).Compare("System Identifier"));
		CHECK(XString(pLasReader->header.generating_software).Compare("Generating Software"));

		for (int i = 0; i < POINT_COUNT; i++)
		{
			pLasReader->read_point();
			LONGS_EQUAL(i, pLasReader->point.X);
			LONGS_EQUAL(i, pLasReader->point.Y);
			LONGS_EQUAL(i, pLasReader->point.Z);
			LONGS_EQUAL(i, pLasReader->point.intensity);
			LONGS_EQUAL(-18, pLasReader->point.scan_angle_rank);
			LONGS_EQUAL(0xAB, pLasReader->point.user_data);
			LONGS_EQUAL(0xABCD, pLasReader->point.point_source_ID);
			LONGS_EQUAL(10 + i % 255, pLasReader->point.get_R());
			LONGS_EQUAL(20 + i % 255, pLasReader->point.get_G());
			LONGS_EQUAL(30 + i % 255, pLasReader->point.get_B());
		}

		pLasReader->close();
		delete pLasReader;
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, RW_Test_12_3)
{
	// Test writing & reading back a .laz file, version 1.2, point format 3
	// 

	XString fileName = XString::CombinePath(gs_DataPath, "temp.laz");
	const int POINT_COUNT = 1000;

	{
		LASwriteOpener lasWriteOpener;

		lasWriteOpener.set_file_name(fileName.c_str());
		if (!lasWriteOpener.active())
		{
			fprintf(stderr, "ERROR: no output specified\n");
		}

		// init header

		LASheader lasheader;
		lasheader.version_major = 1;
		lasheader.version_minor = 2;
		lasheader.point_data_format = 3;
		lasheader.point_data_record_length = 34;
		lasheader.file_creation_day = 123;
		lasheader.file_creation_year = 2018;
		lasheader.x_scale_factor = 0.1;
		lasheader.y_scale_factor = 0.01;
		lasheader.z_scale_factor = 0.001;
		lasheader.x_offset = 1000.0;
		lasheader.y_offset = 2000.0;
		lasheader.z_offset = 0.0;
		strcpy(lasheader.system_identifier, "System Identifier");
		strcpy(lasheader.generating_software, "Generating Software");

		// init point 

		LASpoint laspoint;
		laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);

		// open laswriter

		LASwriter* pLasWriter = lasWriteOpener.open(&lasheader);
		CHECK(pLasWriter != nullptr);
		if (pLasWriter == nullptr)
		{
			fprintf(stderr, "ERROR: could not open laswriter\n");
		}

		fprintf(stderr, "writing 100 points to '%s'.\n", lasWriteOpener.get_file_name());

		// write points

		for (int i = 0; i < POINT_COUNT; i++)
		{
			// populate the point

			laspoint.set_X(i);
			laspoint.set_Y(i);
			laspoint.set_Z(i);
			laspoint.set_intensity((UInt16)i);
			laspoint.set_scan_angle_rank(-18);
			laspoint.set_user_data(0xAB);
			laspoint.set_point_source_ID(0xABCD);
			laspoint.set_gps_time(static_cast<double>(i) * 1.234);
			laspoint.set_R(10 + i % 255);
			laspoint.set_G(20 + i % 255);
			laspoint.set_B(30 + i % 255);

			// write the point

			pLasWriter->write_point(&laspoint);

			// add it to the inventory

			pLasWriter->update_inventory(&laspoint);
		}

		// update the header

		pLasWriter->update_header(&lasheader, true);

		// close the writer

		I64 total_bytes = pLasWriter->close();

		delete pLasWriter;
	}

	{
		// read it back

		LASreadOpener lasreadopener;
		lasreadopener.set_file_name(fileName.c_str());

		LASreader* pLasReader = lasreadopener.open();
		CHECK(pLasReader != nullptr);

		CHECK(XString(pLasReader->header.file_signature).Compare("LASF"));
		CHECK(pLasReader->header.is_compressed() == true);
		LONGS_EQUAL(1, pLasReader->header.version_major);
		LONGS_EQUAL(2, pLasReader->header.version_minor);
		LONGS_EQUAL(3, pLasReader->header.point_data_format);
		LONGS_EQUAL(34, pLasReader->header.point_data_record_length);
		LONGS_EQUAL(123, pLasReader->header.file_creation_day);
		LONGS_EQUAL(2018, pLasReader->header.file_creation_year);
		LONGS_EQUAL(0, pLasReader->header.file_source_ID);
		LONGS_EQUAL(0, pLasReader->header.global_encoding);
		LONGS_EQUAL(1, pLasReader->header.version_major);
		LONGS_EQUAL(2, pLasReader->header.version_minor);
		DOUBLES_EQUAL(0.1, pLasReader->header.x_scale_factor, 0.0);
		DOUBLES_EQUAL(0.01, pLasReader->header.y_scale_factor, 0.0);
		DOUBLES_EQUAL(0.001, pLasReader->header.z_scale_factor, 0.0);
		DOUBLES_EQUAL(1000.0, pLasReader->header.x_offset, 0.0);
		DOUBLES_EQUAL(2000.0, pLasReader->header.y_offset, 0.0);
		DOUBLES_EQUAL(0.0, pLasReader->header.z_offset, 0.0);
		CHECK(XString(pLasReader->header.system_identifier).Compare("System Identifier"));
		CHECK(XString(pLasReader->header.generating_software).Compare("Generating Software"));

		for (int i = 0; i < POINT_COUNT; i++)
		{
			pLasReader->read_point();
			LONGS_EQUAL(i, pLasReader->point.X);
			LONGS_EQUAL(i, pLasReader->point.Y);
			LONGS_EQUAL(i, pLasReader->point.Z);
			LONGS_EQUAL(i, pLasReader->point.intensity);
			LONGS_EQUAL(-18, pLasReader->point.scan_angle_rank);
			LONGS_EQUAL(0xAB, pLasReader->point.user_data);
			LONGS_EQUAL(0xABCD, pLasReader->point.point_source_ID);
			DOUBLES_EQUAL(static_cast<double>(i) * 1.234, pLasReader->point.get_gps_time(), 0.0);
			LONGS_EQUAL(10 + i % 255, pLasReader->point.get_R());
			LONGS_EQUAL(20 + i % 255, pLasReader->point.get_G());
			LONGS_EQUAL(30 + i % 255, pLasReader->point.get_B());
		}

		pLasReader->close();
		delete pLasReader;
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Test_10_0)
{
	// Test reading a .laz file, version 1.0, point format 0
	// 
	// Pretty good test, but VLR data not covered and points only read a couple.
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "1.0_0.laz");

		LASreadOpener lasreadopener;
		lasreadopener.set_file_name(fileName.c_str());

		LASreader* pLasReader = lasreadopener.open();
		CHECK(pLasReader != nullptr);

		CHECK(pLasReader->header.is_compressed() == true);

		CHECK(XString(pLasReader->header.file_signature).Compare("LASF"));

		LONGS_EQUAL(1, pLasReader->header.version_major);
		LONGS_EQUAL(0, pLasReader->header.version_minor);

		LONGS_EQUAL(0, pLasReader->header.point_data_format);

		LONGS_EQUAL(0, pLasReader->header.file_source_ID);
		LONGS_EQUAL(0, pLasReader->header.global_encoding);
		CHECK(XString(pLasReader->header.system_identifier).Compare("libLAS"));
		CHECK(XString(pLasReader->header.generating_software).Compare("libLAS 1.2"));
		LONGS_EQUAL(78, pLasReader->header.file_creation_day);
		LONGS_EQUAL(2008, pLasReader->header.file_creation_year);
		LONGS_EQUAL(227, pLasReader->header.header_size);
		LONGS_EQUAL(440, pLasReader->header.offset_to_point_data);
		LONGS_EQUAL(20, pLasReader->header.point_data_record_length);
		LONGS_EQUAL(1, pLasReader->header.number_of_point_records);
		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[0]);
		LONGS_EQUAL(1, pLasReader->header.number_of_points_by_return[1]);
		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[2]);
		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[3]);
		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[4]);

		DOUBLES_EQUAL(470692.447538, pLasReader->header.min_x, 0.0);
		DOUBLES_EQUAL(470692.447538, pLasReader->header.max_x, 0.0);
		DOUBLES_EQUAL(4602888.9046412, pLasReader->header.min_y, 1E-6);
		DOUBLES_EQUAL(4602888.9046412, pLasReader->header.max_y, 1E-6);
		DOUBLES_EQUAL(16.00, pLasReader->header.min_z, 0.0);
		DOUBLES_EQUAL(16.00, pLasReader->header.max_z, 0.0);

		DOUBLES_EQUAL(0.01, pLasReader->header.x_scale_factor, 0.0);
		DOUBLES_EQUAL(0.01, pLasReader->header.y_scale_factor, 0.0);
		DOUBLES_EQUAL(0.01, pLasReader->header.z_scale_factor, 0.0);

		DOUBLES_EQUAL(0.0, pLasReader->header.x_offset, 0.0);
		DOUBLES_EQUAL(0.0, pLasReader->header.y_offset, 0.0);
		DOUBLES_EQUAL(0.0, pLasReader->header.z_offset, 0.0);


		// vlr record 1/3
		CHECK(XString(pLasReader->header.vlrs[0].user_id).Compare("LASF_Projection"));
		CHECK(XString(pLasReader->header.vlrs[0].description).Compare(""));
		LONGS_EQUAL(34735, pLasReader->header.vlrs[0].record_id);
		LONGS_EQUAL(64, pLasReader->header.vlrs[0].record_length_after_header);
		//TODO:
		//need to parse out and verify "vlrs[].data"

		// vlr record 2/3
		CHECK(XString(pLasReader->header.vlrs[1].user_id).Compare("LASF_Projection"));
		CHECK(XString(pLasReader->header.vlrs[1].description).Compare(""));
		LONGS_EQUAL(34737, pLasReader->header.vlrs[1].record_id);
		LONGS_EQUAL(39, pLasReader->header.vlrs[1].record_length_after_header);

		// vlr record 3/3
		CHECK(XString(pLasReader->header.vlrs[1].user_id).Compare("LASF_Projection"));
		CHECK(XString(pLasReader->header.vlrs[1].description).Compare(""));
		LONGS_EQUAL(34737, pLasReader->header.vlrs[1].record_id);
		LONGS_EQUAL(39, pLasReader->header.vlrs[1].record_length_after_header);

		
		// point records

		CHECK(pLasReader->read_point());
		LONGS_EQUAL(47069244, pLasReader->point.X);
		LONGS_EQUAL(460288890, pLasReader->point.Y);
		LONGS_EQUAL(1600, pLasReader->point.Z);

		pLasReader->close();
		delete pLasReader;
	}
}

//----------------------------------------------------------------------------
TEST(LasFile, Read_Test_11_1)
{
	// Test reading a .laz file, version 1.1, point format 1
	// 
	// Pretty good test, but VLR data not covered and points only read a couple.
	//

	{
		XString fileName = XString::CombinePath(gs_DataPath, "fusa-1.1_1.laz");

		LASreadOpener lasreadopener;
		lasreadopener.set_file_name(fileName.c_str());

		LASreader* pLasReader = lasreadopener.open();
		CHECK(pLasReader != nullptr);

		CHECK(pLasReader->header.is_compressed() == true);

		CHECK(XString(pLasReader->header.file_signature).Compare("LASF"));
		LONGS_EQUAL(0, pLasReader->header.file_source_ID);
		LONGS_EQUAL(0, pLasReader->header.global_encoding);
		LONGS_EQUAL(1, pLasReader->header.version_major);
		LONGS_EQUAL(1, pLasReader->header.version_minor);
		CHECK(XString(pLasReader->header.system_identifier).Compare("LAStools (c) by Martin Isenburg"));
		CHECK(XString(pLasReader->header.generating_software).Compare("lasclassify (110920) unlicensed"));
		LONGS_EQUAL(40, pLasReader->header.file_creation_day);
		LONGS_EQUAL(2010, pLasReader->header.file_creation_year);
		LONGS_EQUAL(227, pLasReader->header.header_size);
		LONGS_EQUAL(321, pLasReader->header.offset_to_point_data);
		LONGS_EQUAL(1, pLasReader->header.number_of_variable_length_records);
		LONGS_EQUAL(1, pLasReader->header.point_data_format);
		LONGS_EQUAL(28, pLasReader->header.point_data_record_length);
		LONGS_EQUAL(277573, pLasReader->header.number_of_point_records);
		LONGS_EQUAL(263413, pLasReader->header.number_of_points_by_return[0]);
		LONGS_EQUAL(13879, pLasReader->header.number_of_points_by_return[1]);
		LONGS_EQUAL(281, pLasReader->header.number_of_points_by_return[2]);
		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[3]);
		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[4]);

		DOUBLES_EQUAL(277750.00, pLasReader->header.min_x, 0.0);
		DOUBLES_EQUAL(277999.99, pLasReader->header.max_x, 0.0);
		DOUBLES_EQUAL(6122250.00, pLasReader->header.min_y, 0.0);
		DOUBLES_EQUAL(6122499.99, pLasReader->header.max_y, 1E-9);
		DOUBLES_EQUAL(42.21, pLasReader->header.min_z, 0.0);
		DOUBLES_EQUAL(64.35, pLasReader->header.max_z, 0.0);

		DOUBLES_EQUAL(0.01, pLasReader->header.x_scale_factor, 0.0);
		DOUBLES_EQUAL(0.01, pLasReader->header.y_scale_factor, 0.0);
		DOUBLES_EQUAL(0.01, pLasReader->header.z_scale_factor, 0.0);

		DOUBLES_EQUAL(0.0, pLasReader->header.x_offset, 0.0);
		DOUBLES_EQUAL(0.0, pLasReader->header.y_offset, 0.0);
		DOUBLES_EQUAL(0.0, pLasReader->header.z_offset, 0.0);

		// record 1/1
		CHECK(XString(pLasReader->header.vlrs[0].user_id).Compare("LASF_Projection"));
		CHECK(XString(pLasReader->header.vlrs[0].description).Compare("by LAStools of Martin Isenburg"));
		LONGS_EQUAL(34735, pLasReader->header.vlrs[0].record_id);
		LONGS_EQUAL(40, pLasReader->header.vlrs[0].record_length_after_header);
//TODO:
//need to parse out and verify "vlrs[].data"

		CHECK(pLasReader->read_point());
		LONGS_EQUAL(27799997, pLasReader->point.X);
		LONGS_EQUAL(612234220, pLasReader->point.Y);
		LONGS_EQUAL(6435, pLasReader->point.Z);
		LONGS_EQUAL(10, pLasReader->point.intensity);
		LONGS_EQUAL(1, pLasReader->point.return_number);
		LONGS_EQUAL(2, pLasReader->point.number_of_returns);
		LONGS_EQUAL(5, pLasReader->point.classification);
		LONGS_EQUAL(89, pLasReader->point.scan_angle_rank);
		DOUBLES_EQUAL(5880.9630280000001, pLasReader->point.gps_time, 0.0);
		CHECK(pLasReader->point.have_gps_time == true);
		CHECK(pLasReader->point.have_rgb == false);
		CHECK(pLasReader->point.have_nir == false);
		CHECK(pLasReader->point.have_wavepacket == false);
		
		CHECK(pLasReader->read_point());
		LONGS_EQUAL(27799997, pLasReader->point.X);
		LONGS_EQUAL(612234253, pLasReader->point.Y);
		LONGS_EQUAL(6430, pLasReader->point.Z);
		LONGS_EQUAL(15, pLasReader->point.intensity);
		LONGS_EQUAL(1, pLasReader->point.return_number);
		LONGS_EQUAL(1, pLasReader->point.number_of_returns);
		LONGS_EQUAL(5, pLasReader->point.classification);
		LONGS_EQUAL(89, pLasReader->point.scan_angle_rank);
		DOUBLES_EQUAL(5880.9630319999997, pLasReader->point.gps_time, 0.0);

		pLasReader->seek(277572);
		CHECK(pLasReader->read_point());
		LONGS_EQUAL(27775001, pLasReader->point.X);
		LONGS_EQUAL(612225418, pLasReader->point.Y);
		LONGS_EQUAL(4243, pLasReader->point.Z);
		LONGS_EQUAL(35, pLasReader->point.intensity);
		LONGS_EQUAL(1, pLasReader->point.return_number);
		LONGS_EQUAL(2, pLasReader->point.classification);
		LONGS_EQUAL(80, pLasReader->point.scan_angle_rank);
		DOUBLES_EQUAL(5886.7397380000002, pLasReader->point.gps_time, 0.0);

		pLasReader->close();
		delete pLasReader;
	}
}

////----------------------------------------------------------------------------
//TEST(LasFile, Read_Test_12_4)
//{
//	// Test reading a .laz file, version 1.2, point format 4
//	// 
//	// Pretty good test, but VLR data not covered and points only read a couple.
//	//
//
//	{
//		XString fileName = XString::CombinePath(gs_DataPath, "points-1.2_4.laz");
//
//		LASreadOpener lasreadopener;
//		lasreadopener.set_file_name(fileName.c_str());
//
//		LASreader* pLasReader = lasreadopener.open();
//		CHECK(pLasReader != nullptr);
//
//		CHECK(XString(pLasReader->header.file_signature).Compare("LASF"));
//		CHECK(pLasReader->header.is_compressed() == true);
//
//		LONGS_EQUAL(1, pLasReader->header.version_major);
//		LONGS_EQUAL(2, pLasReader->header.version_minor);
//
//		LONGS_EQUAL(4, pLasReader->header.point_data_format);
//
//		LONGS_EQUAL(0, pLasReader->header.file_source_ID);
//		LONGS_EQUAL(0, pLasReader->header.global_encoding);
//		CHECK(XString(pLasReader->header.system_identifier).Compare("LAStools (c) by rapidlasso GmbH"));
//		CHECK(XString(pLasReader->header.generating_software).Compare("las2las (version 180706)"));
//		LONGS_EQUAL(53, pLasReader->header.file_creation_day);
//		LONGS_EQUAL(2019, pLasReader->header.file_creation_year);
//		LONGS_EQUAL(227, pLasReader->header.header_size);
//		LONGS_EQUAL(1184, pLasReader->header.offset_to_point_data);
//		LONGS_EQUAL(3, pLasReader->header.number_of_variable_length_records);
//		LONGS_EQUAL(57, pLasReader->header.point_data_record_length);
//		LONGS_EQUAL(114200, pLasReader->header.number_of_point_records);
//		LONGS_EQUAL(93148, pLasReader->header.number_of_points_by_return[0]);
//		LONGS_EQUAL(18578, pLasReader->header.number_of_points_by_return[1]);
//		LONGS_EQUAL(2383, pLasReader->header.number_of_points_by_return[2]);
//		LONGS_EQUAL(91, pLasReader->header.number_of_points_by_return[3]);
//		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[4]);
//
//		DOUBLES_EQUAL(966876.73999999999, pLasReader->header.min_x, 0.0);
//		DOUBLES_EQUAL(967303.01000000001, pLasReader->header.max_x, 0.0);
//		DOUBLES_EQUAL(1438530.4600000000, pLasReader->header.min_y, 1E-9);
//		DOUBLES_EQUAL(1438949.9900000000, pLasReader->header.max_y, 1E-9);
//		DOUBLES_EQUAL(78.079999999999998, pLasReader->header.min_z, 1E-9);
//		DOUBLES_EQUAL(223.20000000000002, pLasReader->header.max_z, 1E-9);
//
//		DOUBLES_EQUAL(0.01, pLasReader->header.x_scale_factor, 0.0);
//		DOUBLES_EQUAL(0.01, pLasReader->header.y_scale_factor, 0.0);
//		DOUBLES_EQUAL(0.01, pLasReader->header.z_scale_factor, 0.0);
//
//		DOUBLES_EQUAL(0.0, pLasReader->header.x_offset, 0.0);
//		DOUBLES_EQUAL(0.0, pLasReader->header.y_offset, 0.0);
//		DOUBLES_EQUAL(0.0, pLasReader->header.z_offset, 0.0);
//
//
//		// Variable length records
//
//		// vlr record 1/3
//		CHECK(XString(pLasReader->header.vlrs[0].user_id).Compare("LASF_Projection"));
//		CHECK(XString(pLasReader->header.vlrs[0].description).Compare("GeoTIFF GeoKeyDirectoryTag"));
//		LONGS_EQUAL(34735, pLasReader->header.vlrs[0].record_id);
//		LONGS_EQUAL(64, pLasReader->header.vlrs[0].record_length_after_header);
//		//TODO:
//		//need to parse out and verify "vlrs[].data"
//
//		// vlr record 2/3
//		CHECK(XString(pLasReader->header.vlrs[1].user_id).Compare("LASF_Projection"));
//		CHECK(XString(pLasReader->header.vlrs[1].description).Compare("GeoTIFF GeoAsciiParamsTag"));
//		LONGS_EQUAL(34737, pLasReader->header.vlrs[1].record_id);
//		LONGS_EQUAL(34, pLasReader->header.vlrs[1].record_length_after_header);
//
//		// vlr record 3/3
//		CHECK(XString(pLasReader->header.vlrs[1].user_id).Compare("LASF_Projection"));
//		CHECK(XString(pLasReader->header.vlrs[1].description).Compare("GeoTIFF GeoAsciiParamsTag"));
//		LONGS_EQUAL(34737, pLasReader->header.vlrs[1].record_id);
//		LONGS_EQUAL(34, pLasReader->header.vlrs[1].record_length_after_header);
//
//
//		// Point records
//
//		// read point 0
//		CHECK(pLasReader->read_point());
//		LONGS_EQUAL(96730297, pLasReader->point.X);
//		LONGS_EQUAL(96730297, pLasReader->point.get_X());
//		LONGS_EQUAL(143876416, pLasReader->point.Y);
//		LONGS_EQUAL(143876416, pLasReader->point.get_Y());
//		LONGS_EQUAL(12451, pLasReader->point.Z);
//		LONGS_EQUAL(12451, pLasReader->point.get_Z());
//		DOUBLES_EQUAL(967302.97, pLasReader->point.get_x(), 1E-9);
//		DOUBLES_EQUAL(1438764.16, pLasReader->point.get_y(), 1E-9);
//		DOUBLES_EQUAL(124.51, pLasReader->point.get_z(), 1E-9);
//		LONGS_EQUAL(54, pLasReader->point.intensity);
//		LONGS_EQUAL(54, pLasReader->point.get_intensity());
//		LONGS_EQUAL(2, pLasReader->point.return_number);
//		LONGS_EQUAL(2, pLasReader->point.get_return_number());
//		LONGS_EQUAL(3, pLasReader->point.number_of_returns);
//		LONGS_EQUAL(3, pLasReader->point.get_number_of_returns());
//		LONGS_EQUAL(1, pLasReader->point.classification);
//		LONGS_EQUAL(1, pLasReader->point.get_classification());
//		LONGS_EQUAL(11, pLasReader->point.scan_angle_rank);
//		LONGS_EQUAL(11, pLasReader->point.get_scan_angle_rank());
//		LONGS_EQUAL(1, pLasReader->point.scan_direction_flag);
//		LONGS_EQUAL(1, pLasReader->point.get_scan_direction_flag());
//		LONGS_EQUAL(0, pLasReader->point.edge_of_flight_line);
//		LONGS_EQUAL(0, pLasReader->point.get_edge_of_flight_line());
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.get_gps_time(), 0.0);
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.gps_time, 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getXt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getYt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getZt(), 0.0);
//		LONGS_EQUAL(0, pLasReader->point.wavepacket.getSize());
//		CHECK(pLasReader->point.have_gps_time == true);
//		CHECK(pLasReader->point.have_rgb == false);
//		CHECK(pLasReader->point.have_nir == false);
//		CHECK(pLasReader->point.have_wavepacket == true);
//
//		// read point 1
//		CHECK(pLasReader->read_point());
//		LONGS_EQUAL(96730291, pLasReader->point.X);
//		LONGS_EQUAL(96730291, pLasReader->point.get_X());
//		LONGS_EQUAL(143876843, pLasReader->point.Y);
//		LONGS_EQUAL(143876843, pLasReader->point.get_Y());
//		LONGS_EQUAL(14383, pLasReader->point.Z);
//		LONGS_EQUAL(14383, pLasReader->point.get_Z());
//		DOUBLES_EQUAL(967302.91, pLasReader->point.get_x(), 1E-9);
//		DOUBLES_EQUAL(1438768.43, pLasReader->point.get_y(), 1E-9);
//		DOUBLES_EQUAL(143.83, pLasReader->point.get_z(), 1E-9);
//		LONGS_EQUAL(9, pLasReader->point.intensity);
//		LONGS_EQUAL(9, pLasReader->point.get_intensity());
//		LONGS_EQUAL(1, pLasReader->point.return_number);
//		LONGS_EQUAL(1, pLasReader->point.get_return_number());
//		LONGS_EQUAL(3, pLasReader->point.number_of_returns);
//		LONGS_EQUAL(3, pLasReader->point.get_number_of_returns());
//		LONGS_EQUAL(1, pLasReader->point.classification);
//		LONGS_EQUAL(1, pLasReader->point.get_classification());
//		LONGS_EQUAL(11, pLasReader->point.scan_angle_rank);
//		LONGS_EQUAL(11, pLasReader->point.get_scan_angle_rank());
//		LONGS_EQUAL(1, pLasReader->point.scan_direction_flag);
//		LONGS_EQUAL(1, pLasReader->point.get_scan_direction_flag());
//		LONGS_EQUAL(0, pLasReader->point.edge_of_flight_line);
//		LONGS_EQUAL(0, pLasReader->point.get_edge_of_flight_line());
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.get_gps_time(), 0.0);
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.gps_time, 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getXt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getYt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getZt(), 0.0);
//		LONGS_EQUAL(0, pLasReader->point.wavepacket.getSize());
//		CHECK(pLasReader->point.have_gps_time == true);
//		CHECK(pLasReader->point.have_rgb == false);
//		CHECK(pLasReader->point.have_nir == false);
//		CHECK(pLasReader->point.have_wavepacket == true);
//
//		// read last point
//		pLasReader->seek(114199);
//		CHECK(pLasReader->read_point());
//		LONGS_EQUAL(96687866, pLasReader->point.X);
//		LONGS_EQUAL(96687866, pLasReader->point.get_X());
//		LONGS_EQUAL(143853377, pLasReader->point.Y);
//		LONGS_EQUAL(143853377, pLasReader->point.get_Y());
//		LONGS_EQUAL(7881, pLasReader->point.Z);
//		LONGS_EQUAL(7881, pLasReader->point.get_Z());
//		DOUBLES_EQUAL(966878.66, pLasReader->point.get_x(), 1E-12);
//		DOUBLES_EQUAL(1438533.77, pLasReader->point.get_y(), 1E-12);
//		DOUBLES_EQUAL(78.81, pLasReader->point.get_z(), 1E-12);
//		LONGS_EQUAL(15, pLasReader->point.intensity);
//		LONGS_EQUAL(15, pLasReader->point.get_intensity());
//		LONGS_EQUAL(1, pLasReader->point.return_number);
//		LONGS_EQUAL(1, pLasReader->point.get_return_number());
//		LONGS_EQUAL(1, pLasReader->point.number_of_returns);
//		LONGS_EQUAL(1, pLasReader->point.get_number_of_returns());
//		LONGS_EQUAL(2, pLasReader->point.classification);
//		LONGS_EQUAL(2, pLasReader->point.get_classification());
//		LONGS_EQUAL(-10, pLasReader->point.scan_angle_rank);
//		LONGS_EQUAL(-10, pLasReader->point.get_scan_angle_rank());
//		LONGS_EQUAL(1, pLasReader->point.scan_direction_flag);
//		LONGS_EQUAL(1, pLasReader->point.get_scan_direction_flag());
//		LONGS_EQUAL(0, pLasReader->point.edge_of_flight_line);
//		LONGS_EQUAL(0, pLasReader->point.get_edge_of_flight_line());
//		DOUBLES_EQUAL(89184.181665349504, pLasReader->point.get_gps_time(), 0.0);
//		DOUBLES_EQUAL(89184.181665349504, pLasReader->point.gps_time, 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getXt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getYt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getZt(), 0.0);
//		LONGS_EQUAL(0, pLasReader->point.wavepacket.getSize());
//		CHECK(pLasReader->point.have_gps_time == true);
//		CHECK(pLasReader->point.have_rgb == false);
//		CHECK(pLasReader->point.have_nir == false);
//		CHECK(pLasReader->point.have_wavepacket == true);
//
//		pLasReader->close();
//		delete pLasReader;
//	}
//}
//
////----------------------------------------------------------------------------
//TEST(LasFile, Read_Test_13_5)
//{
//	// Test reading a .laz file, version 1.3, point format 5
//	// 
//	// Pretty good test, but VLR data not covered and points only read a couple.
//	//
//
//	{
//		XString fileName = XString::CombinePath(gs_DataPath, "points-1.3_5.laz");
//
//		LASreadOpener lasreadopener;
//		lasreadopener.set_file_name(fileName.c_str());
//
//		LASreader* pLasReader = lasreadopener.open();
//		CHECK(pLasReader != nullptr);
//
//		CHECK(pLasReader->header.is_compressed() == true);
//
//		LONGS_EQUAL(1, pLasReader->header.version_major);
//		LONGS_EQUAL(3, pLasReader->header.version_minor);
//		LONGS_EQUAL(5, pLasReader->header.point_data_format);
//
//		CHECK(XString(pLasReader->header.file_signature).Compare("LASF"));
//		LONGS_EQUAL(0, pLasReader->header.file_source_ID);
//		LONGS_EQUAL(0, pLasReader->header.global_encoding);
//		CHECK(XString(pLasReader->header.system_identifier).Compare("LAStools (c) by rapidlasso GmbH"));
//		CHECK(XString(pLasReader->header.generating_software).Compare("las2las (version 180706)"));
//		LONGS_EQUAL(53, pLasReader->header.file_creation_day);
//		LONGS_EQUAL(2019, pLasReader->header.file_creation_year);
//		LONGS_EQUAL(235, pLasReader->header.header_size);
//		LONGS_EQUAL(1192, pLasReader->header.offset_to_point_data);
//		LONGS_EQUAL(3, pLasReader->header.number_of_variable_length_records);
//		LONGS_EQUAL(63, pLasReader->header.point_data_record_length);
//		LONGS_EQUAL(114200, pLasReader->header.number_of_point_records);
//		LONGS_EQUAL(93148, pLasReader->header.number_of_points_by_return[0]);
//		LONGS_EQUAL(18578, pLasReader->header.number_of_points_by_return[1]);
//		LONGS_EQUAL(2383, pLasReader->header.number_of_points_by_return[2]);
//		LONGS_EQUAL(91, pLasReader->header.number_of_points_by_return[3]);
//		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[4]);
//
//		DOUBLES_EQUAL(966876.73999999999, pLasReader->header.min_x, 0.0);
//		DOUBLES_EQUAL(967303.01000000001, pLasReader->header.max_x, 0.0);
//		DOUBLES_EQUAL(1438530.4600000000, pLasReader->header.min_y, 1E-9);
//		DOUBLES_EQUAL(1438949.9900000000, pLasReader->header.max_y, 1E-9);
//		DOUBLES_EQUAL(78.079999999999998, pLasReader->header.min_z, 1E-9);
//		DOUBLES_EQUAL(223.20000000000002, pLasReader->header.max_z, 1E-9);
//
//		DOUBLES_EQUAL(0.01, pLasReader->header.x_scale_factor, 0.0);
//		DOUBLES_EQUAL(0.01, pLasReader->header.y_scale_factor, 0.0);
//		DOUBLES_EQUAL(0.01, pLasReader->header.z_scale_factor, 0.0);
//
//		DOUBLES_EQUAL(0.0, pLasReader->header.x_offset, 0.0);
//		DOUBLES_EQUAL(0.0, pLasReader->header.y_offset, 0.0);
//		DOUBLES_EQUAL(0.0, pLasReader->header.z_offset, 0.0);
//
//
//		// Variable length records
//
//		// vlr record 1/3
//		CHECK(XString(pLasReader->header.vlrs[0].user_id).Compare("LASF_Projection"));
//		CHECK(XString(pLasReader->header.vlrs[0].description).Compare("GeoTIFF GeoKeyDirectoryTag"));
//		LONGS_EQUAL(34735, pLasReader->header.vlrs[0].record_id);
//		LONGS_EQUAL(64, pLasReader->header.vlrs[0].record_length_after_header);
//		//TODO:
//		//need to parse out and verify "vlrs[].data"
//
//		// vlr record 2/3
//		CHECK(XString(pLasReader->header.vlrs[1].user_id).Compare("LASF_Projection"));
//		CHECK(XString(pLasReader->header.vlrs[1].description).Compare("GeoTIFF GeoAsciiParamsTag"));
//		LONGS_EQUAL(34737, pLasReader->header.vlrs[1].record_id);
//		LONGS_EQUAL(34, pLasReader->header.vlrs[1].record_length_after_header);
//
//		// vlr record 3/3
//		CHECK(XString(pLasReader->header.vlrs[1].user_id).Compare("LASF_Projection"));
//		CHECK(XString(pLasReader->header.vlrs[1].description).Compare("GeoTIFF GeoAsciiParamsTag"));
//		LONGS_EQUAL(34737, pLasReader->header.vlrs[1].record_id);
//		LONGS_EQUAL(34, pLasReader->header.vlrs[1].record_length_after_header);
//
//
//		// read point 0
//		CHECK(pLasReader->read_point());
//		LONGS_EQUAL(96730297, pLasReader->point.X);
//		LONGS_EQUAL(96730297, pLasReader->point.get_X());
//		LONGS_EQUAL(143876416, pLasReader->point.Y);
//		LONGS_EQUAL(143876416, pLasReader->point.get_Y());
//		LONGS_EQUAL(12451, pLasReader->point.Z);
//		LONGS_EQUAL(12451, pLasReader->point.get_Z());
//		DOUBLES_EQUAL(967302.97, pLasReader->point.get_x(), 1E-9);
//		DOUBLES_EQUAL(1438764.16, pLasReader->point.get_y(), 1E-9);
//		DOUBLES_EQUAL(124.51, pLasReader->point.get_z(), 1E-9);
//		LONGS_EQUAL(54, pLasReader->point.intensity);
//		LONGS_EQUAL(54, pLasReader->point.get_intensity());
//		LONGS_EQUAL(2, pLasReader->point.return_number);
//		LONGS_EQUAL(2, pLasReader->point.get_return_number());
//		LONGS_EQUAL(3, pLasReader->point.number_of_returns);
//		LONGS_EQUAL(3, pLasReader->point.get_number_of_returns());
//		LONGS_EQUAL(1, pLasReader->point.classification);
//		LONGS_EQUAL(1, pLasReader->point.get_classification());
//		LONGS_EQUAL(11, pLasReader->point.scan_angle_rank);
//		LONGS_EQUAL(11, pLasReader->point.get_scan_angle_rank());
//		LONGS_EQUAL(1, pLasReader->point.scan_direction_flag);
//		LONGS_EQUAL(1, pLasReader->point.get_scan_direction_flag());
//		LONGS_EQUAL(0, pLasReader->point.edge_of_flight_line);
//		LONGS_EQUAL(0, pLasReader->point.get_edge_of_flight_line());
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.get_gps_time(), 0.0);
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.gps_time, 0.0);
//		LONGS_EQUAL(0, pLasReader->point.rgb[0]);
//		LONGS_EQUAL(0, pLasReader->point.rgb[1]);
//		LONGS_EQUAL(0, pLasReader->point.rgb[2]);
//		LONGS_EQUAL(0, pLasReader->point.rgb[3]);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getXt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getYt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getZt(), 0.0);
//		LONGS_EQUAL(0, pLasReader->point.wavepacket.getSize());
//		CHECK(pLasReader->point.have_gps_time == true);
//		CHECK(pLasReader->point.have_rgb == true);
//		CHECK(pLasReader->point.have_nir == false);
//		CHECK(pLasReader->point.have_wavepacket == true);
//
//		// read point 1
//		CHECK(pLasReader->read_point());
//		LONGS_EQUAL(96730291, pLasReader->point.X);
//		LONGS_EQUAL(96730291, pLasReader->point.get_X());
//		LONGS_EQUAL(143876843, pLasReader->point.Y);
//		LONGS_EQUAL(143876843, pLasReader->point.get_Y());
//		LONGS_EQUAL(14383, pLasReader->point.Z);
//		LONGS_EQUAL(14383, pLasReader->point.get_Z());
//		DOUBLES_EQUAL(967302.91, pLasReader->point.get_x(), 1E-9);
//		DOUBLES_EQUAL(1438768.43, pLasReader->point.get_y(), 1E-9);
//		DOUBLES_EQUAL(143.83, pLasReader->point.get_z(), 1E-9);
//		LONGS_EQUAL(9, pLasReader->point.intensity);
//		LONGS_EQUAL(9, pLasReader->point.get_intensity());
//		LONGS_EQUAL(1, pLasReader->point.return_number);
//		LONGS_EQUAL(1, pLasReader->point.get_return_number());
//		LONGS_EQUAL(3, pLasReader->point.number_of_returns);
//		LONGS_EQUAL(3, pLasReader->point.get_number_of_returns());
//		LONGS_EQUAL(1, pLasReader->point.classification);
//		LONGS_EQUAL(1, pLasReader->point.get_classification());
//		LONGS_EQUAL(11, pLasReader->point.scan_angle_rank);
//		LONGS_EQUAL(11, pLasReader->point.get_scan_angle_rank());
//		LONGS_EQUAL(1, pLasReader->point.scan_direction_flag);
//		LONGS_EQUAL(1, pLasReader->point.get_scan_direction_flag());
//		LONGS_EQUAL(0, pLasReader->point.edge_of_flight_line);
//		LONGS_EQUAL(0, pLasReader->point.get_edge_of_flight_line());
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.get_gps_time(), 0.0);
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.gps_time, 0.0);
//		LONGS_EQUAL(0, pLasReader->point.rgb[0]);
//		LONGS_EQUAL(0, pLasReader->point.rgb[1]);
//		LONGS_EQUAL(0, pLasReader->point.rgb[2]);
//		LONGS_EQUAL(0, pLasReader->point.rgb[3]);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getXt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getYt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getZt(), 0.0);
//		LONGS_EQUAL(0, pLasReader->point.wavepacket.getSize());
//		CHECK(pLasReader->point.have_gps_time == true);
//		CHECK(pLasReader->point.have_rgb == true);
//		CHECK(pLasReader->point.have_nir == false);
//		CHECK(pLasReader->point.have_wavepacket == true);
//
//		// read last point
//		pLasReader->seek(114199);
//		CHECK(pLasReader->read_point());
//		LONGS_EQUAL(96687866, pLasReader->point.X);
//		LONGS_EQUAL(96687866, pLasReader->point.get_X());
//		LONGS_EQUAL(143853377, pLasReader->point.Y);
//		LONGS_EQUAL(143853377, pLasReader->point.get_Y());
//		LONGS_EQUAL(7881, pLasReader->point.Z);
//		LONGS_EQUAL(7881, pLasReader->point.get_Z());
//		DOUBLES_EQUAL(966878.66, pLasReader->point.get_x(), 1E-12);
//		DOUBLES_EQUAL(1438533.77, pLasReader->point.get_y(), 1E-12);
//		DOUBLES_EQUAL(78.81, pLasReader->point.get_z(), 1E-12);
//		LONGS_EQUAL(15, pLasReader->point.intensity);
//		LONGS_EQUAL(15, pLasReader->point.get_intensity());
//		LONGS_EQUAL(1, pLasReader->point.return_number);
//		LONGS_EQUAL(1, pLasReader->point.get_return_number());
//		LONGS_EQUAL(1, pLasReader->point.number_of_returns);
//		LONGS_EQUAL(1, pLasReader->point.get_number_of_returns());
//		LONGS_EQUAL(2, pLasReader->point.classification);
//		LONGS_EQUAL(2, pLasReader->point.get_classification());
//		LONGS_EQUAL(-10, pLasReader->point.scan_angle_rank);
//		LONGS_EQUAL(-10, pLasReader->point.get_scan_angle_rank());
//		LONGS_EQUAL(1, pLasReader->point.scan_direction_flag);
//		LONGS_EQUAL(1, pLasReader->point.get_scan_direction_flag());
//		LONGS_EQUAL(0, pLasReader->point.edge_of_flight_line);
//		LONGS_EQUAL(0, pLasReader->point.get_edge_of_flight_line());
//		DOUBLES_EQUAL(89184.181665349504, pLasReader->point.get_gps_time(), 0.0);
//		DOUBLES_EQUAL(89184.181665349504, pLasReader->point.gps_time, 0.0);
//		LONGS_EQUAL(0, pLasReader->point.rgb[0]);
//		LONGS_EQUAL(0, pLasReader->point.rgb[1]);
//		LONGS_EQUAL(0, pLasReader->point.rgb[2]);
//		LONGS_EQUAL(0, pLasReader->point.rgb[3]);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getXt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getYt(), 0.0);
//		DOUBLES_EQUAL(0, pLasReader->point.wavepacket.getZt(), 0.0);
//		LONGS_EQUAL(0, pLasReader->point.wavepacket.getSize());
//		CHECK(pLasReader->point.have_gps_time == true);
//		CHECK(pLasReader->point.have_rgb == true);
//		CHECK(pLasReader->point.have_nir == false);
//		CHECK(pLasReader->point.have_wavepacket == true);
//
//		pLasReader->close();
//		delete pLasReader;
//	}
//}
//
////----------------------------------------------------------------------------
//TEST(LasFile, Read_Test_14_6)
//{
//	// Test reading a .laz file, version 1.4, point format 6
//	// 
//	// Pretty good test, but VLR data not covered and points only read a couple.
//	//
//
//	{
//		XString fileName = XString::CombinePath(gs_DataPath, "points-1.4_6.laz");
//
//		LASreadOpener lasreadopener;
//		lasreadopener.set_file_name(fileName.c_str());
//
//		LASreader* pLasReader = lasreadopener.open();
//		CHECK(pLasReader != nullptr);
//
//		CHECK(pLasReader->header.is_compressed() == true);
//
//		CHECK(XString(pLasReader->header.file_signature).Compare("LASF"));
//		LONGS_EQUAL(0, pLasReader->header.file_source_ID);
//		LONGS_EQUAL(0, pLasReader->header.global_encoding);
//		LONGS_EQUAL(1, pLasReader->header.version_major);
//		LONGS_EQUAL(4, pLasReader->header.version_minor);
//		CHECK(XString(pLasReader->header.system_identifier).Compare("LAStools (c) by rapidlasso GmbH"));
//		CHECK(XString(pLasReader->header.generating_software).Compare("las2las (version 180706)"));
//		LONGS_EQUAL(53, pLasReader->header.file_creation_day);
//		LONGS_EQUAL(2019, pLasReader->header.file_creation_year);
//		LONGS_EQUAL(375, pLasReader->header.header_size);
//		LONGS_EQUAL(1332, pLasReader->header.offset_to_point_data);
//		LONGS_EQUAL(3, pLasReader->header.number_of_variable_length_records);
//		LONGS_EQUAL(6, pLasReader->header.point_data_format);
//		LONGS_EQUAL(30, pLasReader->header.point_data_record_length);
//		LONGS_EQUAL(0, pLasReader->header.number_of_point_records);
//		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[0]);
//		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[1]);
//		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[2]);
//		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[3]);
//		LONGS_EQUAL(0, pLasReader->header.number_of_points_by_return[4]);
//		LONGS_EQUAL(114200, static_cast<long>(pLasReader->header.extended_number_of_point_records));
//		LONGS_EQUAL(93148, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[0]));
//		LONGS_EQUAL(18578, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[1]));
//		LONGS_EQUAL(2383, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[2]));
//		LONGS_EQUAL(91, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[3]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[4]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[5]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[6]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[7]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[8]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[9]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[10]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[11]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[12]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[13]));
//		LONGS_EQUAL(0, static_cast<long>(pLasReader->header.extended_number_of_points_by_return[14]));
//
//		DOUBLES_EQUAL(966876.73999999999, pLasReader->header.min_x, 0.0);
//		DOUBLES_EQUAL(967303.01000000001, pLasReader->header.max_x, 0.0);
//		DOUBLES_EQUAL(1438530.4600000000, pLasReader->header.min_y, 1E-9);
//		DOUBLES_EQUAL(1438949.9900000000, pLasReader->header.max_y, 1E-9);
//		DOUBLES_EQUAL(78.079999999999998, pLasReader->header.min_z, 1E-9);
//		DOUBLES_EQUAL(223.20000000000002, pLasReader->header.max_z, 1E-9);
//
//		DOUBLES_EQUAL(0.01, pLasReader->header.x_scale_factor, 0.0);
//		DOUBLES_EQUAL(0.01, pLasReader->header.y_scale_factor, 0.0);
//		DOUBLES_EQUAL(0.01, pLasReader->header.z_scale_factor, 0.0);
//
//		DOUBLES_EQUAL(0.0, pLasReader->header.x_offset, 0.0);
//		DOUBLES_EQUAL(0.0, pLasReader->header.y_offset, 0.0);
//		DOUBLES_EQUAL(0.0, pLasReader->header.z_offset, 0.0);
//
//
//		// Variable length records
//
//		// vlr record 1/3
//		CHECK(XString(pLasReader->header.vlrs[0].user_id).Compare("LASF_Projection"));
//		CHECK(XString(pLasReader->header.vlrs[0].description).Compare("GeoTIFF GeoKeyDirectoryTag"));
//		LONGS_EQUAL(34735, pLasReader->header.vlrs[0].record_id);
//		LONGS_EQUAL(64, pLasReader->header.vlrs[0].record_length_after_header);
//		//TODO:
//		//need to parse out and verify "vlrs[].data"
//
//		// vlr record 2/3
//		CHECK(XString(pLasReader->header.vlrs[1].user_id).Compare("LASF_Projection"));
//		CHECK(XString(pLasReader->header.vlrs[1].description).Compare("GeoTIFF GeoAsciiParamsTag"));
//		LONGS_EQUAL(34737, pLasReader->header.vlrs[1].record_id);
//		LONGS_EQUAL(34, pLasReader->header.vlrs[1].record_length_after_header);
//
//		// vlr record 3/3
//		CHECK(XString(pLasReader->header.vlrs[1].user_id).Compare("LASF_Projection"));
//		CHECK(XString(pLasReader->header.vlrs[1].description).Compare("GeoTIFF GeoAsciiParamsTag"));
//		LONGS_EQUAL(34737, pLasReader->header.vlrs[1].record_id);
//		LONGS_EQUAL(34, pLasReader->header.vlrs[1].record_length_after_header);
//
//
//		// read point 0
//		CHECK(pLasReader->read_point());
//		LONGS_EQUAL(96730297, pLasReader->point.X);
//		LONGS_EQUAL(96730297, pLasReader->point.get_X());
//		LONGS_EQUAL(143876416, pLasReader->point.Y);
//		LONGS_EQUAL(143876416, pLasReader->point.get_Y());
//		LONGS_EQUAL(12451, pLasReader->point.Z);
//		LONGS_EQUAL(12451, pLasReader->point.get_Z());
//		DOUBLES_EQUAL(967302.97, pLasReader->point.get_x(), 1E-9);
//		DOUBLES_EQUAL(1438764.16, pLasReader->point.get_y(), 1E-9);
//		DOUBLES_EQUAL(124.51, pLasReader->point.get_z(), 1E-9);
//		LONGS_EQUAL(54, pLasReader->point.intensity);
//		LONGS_EQUAL(54, pLasReader->point.get_intensity());
//		LONGS_EQUAL(2, pLasReader->point.return_number);
//		LONGS_EQUAL(2, pLasReader->point.get_return_number());
//		LONGS_EQUAL(3, pLasReader->point.number_of_returns);
//		LONGS_EQUAL(3, pLasReader->point.get_number_of_returns());
//		LONGS_EQUAL(1, pLasReader->point.classification);
//		LONGS_EQUAL(1, pLasReader->point.get_classification());
//		LONGS_EQUAL(11, pLasReader->point.scan_angle_rank);
//		LONGS_EQUAL(11, pLasReader->point.get_scan_angle_rank());
//		LONGS_EQUAL(1, pLasReader->point.scan_direction_flag);
//		LONGS_EQUAL(1, pLasReader->point.get_scan_direction_flag());
//		LONGS_EQUAL(0, pLasReader->point.edge_of_flight_line);
//		LONGS_EQUAL(0, pLasReader->point.get_edge_of_flight_line());
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.get_gps_time(), 0.0);
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.gps_time, 0.0);
//		CHECK(pLasReader->point.have_gps_time == true);
//		CHECK(pLasReader->point.have_rgb == false);
//		CHECK(pLasReader->point.have_nir == false);
//		CHECK(pLasReader->point.have_wavepacket == false);
//
//		// read point 1
//		CHECK(pLasReader->read_point());
//		LONGS_EQUAL(96730291, pLasReader->point.X);
//		LONGS_EQUAL(96730291, pLasReader->point.get_X());
//		LONGS_EQUAL(143876843, pLasReader->point.Y);
//		LONGS_EQUAL(143876843, pLasReader->point.get_Y());
//		LONGS_EQUAL(14383, pLasReader->point.Z);
//		LONGS_EQUAL(14383, pLasReader->point.get_Z());
//		DOUBLES_EQUAL(967302.91, pLasReader->point.get_x(), 1E-9);
//		DOUBLES_EQUAL(1438768.43, pLasReader->point.get_y(), 1E-9);
//		DOUBLES_EQUAL(143.83, pLasReader->point.get_z(), 1E-9);
//		LONGS_EQUAL(9, pLasReader->point.intensity);
//		LONGS_EQUAL(9, pLasReader->point.get_intensity());
//		LONGS_EQUAL(1, pLasReader->point.return_number);
//		LONGS_EQUAL(1, pLasReader->point.get_return_number());
//		LONGS_EQUAL(3, pLasReader->point.number_of_returns);
//		LONGS_EQUAL(3, pLasReader->point.get_number_of_returns());
//		LONGS_EQUAL(1, pLasReader->point.classification);
//		LONGS_EQUAL(1, pLasReader->point.get_classification());
//		LONGS_EQUAL(11, pLasReader->point.scan_angle_rank);
//		LONGS_EQUAL(11, pLasReader->point.get_scan_angle_rank());
//		LONGS_EQUAL(1, pLasReader->point.scan_direction_flag);
//		LONGS_EQUAL(1, pLasReader->point.get_scan_direction_flag());
//		LONGS_EQUAL(0, pLasReader->point.edge_of_flight_line);
//		LONGS_EQUAL(0, pLasReader->point.get_edge_of_flight_line());
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.get_gps_time(), 0.0);
//		DOUBLES_EQUAL(87603.905174328858, pLasReader->point.gps_time, 0.0);
//		CHECK(pLasReader->point.have_gps_time == true);
//		CHECK(pLasReader->point.have_rgb == false);
//		CHECK(pLasReader->point.have_nir == false);
//		CHECK(pLasReader->point.have_wavepacket == false);
//
//		// read last point
//		pLasReader->seek(114199);
//		CHECK(pLasReader->read_point());
//		LONGS_EQUAL(96687866, pLasReader->point.X);
//		LONGS_EQUAL(96687866, pLasReader->point.get_X());
//		LONGS_EQUAL(143853377, pLasReader->point.Y);
//		LONGS_EQUAL(143853377, pLasReader->point.get_Y());
//		LONGS_EQUAL(7881, pLasReader->point.Z);
//		LONGS_EQUAL(7881, pLasReader->point.get_Z());
//		DOUBLES_EQUAL(966878.66, pLasReader->point.get_x(), 1E-12);
//		DOUBLES_EQUAL(1438533.77, pLasReader->point.get_y(), 1E-12);
//		DOUBLES_EQUAL(78.81, pLasReader->point.get_z(), 1E-12);
//		LONGS_EQUAL(15, pLasReader->point.intensity);
//		LONGS_EQUAL(15, pLasReader->point.get_intensity());
//		LONGS_EQUAL(1, pLasReader->point.return_number);
//		LONGS_EQUAL(1, pLasReader->point.get_return_number());
//		LONGS_EQUAL(1, pLasReader->point.number_of_returns);
//		LONGS_EQUAL(1, pLasReader->point.get_number_of_returns());
//		LONGS_EQUAL(2, pLasReader->point.classification);
//		LONGS_EQUAL(2, pLasReader->point.get_classification());
//		LONGS_EQUAL(-10, pLasReader->point.scan_angle_rank);
//		LONGS_EQUAL(-10, pLasReader->point.get_scan_angle_rank());
//		LONGS_EQUAL(1, pLasReader->point.scan_direction_flag);
//		LONGS_EQUAL(1, pLasReader->point.get_scan_direction_flag());
//		LONGS_EQUAL(0, pLasReader->point.edge_of_flight_line);
//		LONGS_EQUAL(0, pLasReader->point.get_edge_of_flight_line());
//		DOUBLES_EQUAL(89184.181665349504, pLasReader->point.get_gps_time(), 0.0);
//		DOUBLES_EQUAL(89184.181665349504, pLasReader->point.gps_time, 0.0);
//		CHECK(pLasReader->point.have_gps_time == true);
//		CHECK(pLasReader->point.have_rgb == false);
//		CHECK(pLasReader->point.have_nir == false);
//		CHECK(pLasReader->point.have_wavepacket == false);
//
//		pLasReader->close();
//		delete pLasReader;
//	}
//}

//----------------------------------------------------------------------------
TEST(LasFile, DataSize)
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
