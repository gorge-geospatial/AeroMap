#ifndef DATETIME_H
#define DATETIME_H

#include "MarkTypes.h"

typedef UInt32 TDATE;	// make base type available to anyone that includes xface

//	Encoding of days of the week. For the US calendar the week starts on Sunday, 
//	for the ISO calendar it's on Monday.

#ifndef WEEK_STARTS_SUNDAY
enum WEEK_DAY
{
	MONDAY = 0,
	TUESDAY = 1,
	WEDNESDAY = 2,
	THURSDAY = 3,
	FRIDAY = 4,
	SATURDAY = 5,
	SUNDAY = 6,
};
#else
enum WEEK_DAY
{
	SUNDAY = 0,
	MONDAY = 1,
	TUESDAY = 2,
	WEDNESDAY = 3,
	THURSDAY = 4,
	FRIDAY = 5,
	SATURDAY = 6,
};
#endif

class CDateTime
{
public:
	TDATE m_dt;		// date/time value

public:

	// construction / destruction

	CDateTime();
	CDateTime(TDATE date);
	CDateTime(UInt16 year, UInt16 month, UInt16 day);

	// public methods
	
	void	SetDate(UInt16 year, UInt16 month, UInt16 day);	//	accept class date value as year, month, day

	TDATE	GetDate();			//	get class date value
	UInt16	GetMonth();
	UInt16	GetDay();
	UInt16	GetYear();
	UInt16	GetDayOfYear();

	static TDATE GetCurrentDate();

	// in-lines 
	
	inline UInt16 GetDayOfWeek () { return (UInt16)(m_dt % 7); }	//	Calculate the day of the week

private:

	static bool DateToYMD(TDATE d, UInt16 &year, UInt16 &month, UInt16 &day);		//	Convert a day number to year, month, day
	static bool YMDToDate(TDATE &d, UInt16 year, UInt16 month, UInt16 day);		//	Convert year, month, day to a day number

	bool CalcWeekNumber (UInt32 d, UInt16 &w);		//	Calculate the week number

	//	Construct a UInt32
	bool TimeToDate(TDATE &d, UInt16 yy, UInt16 mm, UInt16 dd, UInt16 hh, UInt16 mmm);
	bool TimeToDate(TDATE &d, TDATE date, UInt16 hh, UInt16 mmm);

	//	Extract the information from a UInt32
	bool DateToTime(TDATE d, UInt16 &yy, UInt16 &mm, UInt16 &dd, UInt16 &hh, UInt16 &mmm);
	bool DateToTime(TDATE d, TDATE &date, UInt16 &hh, UInt16 &mmm);

	//	Calculate date for first 'wday' _after_ today.
	inline UInt32 NextWeekDay (TDATE d, UInt16 wday) { return (d + 7 - wday)/ 7 * 7 + wday; }
	inline UInt16 CalcDayOfWeek (TDATE d) { return (UInt16)(d % 7); }	//	Calculate the day of the week

};

#endif // #ifndef DATETIME_H
