/******************************************************************************
 * Copyright (c) 2016-2017, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "OutlierFilter.h"

#include <pdal/KDIndex.h>
#include <pdal/util/ProgramArgs.h>
#include <pdal/util/Utils.h>

#include <string>
#include <vector>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.outlier",
		"Outlier removal",
		"http://pdal.io/stages/filters.outlier.html"
	};

	CREATE_STATIC_STAGE(OutlierFilter, s_info)

	std::string OutlierFilter::getName() const
	{
		return s_info.name;
	}

	void OutlierFilter::addArgs(ProgramArgs& args)
	{
		args.add("method", "Method [default: statistical]", m_method, "statistical");
		args.add("min_k", "Minimum number of neighbors in radius", m_minK, 2);
		args.add("radius", "Radius", m_radius, 1.0);
		args.add("mean_k", "Mean number of neighbors", m_meanK, 8);
		args.add("multiplier", "Standard deviation threshold", m_multiplier, 2.0);
		args.add("class", "Class to use for noise points", m_class, ClassLabel::LowPoint);
	}

	void OutlierFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Dimension::Id::Classification);
	}

	Indices OutlierFilter::processRadius(PointViewPtr inView)
	{
		KD3Index index(*inView);
		index.build();

		point_count_t np = inView->size();

		PointIdList inliers, outliers;

		for (PointId i = 0; i < np; ++i)
		{
			auto ids = index.radius(i, m_radius);
			if (ids.size() > size_t(m_minK))
				inliers.push_back(i);
			else
				outliers.push_back(i);
		}

		return Indices{ inliers, outliers };
	}

	Indices OutlierFilter::processStatistical(PointViewPtr inView)
	{
		KD3Index index(*inView);
		index.build();

		point_count_t np = inView->size();

		PointIdList inliers, outliers;

		std::vector<double> distances(np, 0.0);

		// we increase the count by one because the query point itself will
		// be included with a distance of 0
		point_count_t count = m_meanK + 1;
		PointIdList indices(count);
		std::vector<double> sqr_dists(count);
		for (PointId i = 0; i < np; ++i)
		{

			index.knnSearch(i, count, &indices, &sqr_dists);

			for (size_t j = 1; j < count; ++j)
			{
				double delta = std::sqrt(sqr_dists[j]) - distances[i];
				distances[i] += (delta / j);
			}
			indices.clear(); indices.resize(count);
			sqr_dists.clear(); sqr_dists.resize(count);
		}

		size_t n(0);
		double M1(0.0);
		double M2(0.0);
		for (auto const& d : distances)
		{
			size_t n1(n);
			n++;
			double delta = d - M1;
			double delta_n = delta / n;
			M1 += delta_n;
			M2 += delta * delta_n * n1;
		}
		double mean = M1;
		double variance = M2 / (n - 1.0);
		double stdev = std::sqrt(variance);

		double threshold = mean + m_multiplier * stdev;

		for (PointId i = 0; i < np; ++i)
		{
			if (distances[i] < threshold)
				inliers.push_back(i);
			else
				outliers.push_back(i);
		}

		return Indices{ inliers, outliers };
	}

	PointViewSet OutlierFilter::run(PointViewPtr inView)
	{
		PointViewSet viewSet;
		if (!inView->size())
			return viewSet;

		Indices indices;
		if (Utils::iequals(m_method, "statistical"))
		{
			indices = processStatistical(inView);
		}
		else if (Utils::iequals(m_method, "radius"))
		{
			indices = processRadius(inView);
		}
		else
		{
			log()->get(LogLevel::Warning) << "Requested method is unrecognized. "
				"Please choose from \"statistical\" "
				"or \"radius\".\n";
			viewSet.insert(inView);
			return viewSet;
		}

		if (indices.inliers.empty())
		{
			log()->get(LogLevel::Warning) << "Requested filter would remove all "
				"points. Try a larger radius/smaller "
				"minimum neighbors.\n";
			viewSet.insert(inView);
			return viewSet;
		}

		if (!indices.outliers.empty())
		{
			log()->get(LogLevel::Debug2)
				<< "Labeled " << indices.outliers.size() << " outliers as noise!\n";

			// set the classification label of outlier returns
			for (const auto& i : indices.outliers)
				inView->setField(Dimension::Id::Classification, i, m_class);

			viewSet.insert(inView);
		}
		else
		{
			if (indices.outliers.empty())
				log()->get(LogLevel::Warning)
				<< "Filtered cloud has no outliers!\n";

			// return the input buffer unchanged
			viewSet.insert(inView);
		}

		return viewSet;
	}
}
