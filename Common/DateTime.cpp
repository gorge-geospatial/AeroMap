// DateTime.cpp
//

//	Day, month, year are passed as unsigned short values
//
//	Date representation is a unsigned long, counting from the fictional date March 1st, year 1200 Gregorian
//
//	UInt32 = minutes from the Epoch
//

#include <time.h>
#include "DateTime.h"

//	Length of 1,4,100 and 400 years, in days

const UInt32 LY_1 = 365;
const UInt32 LY_4 = 4 * LY_1 + 1;
const UInt32 LY_100 = 25 * LY_4 - 1;
const UInt32 LY_400 = 4 * LY_100 + 1;

// constant set to make a Sunday or Monday divisible by 7, 1 March 1200 would have been a Wednesday

const UInt16 EPOCH_YEAR = 1200;		// must be divisible by 400.
const UInt32 DOFFSET = WEDNESDAY; 

const UInt16 FIRST_YEAR = 1582;		// introduction of Gregorian calendar by Conc. of Nicae.
const UInt16 LAST_YEAR = 9366;		// to prevent overflow in UInt32

//	Lookup table: length of a month

static const UInt16 monthlen [12] = 
{
	31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 29
};

//	number of days before a month

static const UInt16 daysbeforemonth[13] = 
{
	0,31,61,92,122,153,184,214,245,275,306,337,366
}; 

//	lookup table from day to month

static const unsigned char day2month[366] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11
};

//	Lookup table for Metonic cycle, the year mod 19 determines the date of the
//	paschal full moon.

// M converts dates in offsets from the first of March

#define	M(mo, da)	(UInt16)(31 * (mo - 3) + (da - 1))

static const UInt16 m_cycle[19] = 
{
   M(4, 14), M(4, 3),  M(3, 23), M(4, 11), M(3, 31), M(4, 18),
   M(4, 8),  M(3, 28), M(4, 16), M(4,  5), M(3, 25), M(4, 13),
   M(4, 2),  M(3, 22), M(4, 10), M(3, 30), M(4, 17), M(4, 7),
   M(3, 27)
};

#undef M

//	UInt32 routines

// 	some rather obvious constants
const UInt32 MINUTESPERHOUR = 60;
const UInt32 HOURSPERDAY = 24;
const UInt32 MINUTESPERDAY = HOURSPERDAY * MINUTESPERHOUR;

inline bool IsLeapYear (UInt16 year)
/*
	Leap year test
*/
{
	if (year % 100 != 0)
		return (year % 4 == 0);
	else
		return (year % 400 == 0);
}

//------------------------------------------------------------------------------------------------------------------------
CDateTime::CDateTime()
/*
	default constructor
*/
{
	m_dt = 0;
}

//------------------------------------------------------------------------------------------------------------------------
CDateTime::CDateTime( TDATE date )
/*
	constructor that takes date
*/
{
	m_dt = date;
}

//------------------------------------------------------------------------------------------------------------------------
CDateTime::CDateTime( UInt16 year, UInt16 month, UInt16 day )
/*
	constructor that takes y/m/d
*/
{
	SetDate( year, month, day );
}

//------------------------------------------------------------------------------------------------------------------------
void CDateTime::SetDate( UInt16 year, UInt16 month, UInt16 day )
/*
	set class date value

	the Gregorian calendar was introduced in 1582
*/
{
	UInt32 l_dt = 0;

	if (YMDToDate( l_dt, year, month, day ))
		m_dt = l_dt;
}

//------------------------------------------------------------------------------------------------------------------------
/* static */ bool CDateTime::YMDToDate( UInt32 &d, UInt16 year, UInt16 month, UInt16 day )
/*
	convert year/month/day to integer date type

	The Gregorian calendar was introduced in 1582
*/
{
	if (year < FIRST_YEAR || year > LAST_YEAR)	// 18 March 28277 is the last representable day
		return false;

	year -= EPOCH_YEAR;

	if (month < 1 || month > 12)
		return false;

	//	switch to year starting in March
	if (month >= 3)
	{
		month -= 3;
	}
	else
	{
		year--;
		month += 9;
	};

	//	check whether day is within month
	if (day < 1 || day > monthlen[month])
		return false;

	// very special case: the leap day
	if (month == 11 && day >= 29 && !IsLeapYear(year+1))
		return false; 

	// calculate day number
	d = UInt32(365) * year + (year / 400) + (year / 4) - (year / 100) + daysbeforemonth[month] + (day-1) + DOFFSET;

	return true;
}

//------------------------------------------------------------------------------------------------------------------------
TDATE CDateTime::GetDate()
{
	return m_dt;
}

//------------------------------------------------------------------------------------------------------------------------
UInt16 CDateTime::GetDay()
/*
	Valid return values range between 1 and 31.
*/
{
	UInt16 y, m, d;
	if (!DateToYMD(m_dt, y, m, d))
		d = 0;
	return d;
}

//------------------------------------------------------------------------------------------------------------------------
UInt16 CDateTime::GetMonth()
/*
	Valid return values range between 1 and 12.
*/
{
	UInt16 y, m, d;
	if (!DateToYMD(m_dt, y, m, d))
		m = 0;
	return m;
}

//------------------------------------------------------------------------------------------------------------------------
UInt16 CDateTime::GetYear()
{
	UInt16 y, m, d;
	if (!DateToYMD(m_dt, y, m, d))
		y = 0;
	return y;
}

