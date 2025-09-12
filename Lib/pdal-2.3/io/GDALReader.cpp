/******************************************************************************
* Copyright (c) 2015, Howard Butler <howard@hobu.co>
****************************************************************************/

#include "GDALReader.h"

#include <sstream>

#include <pdal/PointView.h>
#include <pdal/private/gdal/GdalRaster.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"readers.gdal",
		"Read GDAL rasters as point clouds.",
		"http://pdal.io/stages/reader.gdal.html",
		{ "tif", "tiff", "jpeg", "jpg", "png" }

	};

	CREATE_STATIC_STAGE(GDALReader, s_info)

	std::string GDALReader::getName() const
	{
		return s_info.name;
	}

	GDALReader::GDALReader()
		: m_index(0)
	{}

	GDALReader::~GDALReader()
	{
		m_raster.reset();
	}

	void GDALReader::initialize()
	{
		m_raster.reset(new gdal::Raster(m_filename));
		if (m_raster->open() == gdal::GDALError::CantOpen)
			throwError("Couldn't open raster file '" + m_filename + "'.");

		m_raster->open();
		setSpatialReference(m_raster->getSpatialRef());

		m_width = m_raster->width();
		m_height = m_raster->height();
		m_bandTypes = m_raster->getPDALDimensionTypes();

		m_dimNames.clear();
		if (m_header.size())
		{
			m_dimNames = Utils::split(m_header, ',');
			if (m_dimNames.size() != m_bandTypes.size())
				throwError("Dimension names are not the same count as "
					"raster bands.");
		}
		else
		{
			for (size_t i = 0; i < m_bandTypes.size(); ++i)
				m_dimNames.push_back("band_" + std::to_string(i + 1));
		}

		int zBand = 1;
		for (size_t i = 0; i < m_bandIds.size(); ++i)
			if (m_bandIds[i] == Dimension::Id::Z)
			{
				zBand = (int)(i + 1);
				break;
			}

		// Bounds is only used in inspect.  We calculate it here so that
		// the raster can be released.
		m_bounds = m_raster->bounds(zBand);

		m_raster.reset();
	}

	QuickInfo GDALReader::inspect()
	{
		QuickInfo qi;

		initialize();

		qi.m_pointCount = m_width * m_height;
		qi.m_srs = getSpatialReference();
		qi.m_bounds = m_bounds;
		qi.m_valid = true;
		qi.m_dimNames = m_dimNames;

		return qi;
	}


	void GDALReader::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(pdal::Dimension::Id::X);
		layout->registerDim(pdal::Dimension::Id::Y);

		for (size_t i = 0; i < m_bandTypes.size(); ++i)
		{
			const std::string& name = m_dimNames[i];
			Dimension::Type type = m_bandTypes[i];
			m_bandIds.push_back(layout->registerOrAssignDim(name, type));
		}
	}


	void GDALReader::addArgs(ProgramArgs& args)
	{
		args.add("header", "A comma-separated list of dimension IDs to map "
			"raster bands to dimension id", m_header);
		args.add("memorycopy", "Load the given raster file "
			"entirely to memory", m_useMemoryCopy, false).setHidden();
	}


	void GDALReader::ready(PointTableRef table)
	{
		m_raster.reset(new gdal::Raster(m_filename));
		if (m_raster->open() == gdal::GDALError::CantOpen)
			throwError("Couldn't open raster file '" + m_filename + "'.");

		if (m_useMemoryCopy)
		{
			gdal::Raster* r = m_raster->memoryCopy();
			if (r)
				m_raster.reset(r);
			else
				log()->get(LogLevel::Warning) << "Couldn't create raster memory "
				"copy.  Using standard interface.";
		}

		m_index = 0;
		m_row = 0;
		m_col = 0;
	}


	point_count_t GDALReader::read(PointViewPtr view, point_count_t numPts)
	{
		PointId idx = view->size();
		point_count_t cnt = 0;
		PointRef point(*view, idx);
		while (cnt < numPts)
		{
			point.setPointId(idx);
			if (!processOne(point))
				break;
			cnt++;
			idx++;
		}
		return cnt;
	}


	bool GDALReader::processOne(PointRef& point)
	{
		std::array<double, 2> coords;
		if (m_row == m_height)
			return false; // done

		m_raster->pixelToCoord(m_col, m_row, coords);
		double x = coords[0];
		double y = coords[1];
		point.setField(Dimension::Id::X, x);
		point.setField(Dimension::Id::Y, y);

		std::vector<double> data;
		if (m_raster->read(x, y, data) != gdal::GDALError::None)
			return false;

		for (int b = 0; b < m_raster->bandCount(); ++b)
		{
			Dimension::Id id = m_bandIds[b];
			double v = data[b];
			point.setField(id, v);
		}
		m_col++;
		if (m_col == m_width)
		{
			m_col = 0;
			m_row++;
		}

		return true;
	}

	void GDALReader::done(PointTableRef table)
	{
		m_raster->close();
	}
}
