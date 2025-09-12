/******************************************************************************
 * Copyright (c) 2014, Hobu Inc. (howard@hobu.co)
 ****************************************************************************/
#ifndef HEXINFO_H
#define HEXINFO_H

namespace hexer
{
	class HexInfo
	{
	public:
		Point m_center;
		Coord m_pos;
		int m_density;

		int density() const
		{
			return m_density;
		}
		int xgrid() const
		{
			return m_pos.m_x;
		}
		int ygrid() const
		{
			return m_pos.m_y;
		}
		double x() const
		{
			return m_center.m_x;
		}
		double y() const
		{
			return m_center.m_y;
		}
	};
}

#endif // #ifndef HEXINFO_H
