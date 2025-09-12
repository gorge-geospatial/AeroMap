/******************************************************************************
 * Copyright (c) 2016, Hobu Inc.
 ****************************************************************************/

#pragma once

#include <pdal/Dimension.h>

namespace pdal
{
	namespace Dimension
	{
		class Detail
		{
		public:
			Detail() : m_id(Id::Unknown), m_offsetOrOrder(-1), m_type(Type::None)
			{}
			//NOTE - This is strange, but for some reason things run faster with
			// this NOOP virtual dtor.  Perhaps it has something to do with
			// an inlining optimization or perhaps alignment (though a void * doesn't
			// cause the same performance improvement) It may help on no machine
			// except mine, but it doesn't hurt anything, either.
			virtual ~Detail()
			{}

			void setOffset(int offset)
			{
				m_offsetOrOrder = offset;
			}
			void setOrder(int order)
			{
				m_offsetOrOrder = order;
			}
			void setType(Type type)
			{
				m_type = type;
			}
			void setId(Id id)
			{
				m_id = id;
			}
			Id id() const
			{
				return m_id;
			}
			int offset() const
			{
				return m_offsetOrOrder;
			}
			int order() const
			{
				return m_offsetOrOrder;
			}
			Type type() const
			{
				return m_type;
			}
			size_t size() const
			{
				return Dimension::size(m_type);
			}
			BaseType base() const
			{
				return Dimension::base(m_type);
			}

		private:
			Id m_id;
			int m_offsetOrOrder;
			Type m_type;
		};
		typedef std::vector<Detail> DetailList;
	}
}

