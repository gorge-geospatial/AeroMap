#ifndef LASSUMMARYDATA_H
#define LASSUMMARYDATA_H

#include <ostream>
#include <array>

#include <pdal/pdal_internal.h>

#include "LasHeader.h"

namespace pdal
{
	class LasSummaryData
	{
	public:
		struct error : public std::runtime_error
		{
			error(const std::string& err) : std::runtime_error(err)
			{}
		};

		LasSummaryData();

		void addPoint(double x, double y, double z, int returnNumber);
		point_count_t getTotalNumPoints() const
		{
			return m_totalNumPoints;
		}
		BOX3D getBounds() const;
		point_count_t getReturnCount(int returnNumber) const;

		void dump(std::ostream&) const;

	private:
		double m_minX;
		double m_minY;
		double m_minZ;
		double m_maxX;
		double m_maxY;
		double m_maxZ;
		std::array<point_count_t, LasHeader::RETURN_COUNT> m_returnCounts;
		point_count_t m_totalNumPoints;

		LasSummaryData& operator=(const LasSummaryData&); // not implemented
		LasSummaryData(const LasSummaryData&); // not implemented
	};

	std::ostream& operator<<(std::ostream& ostr, const LasSummaryData&);
}

#endif // #ifndef LASSUMMARYDATA_H
