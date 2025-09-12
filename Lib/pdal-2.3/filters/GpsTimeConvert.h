/******************************************************************************
 * Copyright (c) 2021, Preston J. Hartzell (preston.hartzell@gmail.com)
 ****************************************************************************/

#pragma once

#include <pdal/Filter.h>

namespace pdal
{
	class GpsTimeConvert : public Filter
	{
	public:
		GpsTimeConvert() : Filter()
		{}
		std::string getName() const;

	private:
		std::string m_conversion;
		std::string m_strDate;
		std::tm m_tmDate;
		bool m_wrap;
		bool m_wrapped;

		void weekSeconds2GpsTime(PointView& view);
		void gpsTime2WeekSeconds(PointView& view);
		void gpsTime2GpsTime(PointView& view);

		std::tm gpsTime2Date(int seconds);
		int weekStartGpsSeconds(std::tm date);
		void unwrapWeekSeconds(PointView& view);
		void wrapWeekSeconds(PointView& view);

		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual void filter(PointView& view);

		GpsTimeConvert& operator=(const GpsTimeConvert&); // not implemented
		GpsTimeConvert(const GpsTimeConvert&); // not implemented
	};
}
