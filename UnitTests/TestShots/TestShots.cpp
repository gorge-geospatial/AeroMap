//TestShots.cpp
//
//CppUnitLite test harness for Shots class
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CppUnitLite/TestHarness.h"
#include "Shots.h"			// interface to class under test

int main(int argc, char* argv[])
{
	// output name of executable
	if (argc > 0 && argv[0])
		printf("%s\n", argv[0]);

	TestResult tr;
	TestRegistry::runAllTests(tr);

	// always pause if errors
	int failureCount = tr.GetFailureCount();
	if (failureCount > 0)
		getc(stdin);

	return failureCount;
}


//----------------------------------------------------------------------------
TEST(Shots, get_origin)
{
	{
		//vec_rot= [-1.05879908 2.804274 -0.01269243]
		//vec_trx= [135.80771914 -67.90045065 1003.33027218]
		//ret_val= [187.77645321 199.03881374 977.16395609]

		cv::Vec3d vec_rot(-1.05879908, 2.804274, -0.01269243);
		cv::Vec3d vec_trx(135.80771914, -67.90045065, 1003.33027218);

		cv::Vec3d vec_origin = Shots::get_origin(vec_rot, vec_trx);
		DOUBLES_EQUAL(vec_origin[0], 187.77645321, 1E-5);
		DOUBLES_EQUAL(vec_origin[1], 199.03881374, 1E-5);
		DOUBLES_EQUAL(vec_origin[2], 977.16395609, 1E-5);
	}
	{
		//vec_rot = [-2.99514064 -0.85395973 -0.23954058]
		//vec_trx = [-49.62502366 -95.91330815 977.64467656]
		//ret_val =	[-56.7687808 -79.69497403 978.71197801]

		cv::Vec3d vec_rot(-2.99514064, -0.85395973, -0.23954058);
		cv::Vec3d vec_trx(-49.62502366, -95.91330815, 977.64467656);

		cv::Vec3d vec_origin = Shots::get_origin(vec_rot, vec_trx);
		DOUBLES_EQUAL(vec_origin[0], -56.7687808, 1E-5);   
		DOUBLES_EQUAL(vec_origin[1], -79.69497403, 1E-5);
		DOUBLES_EQUAL(vec_origin[2], 978.71197801, 1E-5);
	}
	{
		//vec_rot = [-2.97113457 - 0.93352827 - 0.17814675]
		//vec_trx = [175.41428771 - 133.38686844  996.41462801]
		//ret_val =	[-181.98472658 - 222.6975253   979.12803629]

		cv::Vec3d vec_rot(-2.97113457, -0.93352827, -0.17814675);
		cv::Vec3d vec_trx(175.41428771, -133.38686844, 996.41462801);

		cv::Vec3d vec_origin = Shots::get_origin(vec_rot, vec_trx);
		DOUBLES_EQUAL(vec_origin[0], -181.98472658, 1E-5);
		DOUBLES_EQUAL(vec_origin[1], -222.6975253, 1E-5);
		DOUBLES_EQUAL(vec_origin[2], 979.12803629, 1E-5);
	}
}

