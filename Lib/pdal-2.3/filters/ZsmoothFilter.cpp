/******************************************************************************
 * Copyright (c) 2020, University Nevada, Reno
 ****************************************************************************/

#include <pdal/KDIndex.h>

#include "ZsmoothFilter.h"

namespace pdal
{
	static PluginInfo const ptstatInfo
	{
		"filters.zsmooth",
		"Zsmooth Filter",
		"http://pdal.io/stages/filters.zsmooth.html"
	};

	struct ZsmoothFilter::Private
	{
		double radius;
		double pos;
		std::string dimName;
		Dimension::Id statDim;
	};

	CREATE_SHARED_STAGE(ZsmoothFilter, ptstatInfo)

	std::string ZsmoothFilter::getName() const
	{
		return ptstatInfo.name;
	}

	ZsmoothFilter::ZsmoothFilter() : m_p(new Private)
	{}

	ZsmoothFilter::~ZsmoothFilter()
	{}

	void ZsmoothFilter::addArgs(ProgramArgs& args)
	{
		args.add("radius", "Radius in X/Y plane in which to find neighboring points", m_p->radius, 1.0);
		args.add("medianpercent", "Location (percent) in neighbor list at which to find "
			"neighbor Z value (min == 0, max == 100, median == 50, etc.)", m_p->pos, 50.0);
		args.add("dim", "Name of dimension in which to store statistic", m_p->dimName).setPositional();
	}

	void ZsmoothFilter::addDimensions(PointLayoutPtr layout)
	{
		m_p->statDim = layout->registerOrAssignDim(m_p->dimName, Dimension::Type::Double);
		if (m_p->statDim == Dimension::Id::Z)
			throwError("Can't use 'Z' as output dimension.");
	}

	void ZsmoothFilter::prepared(PointTableRef)
	{
		if (m_p->pos < 0.0 || m_p->pos > 100.0)
			throwError("'medicanpercent' value must be in the range [0, 100]");
		m_p->pos /= 100.0;
	}

	void ZsmoothFilter::filter(PointView& view)
	{
		const KD2Index& kdi = view.build2dIndex();

		for (PointId idx = 0; idx < view.size(); ++idx)
		{
			double d = view.getFieldAs<double>(Dimension::Id::Z, idx);

			std::vector<double> valList;
			PointIdList nears = kdi.radius(idx, m_p->radius);
			for (PointId n = 1; n < nears.size(); ++n)
			{
				double z = view.getFieldAs<double>(Dimension::Id::Z, nears[n]);
				valList.push_back(z);
			}
			std::sort(valList.begin(), valList.end());

			double val;
			if (valList.empty())
				val = view.getFieldAs<double>(Dimension::Id::Z, idx);
			else if (valList.size() == 1)
				val = valList[0];
			else if (m_p->pos == 0.0)
				val = valList[0];
			else if (m_p->pos == 1.0)
				val = valList[valList.size() - 1];
			else
			{
				double pos = m_p->pos * (valList.size() - 1);
				size_t low = (size_t)std::floor(pos);
				size_t high = low + 1;
				double highfrac = pos - low;
				double lowfrac = 1 - highfrac;
				val = valList[low] * lowfrac + valList[high] * highfrac;

			}
			view.setField(m_p->statDim, idx, val);
		}
	}
}
