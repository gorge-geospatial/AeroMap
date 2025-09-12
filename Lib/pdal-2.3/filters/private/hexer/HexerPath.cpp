/******************************************************************************
 * Copyright (c) 2014, Hobu Inc. (howard@hobu.co)
 ****************************************************************************/

#include <cassert>

#include "HexerPath.h"

using namespace std;

namespace hexer
{
	Point Path::getPoint(size_t pointnum) const
	{
		pointnum = (m_orientation == ANTICLOCKWISE) ?
			m_segs.size() - pointnum - 1 : pointnum;
		return m_segs[pointnum].startPos(m_grid);
	}

	vector<Point> Path::points() const
	{
		vector<Point> points;
		if (m_orientation == CLOCKWISE)
		{
			for (size_t i = 0; i < m_segs.size(); ++i)
				points.push_back(m_segs[i].startPos(m_grid));
			points.push_back(m_segs[0].startPos(m_grid));
		}
		else
		{
			// Note that i will wrap to max of size_t when decrementing 0.
			for (size_t i = m_segs.size() - 1; i < m_segs.size(); --i)
				points.push_back(m_segs[i].startPos(m_grid));
			points.push_back(m_segs[m_segs.size() - 1].startPos(m_grid));

		}
		return points;
	}

	void Path::writeRing(std::ostream& out) const
	{
		auto outputPoint = [&out](const Point& p)
		{
			out << p.m_x << " " << p.m_y;
		};

		const vector<Point>& pts = points();
		assert(pts.size() > 2);
		out << "(";
		outputPoint(pts.front());
		for (auto it = pts.begin() + 1; it != pts.end(); ++it)
		{
			out << ", ";
			outputPoint(*it);
		}
		out << ")";
	}

	// WKT (or GeoJSON) doesn't allow nesting of polygons.  You can just have
	// polygons and holes.  Islands within the holes need to be described as
	// separate polygons.  To that end, we gather the islands from all holes
	// and return them to be processed as separate polygons.
	PathPtrList Path::writePolygon(std::ostream& out) const
	{
		PathPtrList islands;

		out << "(";
		writeRing(out);
		const PathPtrList& paths = subPaths();
		for (auto& p : paths)
		{
			out << ", ";
			p->writeRing(out);
			const PathPtrList& subs(p->subPaths());
			islands.insert(islands.end(), subs.begin(), subs.end());
		}
		out << ")";
		return islands;
	}

	void Path::toWKT(std::ostream& out) const
	{
		PathPtrList islands = writePolygon(out);

		// See the note on writePolygon()
		while (islands.size())
		{
			PathPtrList paths;
			paths.swap(islands);
			for (Path* p : paths)
			{
				out << ", ";
				PathPtrList subIslands = p->writePolygon(out);
				islands.insert(islands.end(), subIslands.begin(), subIslands.end());
			}
		}
	}
}
