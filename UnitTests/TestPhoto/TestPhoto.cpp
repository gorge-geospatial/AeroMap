// TestPhoto.cpp
//
// CppUnitLite test harness for Photo class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CppUnitLite/TestHarness.h"
#include "../Common/UnitTest.h"			// unit test helpers
#include "Reconstruction.h"
#include "Photo.h"

std::vector<Photo*> LoadImageList();

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
TEST(Photo, rgb_jpeg)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "rgb_image.jpg");

		Photo photo(file_name);

		LONGS_EQUAL(4896, photo.GetWidth());
		LONGS_EQUAL(3672, photo.GetHeight());
		LONGS_EQUAL(4896, photo.GetExifWidth());
		LONGS_EQUAL(3672, photo.GetExifHeight());

		CHECK(photo.GetMake() == "SONY");
		CHECK(photo.GetModel() == "DSC-WX220");

		CHECK(photo.GetBandName() == "RGB");
		CHECK(photo.GetBandIndex() == 0);

		CHECK(photo.is_rgb() == true);
		CHECK(photo.is_thermal() == false);

		//XString GetCaptureUUID();

		//int GetOrientation();
		DOUBLES_EQUAL(3.3, photo.GetFNumber(), 0.0);
		DOUBLES_EQUAL(1.0 / 250.0, photo.GetExposureTime(), 0.0);
		LONGS_EQUAL(100, photo.GetISOSpeed());

		DOUBLES_EQUAL(41.303821555555551, photo.GetLatitude(), 0.0);
		DOUBLES_EQUAL(-81.750466444444442, photo.GetLongitude(), 0.0);
		DOUBLES_EQUAL(346.31500, photo.GetAltitude(), 0.0);

		DOUBLES_EQUAL(0.85, photo.GetFocalRatio(), 0.0);
		LONGS_EQUAL(1467197224, (long)photo.GetEpoch());
		CHECK(photo.GetCaptureDate() == "2016:06:29 10:47:04");

		CHECK(photo.GetCameraString(false) == "sony dsc-wx220 4896 3672 brown 0.8500");
		CHECK(photo.GetCameraString(true) == "v2 sony dsc-wx220 4896 3672 brown 0.8500");

		CHECK(photo.GetFileName() == file_name);
	}
}

//----------------------------------------------------------------------------
TEST(Photo, thermal_jpeg)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "test_thermal.jpg");

		Photo photo(file_name);

		LONGS_EQUAL(640, photo.GetWidth());
		LONGS_EQUAL(512, photo.GetHeight());
		LONGS_EQUAL(640, photo.GetExifWidth());
		LONGS_EQUAL(512, photo.GetExifHeight());

		CHECK(photo.GetMake() == "DJI");
		CHECK(photo.GetModel() == "FLIR");

		CHECK(photo.GetBandName() == "LWIR");
		CHECK(photo.GetBandIndex() == 0);

		CHECK(photo.is_thermal() == true);
		CHECK(photo.is_rgb() == false);

		//XString GetCaptureUUID();

		////int GetOrientation();
		DOUBLES_EQUAL(1.4, photo.GetFNumber(), 0.0);
		//DOUBLES_EQUAL(1.0 / 250.0, photo.GetExposureTime(), 0.0);
		//LONGS_EQUAL(100, photo.GetISOSpeed());

		DOUBLES_EQUAL(46.517531833333329, photo.GetLatitude(), 0.0);
		DOUBLES_EQUAL(6.5630358333333332, photo.GetLongitude(), 0.0);
		DOUBLES_EQUAL(318.30, photo.GetAltitude(), 0.0);

		//DOUBLES_EQUAL(0.85, photo.GetFocalRatio(), 0.0);
		LONGS_EQUAL(1549374544, (long)photo.GetEpoch());
		CHECK(photo.GetCaptureDate() == "2019:02:05 13:49:04");

		CHECK(photo.GetCameraString(false) == "dji flir 640 512 brown 0.8500");
		CHECK(photo.GetCameraString(true) == "v2 dji flir 640 512 brown 0.8500");

		CHECK(photo.GetFileName() == file_name);
	}
}

//----------------------------------------------------------------------------
TEST(Photo, multi_tiff)
{
	// test all bands of single multispectral shot

	// all band have same capture id
	XString capture_uuid = "0900EB3C0598921532B04A6C40340E0C";

	// band 0 - green
	{
		XString file_name = XString::CombinePath(gs_DataPath, "band_green.tif");

		Photo photo(file_name);

		CHECK(photo.GetBandName() == "Green");
		CHECK(photo.GetBandIndex() == 0);
		CHECK(photo.GetCaptureUUID() == capture_uuid);

		LONGS_EQUAL(1280, photo.GetWidth());
		LONGS_EQUAL(960, photo.GetHeight());
		LONGS_EQUAL(1280, photo.GetExifWidth());
		LONGS_EQUAL(960, photo.GetExifHeight());

		CHECK(photo.GetMake() == "Parrot");
		CHECK(photo.GetModel() == "Sequoia");

		CHECK(photo.is_thermal() == false);
	}

	// band 1 - red
	{
		XString file_name = XString::CombinePath(gs_DataPath, "band_red.tif");

		Photo photo(file_name);

		CHECK(photo.GetBandName() == "Red");
		CHECK(photo.GetBandIndex() == 1);
		CHECK(photo.GetCaptureUUID() == capture_uuid);

		LONGS_EQUAL(1280, photo.GetWidth());
		LONGS_EQUAL(960, photo.GetHeight());
		LONGS_EQUAL(1280, photo.GetExifWidth());
		LONGS_EQUAL(960, photo.GetExifHeight());

		CHECK(photo.GetMake() == "Parrot");
		CHECK(photo.GetModel() == "Sequoia");

		CHECK(photo.is_thermal() == false);
	}

	// band 2 - red edge
	{
		XString file_name = XString::CombinePath(gs_DataPath, "band_reg.tif");

		Photo photo(file_name);

		CHECK(photo.GetBandName() == "Red edge");
		CHECK(photo.GetBandIndex() == 2);
		CHECK(photo.GetCaptureUUID() == capture_uuid);

		LONGS_EQUAL(1280, photo.GetWidth());
		LONGS_EQUAL(960, photo.GetHeight());
		LONGS_EQUAL(1280, photo.GetExifWidth());
		LONGS_EQUAL(960, photo.GetExifHeight());

		CHECK(photo.GetMake() == "Parrot");
		CHECK(photo.GetModel() == "Sequoia");

		CHECK(photo.is_thermal() == false);
	}

	// band 3 - nir
	{
		XString file_name = XString::CombinePath(gs_DataPath, "band_nir.tif");

		Photo photo(file_name);

		CHECK(photo.GetBandName() == "NIR");
		CHECK(photo.GetBandIndex() == 3);
		CHECK(photo.GetCaptureUUID() == capture_uuid);

		LONGS_EQUAL(1280, photo.GetWidth());
		LONGS_EQUAL(960, photo.GetHeight());
		LONGS_EQUAL(1280, photo.GetExifWidth());
		LONGS_EQUAL(960, photo.GetExifHeight());

		CHECK(photo.GetMake() == "Parrot");
		CHECK(photo.GetModel() == "Sequoia");

		CHECK(photo.is_thermal() == false);
	}
}
