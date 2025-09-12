/******************************************************************************
 * Copyright (c) 2017, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "VoxelCenterNearestNeighborFilter.h"

#include <pdal/KDIndex.h>

#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.voxelcenternearestneighbor",
		"Voxel Center Nearest Neighbor Filter",
		"http://pdal.io/stages/filters.voxelcenternearestneighbor.html"
	};

	CREATE_STATIC_STAGE(VoxelCenterNearestNeighborFilter, s_info)

	std::string VoxelCenterNearestNeighborFilter::getName() const
	{
		return s_info.name;
	}

	void VoxelCenterNearestNeighborFilter::addArgs(ProgramArgs& args)
	{
		args.add("cell", "Cell size", m_cell, 1.0);
	}

	PointViewSet VoxelCenterNearestNeighborFilter::run(PointViewPtr view)
	{
		BOX3D bounds;
		view->calculateBounds(bounds);

		// Find distance from voxel center to point.  If the distance is less
		// than previous (or is the first one for the voxel), store the
		// point ID and distance.
		std::map<std::tuple<size_t, size_t, size_t>,
			std::tuple<PointId, double>> populated_voxels;
		for (PointId id = 0; id < view->size(); ++id)
		{
			double x = view->getFieldAs<double>(Dimension::Id::X, id);
			double y = view->getFieldAs<double>(Dimension::Id::Y, id);
			double z = view->getFieldAs<double>(Dimension::Id::Z, id);
			size_t c = static_cast<size_t>((x - bounds.minx) / m_cell);
			size_t r = static_cast<size_t>((y - bounds.miny) / m_cell);
			size_t d = static_cast<size_t>((z - bounds.minz) / m_cell);
			double xv = bounds.minx + (c + 0.5) * m_cell;
			double yv = bounds.miny + (r + 0.5) * m_cell;
			double zv = bounds.minz + (d + 0.5) * m_cell;
			double dist = pow(xv - x, 2) + pow(yv - y, 2) + pow(zv - z, 2);

			auto t = std::make_tuple(r, c, d);
			auto pi = populated_voxels.find(t);
			if (pi == populated_voxels.end())
				populated_voxels.insert(
					std::make_pair(t, std::make_tuple(id, dist)));
			else
			{
				auto& t2 = pi->second;  // Get point/distance tuple.
				double curDist = std::get<1>(t2);
				if (dist < curDist)
					t2 = std::make_tuple(id, dist);
			}
		}

		// Append the ID of the point nearest the voxel center to the output view.
		PointViewPtr output = view->makeNew();
		for (auto const& t : populated_voxels)
		{
			auto& t2 = t.second; // Get point/distance tuple.
			output->appendPoint(*view, std::get<0>(t2));
		}

		PointViewSet viewSet;
		viewSet.insert(output);
		return viewSet;
	}
}
