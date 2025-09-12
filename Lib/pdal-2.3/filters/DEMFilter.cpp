/******************************************************************************
* Copyright (c) 2017, Howard Butler (info@hobu.co)
****************************************************************************/

#include "DEMFilter.h"

#include <string>
#include <vector>

#include <pdal/private/gdal/GdalRaster.h>
#include "private/DimRange.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.dem",
		"Filter points about an elevation surface",
		"http://pdal.io/stages/filters.dem.html"
	};

	CREATE_STATIC_STAGE(DEMFilter, s_info)

	struct DEMArgs
	{
		Dimension::Id m_dim;
		DimRange m_range;
		std::string m_raster;
		int32_t m_band;
	};

	DEMFilter::DEMFilter() : m_args(new DEMArgs)
	{}

	DEMFilter::~DEMFilter()
	{}

	std::string DEMFilter::getName() const
	{
		return s_info.name;
	}

	void DEMFilter::addDimensions(PointLayoutPtr layout)
	{
	}


	void DEMFilter::addArgs(ProgramArgs& args)
	{
		args.add("limits", "Dimension limits for filtering", m_args->m_range).setPositional();
		args.add("raster", "GDAL-readable raster to use for DEM", m_args->m_raster).setPositional();
		args.add("band", "Band number to filter (count from 1)", m_args->m_band, 1);

	}

	void DEMFilter::ready(PointTableRef table)
	{
		m_raster.reset(new gdal::Raster(m_args->m_raster));
		m_raster->open();
	}


	void DEMFilter::prepared(PointTableRef table)
	{
		const PointLayoutPtr layout(table.layout());
		m_args->m_dim = layout->findDim(m_args->m_range.m_name);
		if (m_args->m_dim == Dimension::Id::Unknown)
			throwError("Missing dimension with name '" + m_args->m_range.m_name +
				"'in input PointView.");
		if (m_args->m_band <= 0)
			throwError("Band must be greater than 0");

	}


	bool DEMFilter::processOne(PointRef& point)
	{
		static std::vector<double> data;

		double x = point.getFieldAs<double>(Dimension::Id::X);
		double y = point.getFieldAs<double>(Dimension::Id::Y);
		double z = point.getFieldAs<double>(m_args->m_dim);

		bool passes(false);

		if (m_raster->read(x, y, data) == gdal::GDALError::None)
		{
			double v = data[m_args->m_band - 1];
			double lb = v - m_args->m_range.m_lower_bound;
			double ub = v + m_args->m_range.m_upper_bound;


			if (z >= lb && z <= ub)
				passes = true;
		}
		return passes;
	}

	PointViewSet DEMFilter::run(PointViewPtr inView)
	{
		PointViewSet viewSet;
		if (!inView->size())
			return viewSet;

		PointViewPtr outView = inView->makeNew();

		for (PointId i = 0; i < inView->size(); ++i)
		{
			PointRef point = inView->point(i);
			if (processOne(point))
				outView->appendPoint(*inView, i);
		}

		viewSet.insert(outView);
		return viewSet;
	}
}