//----------------------------------------------------------------------------
TEST(Shots, get_rotation_matrix)
{
	{
		// rotation =
		//		[-1.05879908 2.804274 -0.01269243]
		// cv2.Rodrigues(rotation)[0] =
		//		[-0.74139839 -0.6568718   0.13728753]
		//		[-0.65808761  0.75172221  0.04282998]
		//		[-0.13133589 -0.05859315 -0.98960483]

		cv::Vec3d vec_rot(-1.05879908, 2.804274, -0.01269243);
		cv::Mat mat = Shots::get_rotation_matrix(vec_rot);

		double m00 = mat.at<double>(0, 0);
		double m01 = mat.at<double>(0, 1);
		double m02 = mat.at<double>(0, 2);
		DOUBLES_EQUAL(-0.74139839, m00, 1E-8);
		DOUBLES_EQUAL(-0.6568718,  m01, 1E-8);
		DOUBLES_EQUAL( 0.13728753, m02, 1E-8);

		double m10 = mat.at<double>(1, 0);
		double m11 = mat.at<double>(1, 1);
		double m12 = mat.at<double>(1, 2);
		DOUBLES_EQUAL(-0.65808761, m10, 1E-8);
		DOUBLES_EQUAL(0.75172221, m11, 1E-8);
		DOUBLES_EQUAL(0.04282998, m12, 1E-8);

		double m20 = mat.at<double>(2, 0);
		double m21 = mat.at<double>(2, 1);
		double m22 = mat.at<double>(2, 2);
		DOUBLES_EQUAL(-0.13133589, m20, 1E-8);
		DOUBLES_EQUAL(-0.05859315, m21, 1E-8);
		DOUBLES_EQUAL(-0.98960483, m22, 1E-8);
	}
	{
		//rotation =
		//		[-2.99514064 -0.85395973 -0.23954058]
		//cv2.Rodrigues(rotation)[0] =
		//		[0.83877747  0.52558964  0.14215444]
		//		[0.52284541 -0.85037759  0.05908151]
		//		[0.15193758  0.02476855 -0.9880797]

		cv::Vec3d vec_rot(-2.99514064, -0.85395973, -0.23954058);
		cv::Mat mat = Shots::get_rotation_matrix(vec_rot);

		double m00 = mat.at<double>(0, 0);
		double m01 = mat.at<double>(0, 1);
		double m02 = mat.at<double>(0, 2);
		DOUBLES_EQUAL(0.83877747, m00, 1E-8);
		DOUBLES_EQUAL(0.52558964, m01, 1E-8);
		DOUBLES_EQUAL(0.14215444, m02, 1E-8);

		double m10 = mat.at<double>(1, 0);
		double m11 = mat.at<double>(1, 1);
		double m12 = mat.at<double>(1, 2);
		DOUBLES_EQUAL(0.52284541, m10, 1E-8);
		DOUBLES_EQUAL(-0.85037759, m11, 1E-8);
		DOUBLES_EQUAL(0.05908151, m12, 1E-8);

		double m20 = mat.at<double>(2, 0);
		double m21 = mat.at<double>(2, 1);
		double m22 = mat.at<double>(2, 2);
		DOUBLES_EQUAL(0.15193758, m20, 1E-8);
		DOUBLES_EQUAL(0.02476855, m21, 1E-8);
		DOUBLES_EQUAL(-0.9880797, m22, 1E-8);
	}
	{
		//rotation =
		//		[-2.97113457 -0.93352827 -0.17814675]
		//cv2.Rodrigues(rotation)[0] =
		//		[ 0.8143841  0.57126648  0.10214273]
		//		[0.56873548 -0.82066056  0.05528282]
		//		[0.11540574  0.01307074 -0.99323244]

		cv::Vec3d vec_rot(-2.97113457, -0.93352827, -0.17814675);
		cv::Mat mat = Shots::get_rotation_matrix(vec_rot);

		double m00 = mat.at<double>(0, 0);
		double m01 = mat.at<double>(0, 1);
		double m02 = mat.at<double>(0, 2);
		DOUBLES_EQUAL(0.8143841, m00, 1E-8);
		DOUBLES_EQUAL(0.57126648, m01, 1E-8);
		DOUBLES_EQUAL(0.10214273, m02, 1E-8);

		double m10 = mat.at<double>(1, 0);
		double m11 = mat.at<double>(1, 1);
		double m12 = mat.at<double>(1, 2);
		DOUBLES_EQUAL(0.56873548, m10, 1E-8);
		DOUBLES_EQUAL(-0.82066056, m11, 1E-8);
		DOUBLES_EQUAL(0.05528282, m12, 1E-8);

		double m20 = mat.at<double>(2, 0);
		double m21 = mat.at<double>(2, 1);
		double m22 = mat.at<double>(2, 2);
		DOUBLES_EQUAL(0.11540574, m20, 1E-8);
		DOUBLES_EQUAL(0.01307074, m21, 1E-8);
		DOUBLES_EQUAL(-0.99323244, m22, 1E-8);
	}
}
