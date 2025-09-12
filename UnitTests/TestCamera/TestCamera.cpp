// CppUnitLite test harness for Camera class
//
// Supported Tests:
//		CHECK( boolean )
//		LONGS_EQUAL( num1, num2 )
//

#include <stdio.h>
#include <stdlib.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "Camera.h"						// interface to class under test

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
TEST(Camera, General)
{
	{
		Camera camera;
		camera.SetPos(100.0, 200.0, 300.0);
		CHECK(camera.GetPos().x == 100.0);
		CHECK(camera.GetPos().y == 200.0);
		CHECK(camera.GetPos().z == 300.0);

		camera.MoveForward(10.0);
		camera.MoveLeft(20.0);
		camera.MoveUp(30.0);

		camera.MoveBackward(10.0);
		camera.MoveRight(20.0);
		camera.MoveDown(30.0);

		CHECK(camera.GetPos().x == 100.0);
		CHECK(camera.GetPos().y == 200.0);
		CHECK(camera.GetPos().z == 300.0);

		camera.Translate(10.0, 20.0, 30.0);

		CHECK(camera.GetPos().x == 110.0);
		CHECK(camera.GetPos().y == 220.0);
		CHECK(camera.GetPos().z == 330.0);
	}

	{
		Camera camera;
		camera.SetPos(VEC3(100.0, 200.0, 300.0));
		camera.SetDirUp(VEC3(0, 0, 1), VEC3(0, 1, 0));

		CHECK(camera.GetDir().x == 0.0);
		CHECK(camera.GetDir().y == 0.0);
		CHECK(camera.GetDir().z == 1.0);

		CHECK(camera.GetUp().x == 0.0);
		CHECK(camera.GetUp().y == 1.0);
		CHECK(camera.GetUp().z == 0.0);
	}
}

//----------------------------------------------------------------------------
TEST(Camera, Frustum)
{
	// Test view frustum.

	{
		// Camera right at origin looking down Y axis

		Camera camera;

		camera.SetPos(0.0, 0.0, 0.0);
		camera.SetDirUp(VEC3(0, 1, 0), VEC3(0, 0, 1));

		CHECK(camera.GetPos() == VEC3(0, 0, 0));
		CHECK(camera.GetDir() == VEC3(0, 1, 0));
		CHECK(camera.GetUp() == VEC3(0, 0, 1));

		double fov = DegreeToRadian(60.0);
		double aspect = 1.5;
		double near = 10.0;
		double far = 1000.0;
		camera.SetFrustum(fov, aspect, near, far);

		{
			VEC3 testPoint(0, 100, 0);
			bool inside = camera.PointInFrustum(testPoint);
			CHECK(inside == true);
		}

		{
			VEC3 testPoint(0, far + 10.0, 0);
			bool inside = camera.PointInFrustum(testPoint);
			CHECK(inside == false);
		}

		{
			VEC3 testPoint(0, near - 10.0, 0);
			bool inside = camera.PointInFrustum(testPoint);
			CHECK(inside == false);
		}

		{
			// just inside
			VEC3 testPoint(800.0, 980.0, 0);
			bool inside = camera.PointInFrustum(testPoint);
		}
		{
			// just outside
			VEC3 testPoint(800.0, 980.0, 0);
			bool inside = camera.PointInFrustum(testPoint);
		}
	}

	{
		// Camera at origin, rotated 45 degrees

		Camera camera;

		camera.SetPos(0.0, 0.0, 0.0);
		camera.SetDirUp(VEC3(0, 1, 0), VEC3(0, 0, 1));

		CHECK(camera.GetPos() == VEC3(0, 0, 0));
		CHECK(camera.GetDir() == VEC3(0, 1, 0));
		CHECK(camera.GetUp() == VEC3(0, 0, 1));

		double fov = DegreeToRadian(60.0);
		double aspect = 1.5;
		double near = 10.0;
		double far = 1000.0;
		camera.SetFrustum(fov, aspect, near, far);

		camera.RotateAboutZ(DegreeToRadian(45));

		{
			// d=1272
			VEC3 testPoint1(900, 900, 0);
			bool inside = camera.PointInFrustum(testPoint1);
			CHECK(inside == false);
			// d=1131
			VEC3 testPoint2(800, 800, 0);
			inside = camera.PointInFrustum(testPoint2);
			CHECK(inside == false);
			// d=990
			VEC3 testPoint3(700, 700, 0);
			inside = camera.PointInFrustum(testPoint3);
			CHECK(inside == true);
		}
	}
}

