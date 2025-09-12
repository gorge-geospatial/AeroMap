/******************************************************************************
* Copyright (c) 2017, Hobu Inc. <info@hobu.co>
****************************************************************************/

#include "OGRWriter.h"

#include <sstream>

#include <pdal/PointView.h>
#include <pdal/private/gdal/GDALUtils.h>
#include <pdal/util/FileUtils.h>
#include <pdal/private/gdal/ErrorHandler.h>

#include <ogr_core.h>
#include <ogrsf_frmts.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"writers.ogr",
		"Write a point cloud as a set of OGR points/multipoints",
		"http://pdal.io/stages/writers.ogr.html",
		{ "shp", "geojson" }
	};

	CREATE_STATIC_STAGE(OGRWriter, s_info)

	OGRWriter::OGRWriter()
		: m_driver(nullptr)
		, m_ds(nullptr)
		, m_layer(nullptr)
		, m_feature(nullptr)
	{
	}

	std::string OGRWriter::getName() const
	{
		return s_info.name;
	}

	void OGRWriter::addArgs(ProgramArgs& args)
	{
		args.add("filename", "Output filename", m_filename).setPositional();
		args.add("multicount", "Group 'multicount' points into a structure", m_multiCount, (size_t)1);
		args.add("measure_dim", "Use dimensions as a measure value", m_measureDimName);
		args.add("ogrdriver", "OGR writer driver name", m_driverName, m_driverName);
	}

	void OGRWriter::initialize()
	{
		gdal::registerDrivers();
		if (m_multiCount < 1)
			throwError("'m_multicount' must be greater than 0.");
	}

	void OGRWriter::prepared(PointTableRef table)
	{
		if (m_measureDimName.size())
		{
			m_measureDim = table.layout()->findDim(m_measureDimName);
			if (m_measureDim == Dimension::Id::Unknown)
				throwError("Dimension '" + m_measureDimName + "' (measure_dim) not "
					"found.");
		}

		if (m_driverName.empty())
		{
			if (FileUtils::extension(m_filename) == ".geojson")
				m_driverName = "GeoJSON";
			else
				m_driverName = "ESRI Shapefile";
		}
	}


	void OGRWriter::readyTable(PointTableRef table)
	{
		m_driver = GetGDALDriverManager()->GetDriverByName(m_driverName.data());
		if (m_measureDim == Dimension::Id::Unknown)
			m_geomType = (m_multiCount == 1) ? wkbPoint : wkbMultiPoint;
		else
			m_geomType = (m_multiCount == 1) ? wkbPointZM : wkbMultiPointZM;
	}

	void OGRWriter::readyFile(const std::string& filename, const SpatialReference& srs)
	{
		m_outputFilename = filename;
		m_ds = m_driver->Create(filename.data(), 0, 0, 0, GDT_Unknown, nullptr);
		if (!m_ds)
			throwError("Unable to open OGR datasource '" + filename + "'.\n");
		m_layer = m_ds->CreateLayer("points", nullptr, m_geomType, nullptr);
		if (!m_layer)
			throwError("Can't create OGR layer for points.\n");
		{
			gdal::ErrorHandlerSuspender devnull;

			m_ds->SetProjection(srs.getWKT().data());
		}
		m_feature = OGRFeature::CreateFeature(m_layer->GetLayerDefn());
	}

	void OGRWriter::writeView(const PointViewPtr view)
	{
		m_curCount = 0;
		PointRef point(*view, 0);
		for (PointId idx = 0; idx < view->size(); ++idx)
		{
			point.setPointId(idx);
			processOne(point);
		}
	}

	bool OGRWriter::processOne(PointRef& point)
	{
		double x = point.getFieldAs<double>(Dimension::Id::X);
		double y = point.getFieldAs<double>(Dimension::Id::Y);
		double z = point.getFieldAs<double>(Dimension::Id::Z);
		double m = point.getFieldAs<double>(m_measureDim);

		OGRPoint pt(x, y, z);
		if (m_measureDim != Dimension::Id::Unknown)
			pt.setM(m);
		m_curCount++;

		if (m_multiCount > 1)
			m_multiPoint.addGeometry(&pt);
		if (m_curCount == m_multiCount)
		{
			if (m_multiCount > 1)
			{
				m_feature->SetGeometry(&m_multiPoint);
				m_multiPoint.empty();
			}
			else
				m_feature->SetGeometry(&pt);
			if (m_layer->CreateFeature(m_feature))
				throwError("Couldn't create feature.");
			m_curCount = 0;
		}
		return true;
	}

	void OGRWriter::doneFile()
	{
		if (m_curCount)
			if (m_layer->CreateFeature(m_feature))
				throwError("Couldn't create feature.");
		OGRFeature::DestroyFeature(m_feature);
		GDALClose(m_ds);
		m_layer = nullptr;
		m_ds = nullptr;
	}
}
