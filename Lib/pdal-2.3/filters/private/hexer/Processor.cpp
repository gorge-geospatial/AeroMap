/******************************************************************************
 * Copyright (c) 2014, Hobu Inc. (howard@hobu.co)
 ****************************************************************************/

#include <assert.h>
#include <sstream>
#include <cmath>

#include "Processor.h"
#include "HexGrid.h"
#include "Mathpair.h"

namespace hexer
{
	double distance(const Point& p1, const Point& p2)
	{
		double xdist = p2.m_x - p1.m_x;
		double ydist = p2.m_y - p1.m_y;
		return std::sqrt(xdist * xdist + ydist * ydist);
	}

	// Compute hex size based on distance between consecutive points and
	// density.  The probably needs some work based on more data.
	double computeHexSize(const std::vector<Point>& samples, int density)
	{
		double dist = 0;
		for (std::vector<Point>::size_type i = 0; i < samples.size() - 1; ++i)
		{
			Point p1 = samples[i];
			Point p2 = samples[i + 1];
			dist += distance(p1, p2);
		}
		return ((density * dist) / samples.size());
	}

	void process(HexGrid* grid, PointReader reader)
	{
		double x, y;
		void* context;

		while (reader(x, y, context))
			grid->addPoint(x, y);
		grid->findShapes();
		grid->findParentPaths();
	}

	void processHexes(HexGrid* grid, HexReader reader)
	{
		int x, y;
		void* ctx;

		assert(grid->width() > 0);
		assert(grid->denseLimit() < 0);

		while (reader(x, y, ctx))
			grid->addDenseHexagon(x, y);
		grid->findShapes();
		grid->findParentPaths();
	}
}
