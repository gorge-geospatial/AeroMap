/******************************************************************************
 * Copyright (c) 2018, Connor Manning
 ****************************************************************************/

#pragma once

#include <pdal/PointTable.h>

namespace pdal
{
	class VectorPointTable : public SimplePointTable
	{
	public:
		VectorPointTable(PointLayout& layout) : SimplePointTable(layout)
		{}

		virtual PointId addPoint() override
		{
			return 0;
		}
		virtual bool supportsView() const override
		{
			return true;
		}
		std::size_t numPoints() const
		{
			return m_buffer.size() / m_layoutRef.pointSize();
		}
		std::vector<char>& buffer()
		{
			return m_buffer;
		}

	protected:
		virtual char* getPoint(PointId id) override
		{
			return m_buffer.data() + pointsToBytes(id);
		}

	private:
		std::vector<char> m_buffer;
	};
}
