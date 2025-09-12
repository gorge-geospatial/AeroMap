/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
****************************************************************************/

#include "LasSummaryData.h"

namespace pdal
{
	LasSummaryData::LasSummaryData() :
		m_minX((std::numeric_limits<double>::max)()),
		m_minY((std::numeric_limits<double>::max)()),
		m_minZ((std::numeric_limits<double>::max)()),
		m_maxX(std::numeric_limits<double>::lowest()),
		m_maxY(std::numeric_limits<double>::lowest()),
		m_maxZ(std::numeric_limits<double>::lowest()),
		m_totalNumPoints(0)
	{
		m_returnCounts.fill(0);
	}

	void LasSummaryData::addPoint(double x, double y, double z, int returnNumber)
	{
		++m_totalNumPoints;
		m_minX = (std::min)(m_minX, x);
		m_minY = (std::min)(m_minY, y);
		m_minZ = (std::min)(m_minZ, z);
		m_maxX = (std::max)(m_maxX, x);
		m_maxY = (std::max)(m_maxY, y);
		m_maxZ = (std::max)(m_maxZ, z);

		// Returns numbers are indexed from one, but the array indexes from 0.
		returnNumber--;
		if (returnNumber >= 0 && (size_t)returnNumber < m_returnCounts.size())
			m_returnCounts[returnNumber]++;
	}


	BOX3D LasSummaryData::getBounds() const
	{
		BOX3D output(m_minX, m_minY, m_minZ, m_maxX, m_maxY, m_maxZ);
		return output;
	}

	point_count_t LasSummaryData::getReturnCount(int returnNumber) const
	{
		if (returnNumber < 0 || (size_t)returnNumber >= m_returnCounts.size())
			throw error("Point return number is out of range");
		return m_returnCounts[returnNumber];
	}

	void LasSummaryData::dump(std::ostream& str) const
	{
		str << "MinX: " << m_minX << "\n";
		str << "MinY: " << m_minY << "\n";
		str << "MinZ: " << m_minZ << "\n";
		str << "MaxX: " << m_maxX << "\n";
		str << "MaxY: " << m_maxY << "\n";
		str << "MaxZ: " << m_maxZ << "\n";

		str << "Number of returns:";
		for (size_t i = 0; i < m_returnCounts.size(); ++i)
			str << " " << m_returnCounts[i];
		str << "\n";

		str << "Total number of points: " << m_totalNumPoints << "\n";
	}

	std::ostream& operator<<(std::ostream& ostr, const LasSummaryData& data)
	{
		data.dump(ostr);
		return ostr;
	}
}
