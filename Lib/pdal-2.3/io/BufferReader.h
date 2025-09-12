/******************************************************************************
* Copyright (c) 2014, Howard Butler (howard@hobu.co)
****************************************************************************/

#pragma once

#include <pdal/pdal_export.h>
#include <pdal/PointView.h>
#include <pdal/Reader.h>

namespace pdal
{
	class BufferReader : public pdal::Reader
	{
	public:
		BufferReader() : Reader()
		{}
		void addView(const PointViewPtr& view)
		{
			m_views.insert(view);
		}
		std::string getName() const { return "readers.buffer"; }

	private:
		PointViewSet m_views;

		virtual PointViewSet run(PointViewPtr /*view*/)
		{
			return m_views;
		}
	};
}
