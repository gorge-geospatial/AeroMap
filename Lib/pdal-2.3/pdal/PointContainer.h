/******************************************************************************
* Copyright (c) 2015, Hobu Inc.
****************************************************************************/

#pragma once

#include <pdal/pdal_types.h>
#include <pdal/Dimension.h>
#include <pdal/PointLayout.h>

namespace pdal
{
	class PointContainer
	{
		friend class PointView;
		friend class PointRef;
	private:
		virtual void setFieldInternal(Dimension::Id dim, PointId idx, const void* val) = 0;
		virtual void getFieldInternal(Dimension::Id dim, PointId idx, void* val) const = 0;
		virtual void swapItems(PointId /*id1*/, PointId /*id2*/)
		{
			throw pdal_error("Can't swap items in this container.");
		}
		virtual void setItem(PointId /*dst*/, PointId /*src*/)
		{
			throw pdal_error("Can't set item in this container.");
		}
		virtual bool compare(Dimension::Id /*dim*/, PointId /*id1*/, PointId /*id2*/) const
		{
			throw pdal_error("Can't compare items in this container.");
		}
		virtual PointId getTemp(PointId id)
		{
			return id;
		}
		virtual void freeTemp(PointId /*id*/)
		{}
	public:
		virtual PointLayoutPtr layout() const = 0;
	};
}