//------------------------------------------------------------------------------------------------------------------------
UInt16 CDateTime::GetDayOfYear()
/*
	Valid return values range between 1 and 366, where January 1 = 1.
*/
{
	UInt16 nReturn = 0;

	UInt16 year, month, day;
	if (DateToYMD(m_dt, year, month, day))
	{
		//don't index into daysbeforemonth[], it's an internal structure not consistent with std y/m/d values
		UInt16 nDaysBefore[12] = 
		{
			0,		// jan
			31,		// feb
			59,		// mar
			90,		// apr
			121,	// may
			151,	// jun
			182,	// jul
			213,	// aug
			243,	// sep
			274,	// oct
			304,	// nov
			335,	// dec
		}; 

		nReturn = nDaysBefore[month-1];		// get # of days before month

		if (IsLeapYear(year) && month > 2)		// add 1 for leap year
			nReturn++;

		nReturn += day;			// add days into month
	}

	return nReturn;
}

//------------------------------------------------------------------------------------------------------------------------
/* static */ TDATE CDateTime::GetCurrentDate()
{
	TDATE l_dt = 0;

	time_t timer;		// get system time using ansi standard c runtine calls
	time ( &timer );
	
	tm* lp_time_struct = localtime( &timer );
	
	UInt16 day   = lp_time_struct->tm_mday;
	UInt16 month = lp_time_struct->tm_mon;
	UInt16 year  = lp_time_struct->tm_year;

	year += 1900;		// year relative to 1900
	month++;			// month zero-based

	YMDToDate( l_dt, year, month, day );

	return l_dt;
}

//------------------------------------------------------------------------------------------------------------------------
/* static */ bool CDateTime::DateToYMD(UInt32 d, UInt16 &year, UInt16 &month, UInt16 &day)
{
	//	adjust for March 1st, 0, being on a Wednesday

	if (d < DOFFSET)
		return false;

	d -= DOFFSET;

	//	subtract 400-year periods
	year = 400 * (UInt16)(d / LY_400);
	d %= LY_400;

	//	check for leap day once every 400 years
	if (d == LY_400-1)
	{
		year += (EPOCH_YEAR + 400);
		month = 2;
		day = 29;
		return true;
	}

	//	subtract centuries
	year += 100 * (UInt16)(d / LY_100);
	d %= LY_100;

	//	subtract periods of four years
	year += 4 * (UInt16)(d / LY_4);
	d %= LY_4;

	// check for normal leap day
	if (d == LY_4-1)
	{
		year += (EPOCH_YEAR + 4);
		month = 2;
		day = 29;
		return true;
	}

	// subtract all full years
	year += (UInt16)(d / LY_1);
	d %= LY_1;

	//	look up month
	unsigned i = day2month[d];

	//	adjust month and year to external representation
	month = i+3;
	if (month > 12)
	{
		year++;
		month -= 12;
	}

	year += EPOCH_YEAR;

	// calculate day
	d -= daysbeforemonth[i];
	day = (UInt16)(d+1);

	return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool CDateTime::TimeToDate(UInt32 &d, UInt32 date, UInt16 hh, UInt16 mmm)
/*
	Convert DATE and time into UInt32
*/
{
	if (hh >= HOURSPERDAY || mmm >= MINUTESPERHOUR)
		return false;
	d = mmm + MINUTESPERHOUR * (hh + HOURSPERDAY * (UInt32)date);
	return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool CDateTime::TimeToDate(UInt32 &d, UInt16 yy, UInt16 mm, UInt16 dd, UInt16 hh, UInt16 /*mmm*/)
/*
	Convert DATE and time into UInt32
*/
{
	UInt32 date;
	if (!YMDToDate(date, yy, mm, dd))
		return false;
	return TimeToDate(d, date, hh, mm);
}

//------------------------------------------------------------------------------------------------------------------------
bool CDateTime::DateToTime(UInt32 d, UInt32 &date, UInt16 &hh, UInt16 &mmm)
/*
	Extract DATE and time from a UInt32
*/
{
	date = d / MINUTESPERDAY;
	UInt16 hhmm = (UInt16)(d % MINUTESPERDAY);
	hh = hhmm / (UInt16)MINUTESPERHOUR;
	mmm = hhmm % (UInt16)MINUTESPERHOUR;

	return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool CDateTime::DateToTime(UInt32 d, UInt16 &yy, UInt16 &mm, UInt16 &dd, UInt16 &hh, UInt16 &mmm)
/*
	Extract date and time from UInt32
*/
{
	UInt32 date;
	return (DateToTime(d, date, hh, mm) && DateToYMD(date, yy, mm, dd));
}

//------------------------------------------------------------------------------------------------------------------------
bool CDateTime::CalcWeekNumber (UInt32 date, UInt16 &weeknum)
/*
	Week #1 is the first week of the year that contains a working day, i.e. Jan. 2 is a Mon-Fri.
*/
{
	UInt16 yy, mm, dd;

	// use last day of this week (Sat/Sun, depending you're using US or ISO calendar)
	date += (6 - CalcDayOfWeek(date));

	// we need the current year
	if (!DateToYMD(date, yy, mm, dd))
		return false;

	for(;;)
	{
		// figure out what day Jan. 2 is.
		UInt32 d;
		if (!YMDToDate(d, yy, 1, 2))
			return false;

		UInt16 dow = CalcDayOfWeek(d);

		// Calculate the date of the start of week 1
		switch(dow)
		{
		case MONDAY: case TUESDAY: case WEDNESDAY: case THURSDAY: case FRIDAY:
			d -= dow;
			break;
		default:
			d += (7 - dow);
		};

		if (date >= d)
		{
			weeknum = (UInt16)((date - d)/7 + 1);
			return true;
		};

		// we get here if the date _precedes_ the start of week 1, i.e. it's week 52/53 of the previous year
		yy--;
	}
}
