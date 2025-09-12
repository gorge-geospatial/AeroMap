#ifndef GDALGRID_H
#define GDALGRID_H

#include <math.h>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

#include <pdal/pdal_internal.h>
#include <pdal/private/Raster.h>

namespace pdal
{

	class GDALGrid
	{
		FRIEND_TEST(GDALWriterTest, issue_2095);
	public:
		static const int statCount = 1;
		static const int statMin = 2;
		static const int statMax = 4;
		static const int statMean = 8;
		static const int statStdDev = 16;
		static const int statIdw = 32;

		struct error : public std::runtime_error
		{
			error(const std::string& err) : std::runtime_error(err)
			{}
		};

		// Exported for testing.
		GDALGrid(double xOrigin, double yOrigin, size_t width, size_t height,
			double edgeLength, double radius, int outputTypes, size_t windowSize,
			double power);

		void expandToInclude(double x, double y);

		// Get the number of bands represented by this grid.
		int numBands() const;

		// Return a pointer to the data in a raster band, row-major ordered.
		double* data(const std::string& name);

		// Add a point to the raster grid.
		void addPoint(double x, double y, double z);

		// Compute final values after all points have been added.
		void finalize();

		int width() const;
		int height() const;
		double xOrigin() const;
		double yOrigin() const;

	private:
		int m_windowSize;
		double m_edgeLength;
		double m_radius;
		double m_power;

		typedef std::unique_ptr<Rasterd> DataPtr;
		DataPtr m_count;
		DataPtr m_min;
		DataPtr m_max;
		DataPtr m_mean;
		DataPtr m_stdDev;
		DataPtr m_idw;
		DataPtr m_idwDist;

		int m_outputTypes;

		// Determine if a cell i, j has no associated points.
		bool empty(int i, int j) const
		{
			return m_count->at(i, j) <= 0;
		}

		// Determine if a cell with index \c idx has no associated points.
		bool empty(size_t idx) const
		{
			return (m_count->at(idx) <= 0);
		}

		// Determine the distance from the center of cell at coordinate i, j to
		// a point at absolute coordinate x, y.
		double distance(int i, int j, double x, double y) const;

		// Update cells in the Nth quadrant about point at (x, y, z)
		void updateFirstQuadrant(double x, double y, double z);
		void updateSecondQuadrant(double x, double y, double z);
		void updateThirdQuadrant(double x, double y, double z);
		void updateFourthQuadrant(double x, double y, double z);

		// Update cell at i, j with value at a distance.
		void update(size_t i, size_t j, double val, double dist);

		// Fill cell at index \c i with the nondata value.
		// \i  I coordinate.
		// \j  J coordinate.
		void fillNodata(int i, int j);

		// Fill an empty cell with a value inverse-distance averaged from
		// surrounding cells.
		void windowFill();

		// Fill empty cell at dstI, dstJ with inverse-distance weighted values
		// from neighboring cells.
		void windowFill(int dstI, int dstJ);

		// Cumulate data from a source cell to a destination cell when doing
		// a window fill.
		void windowFillCell(int srcI, int srcJ, int dstI, int dstJ, double distance);
	};
}

#endif // #ifndef GDALGRID_H
