/******************************************************************************
* Copyright (c) 2020, Hobu Inc.
****************************************************************************/

#include "Raster.h"

namespace pdal
{
	// This should probably call expand().
	template <typename T>
	void Raster<T>::setLimits(const RasterLimits& limits)
	{
		m_limits = limits;
		DataVec dataVec(width() * height(), m_initializer);
		m_data.swap(dataVec);
	}


	template <typename T>
	void Raster<T>::expandToInclude(double x, double y)
	{
		int xi = xCell(x);
		int yi = yCell(y);

		if (xi >= 0 && yi >= 0 && xi < width() && yi < height())
			return;

		int w = (std::max)(width(), xi + 1);
		int h = (std::max)(height(), yi + 1);
		int xshift = (std::max)(-xi, 0);
		int yshift = (std::max)(-yi, 0);

		if (xshift)
			w += xshift;
		if (yshift)
			h += yshift;
		expand(w, h, xshift, yshift);
	}


	template <typename T>
	Utils::StatusWithReason Raster<T>::expand(int newWidth, int newHeight, int xshift, int yshift)
	{
		if (newWidth < width())
			return { -1, "Expanded grid must have width at least as large as existing grid." };
		if (newHeight < height())
			return { -1, "Expanded grid must have height at least as large as existing grid." };
		if (width() + xshift > newWidth || height() + yshift > newHeight)
			return { -1, "Can't shift existing grid outside of new grid during expansion." };
		if (newWidth == width() && newHeight == height())
			return true;

		m_limits.xOrigin -= xshift * edgeLength();
		m_limits.yOrigin -= yshift * edgeLength();

		// Raster works upside down from cartesian X/Y
		yshift = newHeight - (height() + yshift);

		auto dstIndex = [newWidth, xshift, yshift](size_t i, size_t j)
		{
			return ((yshift + j) * newWidth) + i + xshift;
		};

		// Note: that i, j are internal to the raster and start at the top left and
		//   move across and down.
		DataVec& src = m_data;
		DataVec dst(newWidth * newHeight, m_initializer);
		for (int j = 0; j < height(); ++j)
		{
			size_t srcPos = index(0, j);
			size_t dstPos = dstIndex(0, j);
			std::copy(src.begin() + srcPos, src.begin() + srcPos + width(),
				dst.begin() + dstPos);
		}
		m_data = std::move(dst);
		m_limits.width = newWidth;
		m_limits.height = newHeight;
		return true;
	}

	// Instantiate Raster<double>
	template class Raster<double>;
}
