/******************************************************************************
* Copyright (c) 2018, Danish Agency for Data Supply and Efficiency,
* sdfe@sdfe.dk
****************************************************************************/

#include <cstddef> // NULL
#include "DelaunayFilter.h"
#include "private/delaunator.h"

namespace pdal
{
	static PluginInfo const s_info
	{
		"filters.delaunay",
		"Perform Delaunay triangulation of a pointcloud",
		"http://pdal.io/stages/filters.delaunay.html"
	};

	CREATE_STATIC_STAGE(DelaunayFilter, s_info)

	std::string DelaunayFilter::getName() const
	{
		return s_info.name;
	}

	DelaunayFilter::DelaunayFilter()
	{}

	DelaunayFilter::~DelaunayFilter()
	{}

	void DelaunayFilter::filter(PointView& pointView)
	{
		// Returns NULL if the mesh already exists
		TriangularMesh* mesh = pointView.createMesh("delaunay2d");

		if (!mesh)
			throwError("Unable to create mesh 'delaunay2d'.");
		if (pointView.size() < 3)
		{
			log()->get(LogLevel::Warning) << getName() << ": triangulation "
				"requested for fewer than three points.\n";
			return;
		}

		std::vector<double> delaunayPoints;
		for (PointId i = 0; i < pointView.size(); i++)
		{
			delaunayPoints.push_back(
				pointView.getFieldAs<double>(Dimension::Id::X, i));
			delaunayPoints.push_back(
				pointView.getFieldAs<double>(Dimension::Id::Y, i));
		}

		// Actually perform the triangulation
		delaunator::Delaunator triangulation(delaunayPoints);

		for (std::size_t i = 0; i < triangulation.triangles.size(); i += 3)
			mesh->add(triangulation.triangles[i + 2], triangulation.triangles[i + 1],
				triangulation.triangles[i]);
	}
}
