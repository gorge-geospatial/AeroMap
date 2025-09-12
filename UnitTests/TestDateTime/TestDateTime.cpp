// CppUnitLite test harness for CDateTime class
//
// not implemented:
//		TDATE	GetDate();			//	get class date value
//		UInt16	GetDayOfYear();
//		static TDATE GetCurrentDate();
//

#include <stdio.h>
#include <stdlib.h>

#include "CppUnitLite/TestHarness.h"	// CppUnitLite library
#include "DateTime.h"					// interface to class under test

int main(int argc, char** argv)
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
TEST(DateTime, ctor)
{
	// test constructors
	//

	{
		// default

		CDateTime dt;
		dt.SetDate(2013, 7, 16);

		CHECK(dt.GetYear() == 2013);
		CHECK(dt.GetMonth() == 7);
		CHECK(dt.GetDay() == 16);
	}

	{
		//TODO:
		//CDateTime dt(?); //  2013, 7, 16 );

		//CHECK( dt.GetYear() == 2013 );
		//CHECK( dt.GetMonth() == 7 );
		//CHECK( dt.GetDay() == 16 );
	}

	{
		// year, month, day
		CDateTime dt(2013, 7, 16);

		CHECK(dt.GetYear() == 2013);
		CHECK(dt.GetMonth() == 7);
		CHECK(dt.GetDay() == 16);
	}
}

//----------------------------------------------------------------------------
TEST(DateTime, SetDate)
{
	CDateTime dt;
	dt.SetDate(2013, 7, 16);

	CHECK(dt.GetYear() == 2013);
	CHECK(dt.GetMonth() == 7);
	CHECK(dt.GetDay() == 16);
}

//----------------------------------------------------------------------------
TEST(DateTime, DayOfWeek)
{
	{
		// non leap year

		CDateTime dt;

		dt.SetDate(2013, 7, 15);
		CHECK(dt.GetDayOfWeek() == 0);

		dt.SetDate(2013, 7, 16);
		CHECK(dt.GetDayOfWeek() == 1);

		dt.SetDate(2013, 7, 17);
		CHECK(dt.GetDayOfWeek() == 2);

		dt.SetDate(2013, 7, 18);
		CHECK(dt.GetDayOfWeek() == 3);

		dt.SetDate(2013, 7, 19);
		CHECK(dt.GetDayOfWeek() == 4);

		dt.SetDate(2013, 7, 20);
		CHECK(dt.GetDayOfWeek() == 5);

		dt.SetDate(2013, 7, 21);
		CHECK(dt.GetDayOfWeek() == 6);

		dt.SetDate(2013, 7, 22);
		CHECK(dt.GetDayOfWeek() == 0);

		dt.SetDate(2013, 7, 23);
		CHECK(dt.GetDayOfWeek() == 1);
	}
}
