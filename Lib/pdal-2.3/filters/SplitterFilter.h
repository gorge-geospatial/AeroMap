/******************************************************************************
 * Copyright (c) 2014, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#ifndef SPLITTERFILTER_H
#define SPLITTERFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class SplitterFilter : public pdal::Filter
	{
	private:
		//This used to be a lambda, but the VS compiler exploded, I guess.
		typedef std::pair<int, int> Coord;
		class CoordCompare
		{
		public:
			bool operator () (const Coord& c1, const Coord& c2) const
			{
				return c1.first < c2.first ? true :
					c1.first > c2.first ? false :
					c1.second < c2.second ? true :
					false;
			};
		};

	public:
		SplitterFilter();
		using PointAdder = std::function<void(PointRef&, int, int)>;

		std::string getName() const;
		void setOrigin(double xOrigin, double yOrigin);
		void processPoint(PointRef& p, PointAdder adder);
		PointViewPtr view(const Coord& c);

		// Return the bounds of a tile.
		BOX2D bounds(const Coord& c) const;
		// Return the buffered bounds of a tile.
		BOX2D bufferedBounds(const Coord& c) const;
		// Return the coordinate extent of tiles.  Note that the return type
		// will contain integer-valued doubles.
		BOX2D extent() const;

	private:
		double m_length;
		double m_xOrigin;
		double m_yOrigin;
		double m_buffer;
		std::map<Coord, PointViewPtr, CoordCompare> m_viewMap;

		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual PointViewSet run(PointViewPtr view);
		bool squareContains(int xpos, int ypos, double x, double y) const;

		SplitterFilter& operator=(const SplitterFilter&); // not implemented
		SplitterFilter(const SplitterFilter&); // not implemented
	};
}

#endif // #ifndef SPLITTERFILTER_H
