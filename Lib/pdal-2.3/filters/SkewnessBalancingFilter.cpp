/******************************************************************************
 * Copyright (c) 2019, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "SkewnessBalancingFilter.h"

#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	static PluginInfo const s_info
	{
		"filters.skewnessbalancing", 
		"Bartels & Wei Skewness Balancing",
		"http://pdal.io/stages/filters.skewnessbalancing.html"
	};

	CREATE_STATIC_STAGE(SkewnessBalancingFilter, s_info)

	std::string SkewnessBalancingFilter::getName() const
	{
		return s_info.name;
	}

	void SkewnessBalancingFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Dimension::Id::Classification);
	}

	void SkewnessBalancingFilter::processGround(PointViewPtr view)
	{
		auto cmp = [](const PointRef& p1, const PointRef& p2) {
			return p1.compare(Dimension::Id::Z, p2);
		};
		std::sort(view->begin(), view->end(), cmp);

		auto setClass = [&view](PointId first, PointId last, int cl)
		{
			for (PointId idx = first; idx <= last; ++idx)
				view->setField(Dimension::Id::Classification, idx, cl);
		};

		point_count_t n(0);
		point_count_t n1(0);
		double delta, delta_n, term1, M1, M2, M3;
		M1 = M2 = M3 = 0.0;

		PointId lastPositive = 0;
		double skewness = 0;
		double lastSkewness = std::numeric_limits<double>::quiet_NaN();
		for (PointId i = 0; i < view->size(); ++i)
		{
			double z = view->getFieldAs<double>(Dimension::Id::Z, i);
			n1 = n;
			n++;
			delta = z - M1;
			delta_n = delta / n;
			term1 = delta * delta_n * n1;
			M1 += delta_n;
			M3 += term1 * delta_n * (n - 2) - 3 * delta_n * M2;
			M2 += term1;
			skewness = std::sqrt(n) * M3 / std::pow(M2, 1.5);
			if (skewness > 0 && lastSkewness <= 0)
			{
				setClass(lastPositive, i - 1, ClassLabel::Ground);
				lastPositive = i;
			}
			lastSkewness = skewness;
		}
		// It's possible that all our points have skewness <= 0, in which case
		// we've never had an opportunity to set the ground state.  Do so now.
		// Otherwise, set the remaining points to non-ground.
		if (lastPositive == 0 && skewness <= 0)
			setClass(lastPositive, view->size() - 1, ClassLabel::Ground);
		else
			setClass(lastPositive, view->size() - 1, ClassLabel::Unclassified);
	}

	PointViewSet SkewnessBalancingFilter::run(PointViewPtr input)
	{
		PointViewSet viewSet;
		if (!input->size())
			return viewSet;
		viewSet.insert(input);

		bool logOutput = log()->getLevel() > LogLevel::Debug1;
		if (logOutput)
			log()->floatPrecision(8);

		processGround(input);

		return viewSet;
	}
}
