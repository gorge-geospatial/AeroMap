/******************************************************************************
 * Project:  PDAL - http://pdal.org - A BSD library for point cloud data.
 * Purpose:  PDAL chipper class
 * Author:   Howard Butler, hobu.inc@gmail.com
 * Copyright (c) 2011, Howard Butler
 ****************************************************************************/

#pragma once

#include <pdal/Filter.h>
#include <pdal/PointView.h>
#include <vector>

namespace pdal
{
	class Stage;

	class ChipperFilter;

	class ChipPtRef
	{
		friend class ChipRefList;
		friend class ChipperFilter;

	private:
		double m_pos;
		point_count_t m_ptindex;
		size_t m_oindex;

	public:
		bool operator < (const ChipPtRef& pt) const
		{
			return m_pos < pt.m_pos;
		}
	};

	class ChipRefList
	{
		friend class ChipperFilter;

	private:
		std::vector<ChipPtRef> m_vec;

		std::vector<ChipPtRef>::size_type size() const
		{
			return m_vec.size();
		}
		void reserve(std::vector<ChipPtRef>::size_type n)
		{
			m_vec.reserve(n);
		}
		void resize(std::vector<ChipPtRef>::size_type n)
		{
			m_vec.resize(n);
		}
		void push_back(const ChipPtRef& ref)
		{
			m_vec.push_back(ref);
		}
		std::vector<ChipPtRef>::iterator begin()
		{
			return m_vec.begin();
		}
		std::vector<ChipPtRef>::iterator end()
		{
			return m_vec.end();
		}
		ChipPtRef& operator[](size_t pos)
		{
			return m_vec[pos];
		}
	};


	class ChipperFilter : public pdal::Filter
	{
	public:
		ChipperFilter() {}
		std::string getName() const;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual PointViewSet run(PointViewPtr view);

		void load(PointView& view, ChipRefList& xvec, ChipRefList& yvec, ChipRefList& spare);
		void partition(point_count_t size);
		void decideSplit(ChipRefList& v1, ChipRefList& v2, ChipRefList& spare, PointId left, PointId right);
		void split(ChipRefList& wide, ChipRefList& narrow, ChipRefList& spare, PointId left, PointId right);
		void emit(ChipRefList& wide, PointId widemin, PointId widemax);

		PointId m_threshold;
		PointViewPtr m_inView;
		PointViewSet m_outViews;
		PointIdList m_partitions;
		ChipRefList m_xvec;
		ChipRefList m_yvec;
		ChipRefList m_spare;

		ChipperFilter& operator=(const ChipperFilter&); // not implemented
		ChipperFilter(const ChipperFilter&); // not implemented
	};
}
