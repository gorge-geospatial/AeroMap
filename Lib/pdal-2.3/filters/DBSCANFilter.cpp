/******************************************************************************
 * Copyright (c) 2019, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

 // Adapted from MIT-licensed implemenation provided by
 // https://github.com/intel-isl/Open3D/pull/1038.

#include "DBSCANFilter.h"

#include <pdal/KDIndex.h>

#include <string>
#include <unordered_set>

namespace pdal
{
	using namespace Dimension;

	static StaticPluginInfo const s_info
	{
		"filters.dbscan", 
		"DBSCAN Clustering.",
		"http://pdal.io/stages/filters.dbscan.html" 
	};

	CREATE_STATIC_STAGE(DBSCANFilter, s_info)

	std::string DBSCANFilter::getName() const
	{
		return s_info.name;
	}

	DBSCANFilter::DBSCANFilter() : Filter()
	{
	}

	void DBSCANFilter::addArgs(ProgramArgs& args)
	{
		args.add("min_points", "Min points per cluster", m_minPoints, static_cast<uint64_t>(6));
		args.add("eps", "Epsilon", m_eps, 1.0);
		args.add("dimensions", "Dimensions to cluster", m_dimStringList, { "X", "Y", "Z" });
	}

	void DBSCANFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Id::ClusterID);
	}

	void DBSCANFilter::prepared(PointTableRef table)
	{
		const PointLayoutPtr layout(table.layout());

		if (m_dimStringList.size())
		{
			for (std::string& s : m_dimStringList)
			{
				Id id = layout->findDim(s);
				if (id == Id::Unknown)
					throwError("Invalid dimension '" + s +
						"' specified for 'dimensions' option.");
				m_dimIdList.push_back(id);
			}
		}
	}

	void DBSCANFilter::filter(PointView& view)
	{
		// Construct KDFlexIndex for radius search.
		KDFlexIndex kdfi(view, m_dimIdList);
		kdfi.build();

		// First pass through point cloud precomputes neighbor indices and
		// initializes ClusterID to -2.
		std::vector<PointIdList> neighbors(view.size());
		for (PointId idx = 0; idx < view.size(); ++idx)
		{
			neighbors[idx] = kdfi.radius(idx, m_eps);
			view.setField(Id::ClusterID, idx, -2);
		}

		// Second pass through point cloud performs DBSCAN clustering.
		int64_t cluster_label = 0;
		for (PointId idx = 0; idx < view.size(); ++idx)
		{
			// Point has already been labeled, so move on to next.
			if (view.getFieldAs<int64_t>(Id::ClusterID, idx) != -2)
				continue;

			// Density of the neighborhood does not meet minimum number of points
			// constraint, label as noise.
			if (neighbors[idx].size() < m_minPoints)
			{
				view.setField(Id::ClusterID, idx, -1);
				continue;
			}

			// Initialize some bookkeeping to track which neighbors have been
			// considered for addition to the current cluster.
			std::unordered_set<PointId> neighbors_next(neighbors[idx].begin(),
				neighbors[idx].end());
			std::unordered_set<PointId> neighbors_visited;
			neighbors_visited.insert(idx);

			// Unlabeled point encountered; assign cluster label.
			view.setField(Id::ClusterID, idx, cluster_label);

			// Consider all neighbors.
			while (!neighbors_next.empty())
			{
				// Select first neighbor and move it to the visited set.
				PointId p = *neighbors_next.begin();
				neighbors_next.erase(neighbors_next.begin());
				neighbors_visited.insert(p);

				// Reassign cluster label to neighbor previously marked as noise.
				if (view.getFieldAs<int64_t>(Id::ClusterID, p) == -1)
					view.setField(Id::ClusterID, p, cluster_label);

				// Neighbor has already been labeled, so move on to next.
				if (view.getFieldAs<int64_t>(Id::ClusterID, p) != -2)
					continue;

				// Assign cluster label to neighbor.
				view.setField(Id::ClusterID, p, cluster_label);

				// If density of neighbor's neighborhood is sufficient, add it's
				// neighbors to the set of neighbors to consider if they are not
				// already.
				if (neighbors[p].size() >= m_minPoints)
				{
					for (PointId q : neighbors[p])
					{
						if (neighbors_visited.count(q) == 0)
							neighbors_next.insert(q);
					}
				}
			}

			cluster_label++;
		}
	}
}
