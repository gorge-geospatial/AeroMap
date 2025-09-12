/******************************************************************************
* Copyright (c) 2019, Aurelien Vila (aurelien.vila@delair.aero)
****************************************************************************/

#include "ProjPipelineFilter.h"

#include <pdal/PointView.h>
#include <pdal/private/SrsTransform.h>
#include <pdal/util/ProgramArgs.h>

#include <ogr_spatialref.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.projpipeline",
		"Transform coordinates using Proj pipeline string, WKT2 coordinate operations or URN definition",
		"http://pdal.io/stages/filters.projpipeline.html"
	};

	CREATE_STATIC_STAGE(ProjPipelineFilter, s_info)

	std::string ProjPipelineFilter::getName() const { return s_info.name; }

	ProjPipelineFilter::ProjPipelineFilter()
	{}


	ProjPipelineFilter::~ProjPipelineFilter()
	{}


	void ProjPipelineFilter::addArgs(ProgramArgs& args)
	{
		args.add("out_srs", "Output spatial reference", m_outSRS);
		args.add("reverse_transfo", "Wether the coordinate operation should be evaluated in the reverse path",
			m_reverseTransfo, false);
		args.add("coord_op", "Coordinate operation (Proj pipeline or WKT2 string or urn definition)",
			m_coordOperation).setPositional();
	}


	void ProjPipelineFilter::initialize()
	{
		setSpatialReference(m_outSRS);
		createTransform(m_coordOperation, m_reverseTransfo);
	}


	void ProjPipelineFilter::createTransform(const std::string coordOperation, bool reverseTransfo)
	{
		m_coordTransform.reset(new CoordTransform(coordOperation, reverseTransfo));
	}


	PointViewSet ProjPipelineFilter::run(PointViewPtr view)
	{
		PointViewSet viewSet;
		PointViewPtr outView = view->makeNew();

		PointRef point(*view, 0);
		for (PointId id = 0; id < view->size(); ++id)
		{
			point.setPointId(id);
			if (processOne(point))
				outView->appendPoint(*view, id);
		}

		viewSet.insert(outView);
		return viewSet;
	}


	bool ProjPipelineFilter::processOne(PointRef& point)
	{
		double x(point.getFieldAs<double>(Dimension::Id::X));
		double y(point.getFieldAs<double>(Dimension::Id::Y));
		double z(point.getFieldAs<double>(Dimension::Id::Z));

		bool ok = m_coordTransform->transform(x, y, z);
		if (ok)
		{
			point.setField(Dimension::Id::X, x);
			point.setField(Dimension::Id::Y, y);
			point.setField(Dimension::Id::Z, z);
		}
		return ok;
	}

	ProjPipelineFilter::CoordTransform::CoordTransform() {}

	ProjPipelineFilter::CoordTransform::CoordTransform(const std::string coordOperation, bool reverseTransfo) {
		OGRCoordinateTransformationOptions coordTransfoOptions;
		coordTransfoOptions.SetCoordinateOperation(coordOperation.c_str(), reverseTransfo);
		OGRSpatialReference nullSrs("");
		m_transform.reset(OGRCreateCoordinateTransformation(&nullSrs, &nullSrs, coordTransfoOptions));
	}

	bool ProjPipelineFilter::CoordTransform::transform(double& x, double& y, double& z) {

		return m_transform && m_transform->Transform(1, &x, &y, &z);
	}
}
