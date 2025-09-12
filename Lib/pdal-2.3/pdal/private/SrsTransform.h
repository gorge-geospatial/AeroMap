/******************************************************************************
 * Copyright (c) 2019, Hobu Inc.
 ****************************************************************************/
#pragma once

#include <pdal/pdal_internal.h>

class OGRCoordinateTransformation;
class OGRSpatialReference;

namespace pdal
{
	class SpatialReference;

	class SrsTransform
	{
	public:
		/// Object that performs transformation from a \src spatial reference
		/// to a \dest spatial reference.
		SrsTransform();
		SrsTransform(const SrsTransform& src);
		SrsTransform(SrsTransform&& src);
		SrsTransform& operator=(SrsTransform&& src);
		SrsTransform(const OGRSpatialReference& src, const OGRSpatialReference& dst);
		SrsTransform(const SpatialReference& src, const SpatialReference& dst);
		SrsTransform(const SpatialReference& src, std::vector<int> srcOrder,
			const SpatialReference& dst, std::vector<int> dstOrder);
		~SrsTransform();

		void set(OGRSpatialReference src, OGRSpatialReference dst);
		void set(const SpatialReference& src, const SpatialReference& dst);

		/// Get the underlying transformation.
		/// \return  Pointer to the underlying coordinate transform.
		OGRCoordinateTransformation* get() const;

		/// Transform the X, Y and Z of a point in place.
		/// \param x  X coordinate
		/// \param y  Y coordinate
		/// \param z  Z coordinate
		/// \return  True if the transformation was successful
		bool transform(double& x, double& y, double& z) const;

		/// Transform a set of points in place.
		/// \param x  X coordinates
		/// \param y  Y coordinates
		/// \param z  Z coordinates
		/// \return  True if the transformation was successful
		bool transform(std::vector<double>& x, std::vector<double>& y,
			std::vector<double>& z) const;

		/// Determine if this represents a valid transform.
		/// \return  Whether the transform is valid or not.
		bool valid() const
		{
			return (bool)m_transform.get();
		}

	private:
		std::unique_ptr<OGRCoordinateTransformation> m_transform;
	};
}
