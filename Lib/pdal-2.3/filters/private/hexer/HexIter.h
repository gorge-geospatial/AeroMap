/******************************************************************************
 * Copyright (c) 2014, Hobu Inc. (howard@hobu.co)
 ****************************************************************************/
#ifndef HEXITER_H
#define HEXITER_H

#include "HexGrid.h"
#include "HexInfo.h"

namespace hexer
{
	class HexIter
	{
	public:
		HexIter(HexGrid::HexMap::iterator iter, HexGrid* grid) :
			m_iter(iter), m_grid(grid)
		{
			advance();
		}

		HexIter& operator++()
		{
			m_iter++;
			advance();
			return *this;
		}

		HexInfo operator*()
		{
			HexInfo info;
			Hexagon& hex = m_iter->second;
			info.m_pos.m_x = hex.x();
			info.m_pos.m_y = hex.y();
			info.m_center.m_x = m_grid->width() * hex.x();
			info.m_center.m_y = m_grid->height() * hex.y();
			if (hex.xodd())
				info.m_center.m_y += (m_grid->height() / 2);
			info.m_density = hex.count();
			return info;
		}

		bool operator == (const HexIter& iter)
		{
			return m_iter == iter.m_iter;
		}
		bool operator != (const HexIter& iter)
		{
			return m_iter != iter.m_iter;
		}

	private:
		void advance()
		{
			while (m_iter != m_grid->m_hexes.end())
			{
				if (m_iter->second.count())
					break;
				m_iter++;
			}
		}

		HexGrid::HexMap::iterator m_iter;
		HexGrid* m_grid;
	};
}

#endif // #ifndef HEXITER_H
