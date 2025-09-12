/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
* Copyright (c) 2013, Howard Butler (hobu.inc@gmail.com)
****************************************************************************/

#include "DeltaKernel.h"

#include <pdal/Stage.h>
#include <pdal/PDALUtils.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"kernels.delta",
		"Delta Kernel",
		"http://pdal.io/apps/delta.html"
	};

	CREATE_STATIC_KERNEL(DeltaKernel, s_info)

	std::string DeltaKernel::getName() const { return s_info.name; }

	DeltaKernel::DeltaKernel() : m_detail(false), m_allDims(false)
	{}

	void DeltaKernel::addSwitches(ProgramArgs& args)
	{
		Arg& src = args.add("source", "source file name", m_sourceFile);
		src.setPositional();
		Arg& candidate = args.add("candidate", "candidate file name", m_candidateFile);
		candidate.setPositional();
		args.add("detail", "Output deltas per-point", m_detail);
		args.add("alldims", "Compute diffs for all dimensions (not just X,Y,Z)", m_allDims);
	}

	PointViewPtr DeltaKernel::loadSet(const std::string& filename, PointTableRef table)
	{
		Stage& reader = makeReader(filename, m_driverOverride);
		reader.prepare(table);
		PointViewSet viewSet = reader.execute(table);
		assert(viewSet.size() == 1);
		return *viewSet.begin();
	}

	int DeltaKernel::execute()
	{
		ColumnPointTable srcTable;
		ColumnPointTable candTable;
		DimIndexMap dims;

		PointViewPtr srcView = loadSet(m_sourceFile, srcTable);
		PointViewPtr candView = loadSet(m_candidateFile, candTable);

		PointLayoutPtr srcLayout = srcTable.layout();
		PointLayoutPtr candLayout = candTable.layout();

		Dimension::IdList ids = srcLayout->dims();
		for (Dimension::Id dim : ids)
		{
			std::string name = srcLayout->dimName(dim);
			if (!m_allDims)
				if (name != "X" && name != "Y" && name != "Z")
					continue;
			DimIndex d;
			d.m_name = name;
			d.m_srcId = dim;
			dims[name] = d;
		}
		ids = candLayout->dims();
		for (Dimension::Id dim : ids)
		{
			std::string name = candLayout->dimName(dim);
			auto di = dims.find(name);
			if (di == dims.end())
				continue;
			DimIndex& d = di->second;
			d.m_candId = dim;
		}

		// Remove dimensions that aren't in both the source and candidate lists.
		for (auto di = dims.begin(); di != dims.end();)
		{
			DimIndex& d = di->second;
			if (d.m_candId == Dimension::Id::Unknown)
				dims.erase(di++);
			else
				++di;
		}

		// Index the candidate data.
		KD3Index index(*candView);
		index.build();

		MetadataNode root;

		if (m_detail)
			root = dumpDetail(srcView, candView, index, dims);
		else
			root = dump(srcView, candView, index, dims);
		Utils::toJSON(root, std::cout);

		return 0;
	}


	MetadataNode DeltaKernel::dump(PointViewPtr& srcView, PointViewPtr& candView,
		KD3Index& index, DimIndexMap& dims)
	{
		MetadataNode root;

		for (PointId id = 0; id < srcView->size(); ++id)
		{
			PointRef point = srcView->point(id);
			PointId candId = index.neighbor(point);

			// It may be faster to put in a special case to avoid having to
			// fetch X, Y and Z, more than once but this is simpler and
			// I'm thinking in most cases it will make no practical difference.
			for (auto di = dims.begin(); di != dims.end(); ++di)
			{
				DimIndex& d = di->second;
				double sv = srcView->getFieldAs<double>(d.m_srcId, id);
				double cv = candView->getFieldAs<double>(d.m_candId, candId);
				accumulate(d, sv - cv);
			}
		}

		root.add("source", m_sourceFile);
		root.add("candidate", m_candidateFile);
		for (auto dpair : dims)
		{
			DimIndex& d = dpair.second;

			MetadataNode dimNode = root.add(d.m_name);
			dimNode.add("min", d.m_min);
			dimNode.add("max", d.m_max);
			dimNode.add("mean", d.m_avg);
		}
		return root;
	}


	void DeltaKernel::accumulate(DimIndex& d, double v)
	{
		d.m_cnt++;
		d.m_min = (std::min)(v, d.m_min);
		d.m_max = (std::max)(v, d.m_max);
		d.m_avg += (v - d.m_avg) / d.m_cnt;
	}


	MetadataNode DeltaKernel::dumpDetail(PointViewPtr& srcView,
		PointViewPtr& candView, KD3Index& index, DimIndexMap& dims)
	{
		MetadataNode root;

		for (PointId id = 0; id < srcView->size(); ++id)
		{
			PointRef point = srcView->point(id);
			PointId candId = index.neighbor(point);

			MetadataNode delta = root.add("delta");
			delta.add("i", id);
			for (auto di = dims.begin(); di != dims.end(); ++di)
			{
				DimIndex& d = di->second;
				double sv = srcView->getFieldAs<double>(d.m_srcId, id);
				double cv = candView->getFieldAs<double>(d.m_candId, candId);

				delta.add(d.m_name, sv - cv);
			}
		}
		return root;
	}
}