//----------------------------------------------------------------------------
TEST(Camera, Rotate)
{
	// Camera rotate methods.

	{
		// Rotate about x

		Camera camera;

		camera.SetPos(0.0, 0.0, 0.0);
		camera.SetDirUp(VEC3(0, 1, 0), VEC3(0, 0, 1));

		camera.RotateAboutX(DegreeToRadian(30.0));

		VEC3 dir = camera.GetDir();
		VEC3 up = camera.GetUp();

		DOUBLES_EQUAL(0.0,   dir.x, 0.0);
		DOUBLES_EQUAL(0.866, dir.y, 0.001);
		DOUBLES_EQUAL(0.5,   dir.z, 0.0);

		DOUBLES_EQUAL(0.0,   up.x, 0.0);
		DOUBLES_EQUAL(-0.5, up.y, 0.0);
		DOUBLES_EQUAL(0.866,   up.z, 0.001);
	}
	{
		// Rotate about y

		Camera camera;

		camera.SetPos(0.0, 0.0, 0.0);
		camera.SetDirUp(VEC3(0, 1, 0), VEC3(0, 0, 1));

		camera.RotateAboutY(DegreeToRadian(30.0));

		VEC3 dir = camera.GetDir();
		VEC3 up = camera.GetUp();

		DOUBLES_EQUAL(0.0, dir.x, 0.0);
		DOUBLES_EQUAL(1.0, dir.y, 0.0);
		DOUBLES_EQUAL(0.0, dir.z, 0.0);

		DOUBLES_EQUAL(0.5,   up.x, 0.0);
		DOUBLES_EQUAL(0.0,   up.y, 0.0);
		DOUBLES_EQUAL(0.866, up.z, 1E6);
	}
	{
		// Rotate about z

		Camera camera;

		camera.SetPos(0.0, 0.0, 0.0);
		camera.SetDirUp(VEC3(0, 1, 0), VEC3(0, 0, 1));

		camera.RotateAboutZ(DegreeToRadian(30.0));

		VEC3 dir = camera.GetDir();
		VEC3 up = camera.GetUp();

		DOUBLES_EQUAL(0.5,   dir.x, 0.0);
		DOUBLES_EQUAL(0.866, dir.y, 0.001);
		DOUBLES_EQUAL(0.0,   dir.z, 0.0);

		DOUBLES_EQUAL(0.0, up.x, 0.0);
		DOUBLES_EQUAL(0.0, up.y, 0.0);
		DOUBLES_EQUAL(1.0, up.z, 0.0);
	}
	{
		// Rotate about camera DIR

		// this test should give same results as rotate
		// about Y axis

		Camera camera;

		camera.SetPos(0.0, 0.0, 0.0);
		camera.SetDirUp(VEC3(0, 1, 0), VEC3(0, 0, 1));

		camera.Rotate(DegreeToRadian(30.0));

		VEC3 dir = camera.GetDir();
		VEC3 up = camera.GetUp();

		DOUBLES_EQUAL(0.0, dir.x, 0.0);
		DOUBLES_EQUAL(1.0, dir.y, 0.0);
		DOUBLES_EQUAL(0.0, dir.z, 0.0);

		DOUBLES_EQUAL(0.5,   up.x, 1E9);
		DOUBLES_EQUAL(0.0,   up.y, 0.0);
		DOUBLES_EQUAL(0.866, up.z, 1E6);
	}
	{
		// Rotate about camera DIR

		Camera camera;

		camera.SetPos(0.0, 0.0, 0.0);
		camera.SetDirUp(VEC3(0, 1, 0), VEC3(0, 0, 1));

		// tilt up 45 degrees
		camera.Tilt(DegreeToRadian(45.0));

		VEC3 dir = camera.GetDir();
		VEC3 up = camera.GetUp();

		DOUBLES_EQUAL(0.0,    dir.x, 0.0);
		DOUBLES_EQUAL(0.7071, dir.y, 1E4);
		DOUBLES_EQUAL(0.7071, dir.z, 1E4);

		DOUBLES_EQUAL( 0.0,    up.x, 0.0);
		DOUBLES_EQUAL(-0.7071, up.y, 1E6);
		DOUBLES_EQUAL( 0.7071, up.z, 1E6);

		// now rotate about camera dir
		camera.Rotate(DegreeToRadian(30.0));

		dir = camera.GetDir();
		up = camera.GetUp();

		// dir doesn't change
		DOUBLES_EQUAL(0.0,    dir.x, 0.0);
		DOUBLES_EQUAL(0.7071, dir.y, 1E4);
		DOUBLES_EQUAL(0.7071, dir.z, 1E4);

		DOUBLES_EQUAL(0.5,   up.x, 1E9);
		DOUBLES_EQUAL(-0.61237243569579458, up.y, 1E9);
		DOUBLES_EQUAL(0.61237243569579458, up.z, 1E9);
	}
}

//----------------------------------------------------------------------------
TEST(Camera, Tilt)
{
	// Camera Tilt() method.

	{
		// Camera right at origin looking down Y axis

		Camera camera;

		camera.SetPos(0.0, 0.0, 0.0);
		camera.SetDirUp(VEC3(0, 1, 0), VEC3(0, 0, 1));

		CHECK(camera.GetPos() == VEC3(0, 0, 0));
		CHECK(camera.GetDir() == VEC3(0, 1, 0));
		CHECK(camera.GetUp() == VEC3(0, 0, 1));

		double tilt = camera.GetTilt();
		DOUBLES_EQUAL(0.0, tilt, 0.0);

//TODO:
		//camera.Tilt(DegreeToRadian(30.0));
		//tilt = RadianToDegree(camera.GetTilt());
		//DOUBLES_EQUAL(30.0, tilt, 1E-6);
	}

	{
		// Camera right at origin looking down Y axis

		Camera camera;

		camera.SetPos(0.0, 0.0, 0.0);
		camera.SetDirUp(VEC3(0, 1, 0), VEC3(0, 0, 1));

		CHECK(camera.GetPos() == VEC3(0, 0, 0));
		CHECK(camera.GetDir() == VEC3(0, 1, 0));
		CHECK(camera.GetUp() == VEC3(0, 0, 1));

		double tilt = camera.GetTilt();
		DOUBLES_EQUAL(0.0, tilt, 0.0);

		camera.Pan(DegreeToRadian(45.0));

		//TODO:
		//		camera.Tilt(DegreeToRadian(30.0));
//		tilt = RadianToDegree(camera.GetTilt());
//		DOUBLES_EQUAL(30.0, tilt, 1E-6);
	}
}
