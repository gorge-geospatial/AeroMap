#ifndef HEXERPATH_H
#define HEXERPATH_H

#include <vector>
#include <ostream>

#include "Mathpair.h"
#include "Segment.h"

namespace hexer
{
	enum Orientation
	{
		CLOCKWISE,     // Outer
		ANTICLOCKWISE  // Hole
	};

	class HexGrid;
	class Path;
	using PathPtrList = std::vector<Path*>;

	class Path
	{
	public:
		Path(HexGrid* m_grid, Orientation orient) :
			m_grid(m_grid), m_parent(NULL), m_orientation(orient)
		{}

		~Path()
		{
			for (auto p : m_children)
				delete p;
		}

		void push_back(const Segment& s)
		{
			m_segs.push_back(s);
		}
		Segment rootSegment()
		{
			return m_segs[0];
		}
		Path* parent()
		{
			return m_parent;
		}
		void setParent(Path* p)
		{
			m_parent = p;
		}
		void addChild(Path* p)
		{
			m_children.push_back(p);
		}
		void finalize(Orientation o)
		{
			m_orientation = o;
			for (size_t i = 0; i < m_children.size(); ++i)
				m_children[i]->finalize(o == CLOCKWISE ? ANTICLOCKWISE : CLOCKWISE);
		}
		size_t pathLength() const
		{
			return m_segs.size();
		}
		Point getPoint(size_t pointnum) const;
		Orientation orientation() const
		{
			return m_orientation;
		}
		std::vector<Point> points() const;
		PathPtrList subPaths() const
		{
			return m_children;
		}
		void toWKT(std::ostream& output) const;

	private:
		/// Grid that owns the path.
		HexGrid* m_grid;
		/// Parent path (NULL if root path)
		Path* m_parent;
		/// Children
		PathPtrList m_children;
		/// Orientation of path AT EXTRACTION - segments are ALWAYS ordered
		/// clockwise.
		Orientation m_orientation;
		/// List of segments that make up the path.
		std::vector<Segment> m_segs;

		void writeRing(std::ostream& out) const;
		PathPtrList writePolygon(std::ostream& out) const;
	};
}

#endif // #ifndef HEXERPATH_H
