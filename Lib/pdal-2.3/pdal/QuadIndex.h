#ifndef QUADINDEX_H
#define QUADINDEX_H

#include <vector>
#include <memory>

#include <pdal/pdal_types.h>
#include <pdal/util/Bounds.h>

namespace pdal
{

	class PointView;

	struct Point
	{
		Point(double x, double y) : x(x), y(y) { }
		Point(const Point& other) : x(other.x), y(other.y) { }

		// Calculates the distance-squared to another point.
		double sqDist(const Point& other) const
		{
			return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
		}

		const double x;
		const double y;

		Point& operator=(const Point&); // not implemented
	};

	struct QuadPointRef
	{
		QuadPointRef(const Point& point, std::size_t pbIndex)
			: point(point)
			, pbIndex(pbIndex)
		{ }

		const Point point;
		const std::size_t pbIndex;

		QuadPointRef& operator=(const QuadPointRef&); // not implemented
		QuadPointRef(const QuadPointRef&); // not implemented
	};

	class QuadIndex
	{
	public:
		QuadIndex(const PointView& view, std::size_t topLevel = 0);
		QuadIndex(
			const PointView& view,
			double xMin,
			double yMin,
			double xMax,
			double yMax,
			std::size_t topLevel = 0);
		QuadIndex(
			const std::vector<std::shared_ptr<QuadPointRef> >& points,
			double xMin,
			double yMin,
			double xMax,
			double yMax,
			std::size_t topLevel = 0);
		~QuadIndex();

		void getBounds(
			double& xMin,
			double& yMin,
			double& xMax,
			double& yMax) const;

		std::size_t getDepth() const;

		std::vector<std::size_t> getFills() const;

		// Return all points at depth levels strictly less than depthEnd.
		// A depthEnd value of zero returns all points in the tree.
		PointIdList getPoints(
			std::size_t depthEnd = 0) const;

		// Return all points at depth levels between [depthBegin, depthEnd).
		// A depthEnd value of zero will return all points at levels >= depthBegin.
		PointIdList getPoints(
			std::size_t depthBegin,
			std::size_t depthEnd) const;

		// Rasterize a single level of the tree.  Empty positions will contain
		// std::numeric_limits<PointId>::max().
		PointIdList getPoints(
			std::size_t rasterize,
			double& xBegin,
			double& xEnd,
			double& xStep,
			double& yBegin,
			double& yEnd,
			double& yStep) const;

		// Get custom raster via bounds and resolution query.  Empty positions will
		// contain std::numeric_limits<PointId>::max().
		PointIdList getPoints(
			double xBegin,
			double xEnd,
			double xStep,
			double yBegin,
			double yEnd,
			double yStep) const;

		// Return all points within the query bounding box, searching only up to
		// depth levels strictly less than depthEnd.
		// A depthEnd value of zero will return all existing points that fall
		// within the query range regardless of depth.
		PointIdList getPoints(
			double xMin,
			double yMin,
			double xMax,
			double yMax,
			std::size_t depthEnd = 0) const;

		PointIdList getPoints(
			const BOX3D& box,
			std::size_t depthEnd = 0) const
		{
			return getPoints(box.minx, box.miny, box.maxx, box.maxy, depthEnd);
		}

		// Return all points within the bounding box, searching at tree depth
		// levels from [depthBegin, depthEnd).
		// A depthEnd value of zero will return all points within the query range
		// that have a tree level >= depthBegin.
		PointIdList getPoints(
			double xMin,
			double yMin,
			double xMax,
			double yMax,
			std::size_t depthBegin,
			std::size_t depthEnd) const;

	private:
		struct QImpl;
		std::unique_ptr<QImpl> m_qImpl;

		// Disable copying and assignment.
		QuadIndex(const QuadIndex&);
		QuadIndex& operator=(QuadIndex&);
	};

}

#endif // #ifndef QUADINDEX_H
