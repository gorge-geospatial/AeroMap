// TestMulti.cpp
//
// CppUnitLite test harness for Multispectral class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CppUnitLite/TestHarness.h"
#include "../Common/UnitTest.h"			// unit test helpers
#include "Reconstruction.h"
#include "Photo.h"
#include "Multispectral.h"				// interface to class under test

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
TEST(Multi, to_8bit)
{
	{
		XString file_name = XString::CombinePath(gs_DataPath, "IMG_00_GRE.TIF");
		cv::Mat image = cv::imread(file_name.c_str(), cv::IMREAD_UNCHANGED);

		cv::Mat image_gray = Multispectral::to_8bit(image);

		//cv::imwrite("c:/users/xmark/desktop/image.png", image);
		//cv::imwrite("c:/users/xmark/desktop/image_gray.png", image_gray);
	}

	{
		XString file_name = XString::CombinePath(gs_DataPath, "rgb_image.jpg");
		cv::Mat image = cv::imread(file_name.c_str(), cv::IMREAD_UNCHANGED);

		cv::Mat image_gray = Multispectral::to_8bit(image);

		//cv::imwrite("c:/users/xmark/desktop/image.png", image);
		//cv::imwrite("c:/users/xmark/desktop/image_gray.png", image_gray);
	}
}

//----------------------------------------------------------------------------
TEST(Multi, band_maps)
{
	std::vector<Photo*> image_list = LoadImageList();
	LONGS_EQUAL(16, (int)image_list.size());

	Reconstruction rec(image_list);
	LONGS_EQUAL(4, (int)rec.m_mc.size());

	XString primary_band_name = Multispectral::get_primary_band_name(rec.m_mc, "auto");
	CHECK(primary_band_name == "Green");

	Multispectral::s2p_type s2p;
	Multispectral::p2s_type p2s;
	Multispectral::compute_band_maps(rec.m_mc, primary_band_name, s2p, p2s);

	int s2p_len = (int)s2p.size();
	LONGS_EQUAL(16, (int)s2p_len);

	// spot check that s2p is pointing to primary bands
	Photo* p = s2p[image_list[5]->GetFileName().c_str()];
	CHECK(p->GetBandName() == primary_band_name);

	int p2s_len = (int)p2s.size();
	LONGS_EQUAL(4, (int)p2s_len);

	for (auto iter : p2s)
	{
		CHECK(iter.second.size() == 3);

		// verify primary is green & non-primary is non-green

		XString s = iter.first.c_str();
		CHECK(s.EndsWithNoCase("_GRE.TIF"));

		s = iter.second[0]->GetFileName();
		CHECK(s.EndsWithNoCase("_GRE.TIF") == false);
	}
}

//----------------------------------------------------------------------------
TEST(Multi, find_features_homography)
{
	cv::Mat image_gray = cv::imread(XString::CombinePath(gs_DataPath, "IMG_00_GRE.TIF").c_str(), cv::IMREAD_UNCHANGED);
	cv::Mat align_image_gray = cv::imread(XString::CombinePath(gs_DataPath, "IMG_00_RED.TIF").c_str(), cv::IMREAD_UNCHANGED);

	image_gray = Multispectral::to_8bit(image_gray);
	align_image_gray = Multispectral::to_8bit(align_image_gray);
	
//	cv::Mat h = Multispectral::find_features_homography(image_gray, align_image_gray);
}

//----------------------------------------------------------------------------
std::vector<Photo*> LoadImageList()
{
	std::vector<Photo*> image_list;

	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_00_GRE.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_00_NIR.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_00_RED.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_00_REG.TIF")));

	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_01_GRE.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_01_NIR.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_01_RED.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_01_REG.TIF")));

	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_02_GRE.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_02_NIR.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_02_RED.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_02_REG.TIF")));

	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_03_GRE.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_03_NIR.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_03_RED.TIF")));
	image_list.push_back(new Photo(XString::CombinePath(gs_DataPath, "IMG_03_REG.TIF")));

	return image_list;
}