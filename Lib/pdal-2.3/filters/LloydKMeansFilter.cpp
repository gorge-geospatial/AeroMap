/******************************************************************************
 * Copyright (c) 2020, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "LloydKMeansFilter.h"

#include "private/Segmentation.h"

#include <pdal/KDIndex.h>

namespace pdal
{
	using namespace Dimension;

	static StaticPluginInfo const s_info
	{
		"filters.lloydkmeans",
		"Extract and label clusters using K-means (Lloyd's algorithm).",
		"http://pdal.io/stages/filters.lloydkmeans.html" 
	};

	CREATE_STATIC_STAGE(LloydKMeansFilter, s_info)

	LloydKMeansFilter::LloydKMeansFilter() {}

	std::string LloydKMeansFilter::getName() const
	{
		return s_info.name;
	}

	void LloydKMeansFilter::addArgs(ProgramArgs& args)
	{
		args.add("k", "Number of clusters to segment", m_k,
			static_cast<uint16_t>(10));
		args.add("dimensions", "Dimensions to cluster", m_dimStringList,
			{ "X", "Y", "Z" });
		args.add("maxiters", "Maximum number of iterations", m_maxiters,
			static_cast<uint16_t>(10));
	}

	void LloydKMeansFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Id::ClusterID);
	}

	void LloydKMeansFilter::prepared(PointTableRef table)
	{
		const PointLayoutPtr layout(table.layout());

		if (m_dimStringList.size())
		{
			for (std::string& s : m_dimStringList)
			{
				Dimension::Id id = layout->findDim(s);
				if (id == Dimension::Id::Unknown)
					throwError("Invalid dimension '" + s +
						"' specified for "
						"'dimensions' option.");
				m_dimIdList.push_back(id);
			}
		}
	}

	void LloydKMeansFilter::filter(PointView& view)
	{
		if (!view.size() || (view.size() < m_k))
			return;

		// come up with k random samples for initial cluster centers (based on
		// spatial farthest point sampling)
		PointIdList ids = Segmentation::farthestPointSampling(view, m_k);

		// setup table with at least XYZ as required by KDIndex, plus any
		// additional dimensions as specified via filter options
		ColumnPointTable table;
		table.layout()->registerDims({ Id::X, Id::Y, Id::Z });
		table.layout()->registerDims(m_dimIdList);
		table.finalize();

		// create view of initial cluster centers
		PointViewPtr centers(new PointView(table));
		PointId i(0);
		for (auto const& id : ids)
		{
			centers->setField(Id::X, i, view.getFieldAs<double>(Id::X, id));
			centers->setField(Id::Y, i, view.getFieldAs<double>(Id::Y, id));
			centers->setField(Id::Z, i, view.getFieldAs<double>(Id::Z, id));
			for (auto const& dimid : m_dimIdList)
			{
				centers->setField(dimid, i, view.getFieldAs<double>(dimid, id));
			}
			i++;
		}

		// construct KDFlexIndex for nearest neighbor search
		KDFlexIndex kdi(*centers, m_dimIdList);
		kdi.build();

		// update cluster centers through specified number of iterations
		for (int iter = 0; iter < m_maxiters; ++iter)
		{
			// initialize mean and count for each cluster
			std::vector<std::vector<double>> M1(m_dimIdList.size(),
				std::vector<double>(m_k, 0.0));
			std::vector<point_count_t> cnt(m_k, 0);

			// for every point, find closest cluster center and assign ClusterID
			for (PointRef p : view)
			{
				PointId q = kdi.neighbor(p);
				p.setField(Id::ClusterID, q);

				// update cluster size and mean
				cnt[q]++;
				point_count_t n(cnt[q]);
				for (size_t i = 0; i < m_dimIdList.size(); ++i)
				{
					double delta = p.getFieldAs<double>(m_dimIdList[i]) - M1[i][q];
					double delta_n = delta / n;
					M1[i][q] += delta_n;
				}
			}

			// adjust clusters based on newly added points
			for (PointId id = 0; id < m_k; ++id)
			{
				for (size_t i = 0; i < m_dimIdList.size(); ++i)
				{
					centers->setField(m_dimIdList[i], id, M1[i][id]);
				}
			}
			centers->invalidateProducts();
			centers->build3dIndex();
		}
	}
}
