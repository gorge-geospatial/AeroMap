/******************************************************************************
* Copyright (c) 2017, Hobu Inc., info@hobu.co
****************************************************************************/

#include "NeighborClassifierFilter.h"

#include <pdal/PipelineManager.h>
#include <pdal/StageFactory.h>
#include <pdal/util/ProgramArgs.h>

#include "private/DimRange.h"

#include <iostream>
#include <utility>
namespace pdal
{

	static PluginInfo const s_info
	{
		"filters.neighborclassifier",
		"Re-assign some point attributes based KNN voting",
		"http://pdal.io/stages/filters.neighborclassifier.html"
	};

	CREATE_STATIC_STAGE(NeighborClassifierFilter, s_info)

	NeighborClassifierFilter::NeighborClassifierFilter() :
		m_dim(Dimension::Id::Classification)
	{}

	NeighborClassifierFilter::~NeighborClassifierFilter()
	{}

	void NeighborClassifierFilter::addArgs(ProgramArgs& args)
	{
		args.add("domain", "Selects which points will be subject to KNN-based assignment", m_domainSpec);
		args.add("k", "Number of nearest neighbors to consult",	m_k).setPositional();
		args.add("candidate", "candidate file name", m_candidateFile);
	}

	void NeighborClassifierFilter::initialize()
	{
		for (auto const& r : m_domainSpec)
		{
			try
			{
				DimRange range;
				range.parse(r);
				m_domain.push_back(range);
			}
			catch (const DimRange::error& err)
			{
				throwError("Invalid 'domain' option: '" + r + "': " + err.what());
			}
		}
		if (m_k < 1)
			throwError("Invalid 'k' option: " + std::to_string(m_k) +
				", must be > 0");
	}


	void NeighborClassifierFilter::prepared(PointTableRef table)
	{
		PointLayoutPtr layout(table.layout());

		for (auto& r : m_domain)
		{
			r.m_id = layout->findDim(r.m_name);
			if (r.m_id == Dimension::Id::Unknown)
				throwError("Invalid dimension name in 'domain' option: '" +
					r.m_name + "'.");
		}
		std::sort(m_domain.begin(), m_domain.end());
	}

	void NeighborClassifierFilter::ready(PointTableRef)
	{
		m_newClass.clear();
	}

	void NeighborClassifierFilter::doOneNoDomain(PointRef& point, PointRef& temp,
		KD3Index& kdi)
	{
		PointIdList iSrc = kdi.neighbors(point, m_k);
		double thresh = iSrc.size() / 2.0;

		// vote NNs
		using CountMap = std::map<int, unsigned int>;
		CountMap counts;
		//std::map<int, unsigned int> counts;
		for (PointId id : iSrc)
		{
			temp.setPointId(id);
			counts[temp.getFieldAs<int>(m_dim)]++;
		}

		// pick winner of the vote
		auto pr = *std::max_element(counts.begin(), counts.end(),
			[](CountMap::const_reference p1, CountMap::const_reference p2)
		{ return p1.second < p2.second; });

		// update point
		auto oldclass = point.getFieldAs<double>(m_dim);
		auto newclass = pr.first;
		if (pr.second > thresh && oldclass != newclass)
			m_newClass[point.pointId()] = newclass;
	}

	// update point.  kdi and temp both reference the NN point cloud
	bool NeighborClassifierFilter::doOne(PointRef& point, PointRef& temp,
		KD3Index& kdi)
	{
		if (m_domain.empty())  // No domain, process all points
			doOneNoDomain(point, temp, kdi);

		for (DimRange& r : m_domain)
		{   // process only points that satisfy a domain condition
			if (r.valuePasses(point.getFieldAs<double>(r.m_id)))
			{
				doOneNoDomain(point, temp, kdi);
				break;
			}
		}
		return true;
	}

	PointViewPtr NeighborClassifierFilter::loadSet(const std::string& filename, PointTableRef table)
	{
		PipelineManager mgr;

		Stage& reader = mgr.makeReader(filename, "");
		reader.prepare(table);
		PointViewSet viewSet = reader.execute(table);
		assert(viewSet.size() == 1);
		return *viewSet.begin();
	}

	void NeighborClassifierFilter::filter(PointView& view)
	{
		PointRef point_src(view, 0);
		if (m_candidateFile.empty())
		{   // No candidate file so NN comes from src file
			KD3Index& kdiSrc = view.build3dIndex();
			PointRef point_nn(view, 0);
			for (PointId id = 0; id < view.size(); ++id)
			{
				point_src.setPointId(id);
				doOne(point_src, point_nn, kdiSrc);
			}
		}
		else
		{   // NN comes from candidate file
			ColumnPointTable candTable;
			PointViewPtr candView = loadSet(m_candidateFile, candTable);
			KD3Index& kdiCand = candView->build3dIndex();
			PointRef point_nn(*candView, 0);
			for (PointId id = 0; id < view.size(); ++id)
			{
				point_src.setPointId(id);
				doOne(point_src, point_nn, kdiCand);
			}
		}

		for (auto& p : m_newClass)
			view.setField(m_dim, p.first, p.second);
	}
}

