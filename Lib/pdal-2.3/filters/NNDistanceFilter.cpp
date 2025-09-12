/******************************************************************************
* Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

#include "NNDistanceFilter.h"

#include <string>
#include <vector>

#include <pdal/KDIndex.h>

namespace pdal
{
	static PluginInfo const s_info
	{
		"filters.nndistance",
		"NN-Distance Filter",
		"http://pdal.io/stages/filters.nndistance.html"
	};

	CREATE_STATIC_STAGE(NNDistanceFilter, s_info)

	std::string NNDistanceFilter::getName() const
	{
		return s_info.name;
	}

	NNDistanceFilter::NNDistanceFilter() : Filter()
	{}

	std::istream& operator>>(std::istream& in, NNDistanceFilter::Mode& mode)
	{
		std::string s;
		in >> s;

		s = Utils::tolower(s);
		if (s == "kth")
			mode = NNDistanceFilter::Mode::Kth;
		else if (s == "avg")
			mode = NNDistanceFilter::Mode::Average;
		else
			in.setstate(std::ios_base::failbit);
		return in;
	}

	std::ostream& operator<<(std::ostream& out, const NNDistanceFilter::Mode& mode)
	{
		switch (mode)
		{
		case NNDistanceFilter::Mode::Kth:
			out << "kth";
		case NNDistanceFilter::Mode::Average:
			out << "avg";
		}
		return out;
	}

	void NNDistanceFilter::addArgs(ProgramArgs& args)
	{
		args.add("mode", "Distance computation mode (kth, avg)", m_mode, Mode::Kth);
		args.add("k", "k neighbors", m_k, size_t(10));
	}

	void NNDistanceFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Dimension::Id::NNDistance);
	}

	void NNDistanceFilter::filter(PointView& view)
	{
		using namespace Dimension;

		// Build the 3D KD-tree.
		KD3Index& index = view.build3dIndex();

		// Increment the minimum number of points, as knnSearch will be returning
		// the query point along with the neighbors.
		size_t k = m_k + 1;

		// Compute the k-distance for each point. The k-distance is the Euclidean
		// distance to k-th nearest neighbor.
		log()->get(LogLevel::Debug) << "Computing k-distances...\n";
		for (PointId idx = 0; idx < view.size(); ++idx)
		{
			PointIdList indices(k);
			std::vector<double> sqr_dists(k);
			index.knnSearch(idx, k, &indices, &sqr_dists);
			double val;
			if (m_mode == Mode::Kth)
				val = std::sqrt(sqr_dists[k - 1]);
			else // m_mode == Mode::Average
			{
				val = 0;

				// We start at 1 since index 0 is the test point.
				for (size_t i = 1; i < k; ++i)
					val += std::sqrt(sqr_dists[i]);
				val /= (k - 1);
			}
			view.setField(Dimension::Id::NNDistance, idx, val);
		}
	}
}
