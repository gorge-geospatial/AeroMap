/******************************************************************************
 * Copyright (c) 2020, Hobu Inc.
 ****************************************************************************/

#include "FaceRasterFilter.h"

#include <pdal/util/Utils.h>
#include <pdal/private/MathUtils.h>
#include <pdal/private/Raster.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.faceraster",
		"Face Raster Filter",
		"http://pdal.io/stages/filters.faceraster.html"
	};

	CREATE_STATIC_STAGE(FaceRasterFilter, s_info)

	std::string FaceRasterFilter::getName() const
	{
		return s_info.name;
	}

	FaceRasterFilter::FaceRasterFilter() : m_limits(new RasterLimits)
	{}

	void FaceRasterFilter::addArgs(ProgramArgs& args)
	{
		m_limits->addArgs(args);
		args.add("mesh", "Mesh name", m_meshName);
		args.add("nodata", "No data value", m_noData, std::numeric_limits<double>::quiet_NaN());
	}

	void FaceRasterFilter::prepared(PointTableRef)
	{
		int cnt = m_limits->checkArgs();
		if (cnt != 0 && cnt != 4)
			throwError("Must specify all or none of 'origin_x', 'origin_y', 'width' and 'height'.");
		m_computeLimits = (cnt == 0);
	}


	void FaceRasterFilter::filter(PointView& v)
	{
		double halfEdge = m_limits->edgeLength / 2;
		double edgeBit = m_limits->edgeLength * .000001;

		// If the user hasn't set bounds, set them based on the data.
		if (m_computeLimits)
		{
			BOX2D bounds;
			v.calculateBounds(bounds);
			m_limits->xOrigin = bounds.minx - halfEdge;
			m_limits->yOrigin = bounds.miny - halfEdge;
			m_limits->width = (int)(((bounds.maxx - m_limits->xOrigin) / m_limits->edgeLength) + 1);
			m_limits->height = (int)(((bounds.maxy - m_limits->yOrigin) / m_limits->edgeLength) + 1);
		}
		Rasterd* raster = v.createRaster("faceraster", *m_limits, m_noData);
		if (!raster)
			throwError("Raster already exists");

		TriangularMesh* m = v.mesh(m_meshName);
		if (!m)
			throwError("Mesh '" + m_meshName + "' does not exist.");

		for (const Triangle& t : *m)
		{
			double x1 = v.getFieldAs<double>(Dimension::Id::X, t.m_a);
			double y1 = v.getFieldAs<double>(Dimension::Id::Y, t.m_a);
			double z1 = v.getFieldAs<double>(Dimension::Id::Z, t.m_a);

			double x2 = v.getFieldAs<double>(Dimension::Id::X, t.m_b);
			double y2 = v.getFieldAs<double>(Dimension::Id::Y, t.m_b);
			double z2 = v.getFieldAs<double>(Dimension::Id::Z, t.m_b);

			double x3 = v.getFieldAs<double>(Dimension::Id::X, t.m_c);
			double y3 = v.getFieldAs<double>(Dimension::Id::Y, t.m_c);
			double z3 = v.getFieldAs<double>(Dimension::Id::Z, t.m_c);

			double xmax = (std::max)((std::max)(x1, x2), x3);
			double xmin = (std::min)((std::min)(x1, x2), x3);
			double ymax = (std::max)((std::max)(y1, y2), y3);
			double ymin = (std::min)((std::min)(y1, y2), y3);

			// Since we're checking cell centers, we add 1/2 the edge length to avoid testing cells
			// where we know the limiting position can't intersect the cell center.  The
			// subtraction of edgeBit for the lower bound is to allow for the case where the
			// minimum position is exactly aligned with a cell center (we could simply start one cell
			// lower and to the left, but this small adjustment eliminates that extra row/col in most
			// cases).
			int ax = raster->xCell(xmin + halfEdge - edgeBit);
			int ay = raster->yCell(ymin + halfEdge - edgeBit);

			// edgeBit adjustment not necessary here since we're rounding up for exact values.
			int bx = raster->xCell(xmax + halfEdge);
			int by = raster->yCell(ymax + halfEdge);

			ax = Utils::clamp(ax, 0, (int)m_limits->width);
			bx = Utils::clamp(bx, 0, (int)m_limits->width);
			ay = Utils::clamp(ay, 0, (int)m_limits->height);
			by = Utils::clamp(by, 0, (int)m_limits->height);

			for (int xi = ax; xi < bx; ++xi)
				for (int yi = ay; yi < by; ++yi)
				{
					double x = raster->xCellPos(xi);
					double y = raster->yCellPos(yi);

					double val = math::barycentricInterpolation(x1, y1, z1,
						x2, y2, z2, x3, y3, z3, x, y);
					if (val != std::numeric_limits<double>::infinity())
						raster->at(xi, yi) = val;
				}
		}
	}
}
