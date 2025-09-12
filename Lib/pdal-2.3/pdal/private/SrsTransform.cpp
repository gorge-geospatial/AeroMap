/******************************************************************************
 * Copyright (c) 2019, Hobu Inc.
 ****************************************************************************/

#include "SrsTransform.h"
#include <pdal/SpatialReference.h>

#include <ogr_spatialref.h>

namespace pdal
{
	SrsTransform::SrsTransform()
	{}

	SrsTransform::SrsTransform(const SrsTransform& src)
	{
		set(*(src.m_transform->GetSourceCS()), *(src.m_transform->GetTargetCS()));
	}

	SrsTransform::SrsTransform(SrsTransform&& src)
		: m_transform(std::move(src.m_transform))
	{}

	SrsTransform::~SrsTransform()
	{}

	SrsTransform& SrsTransform::operator=(SrsTransform&& src)
	{
		m_transform = std::move(src.m_transform);
		return *this;
	}

	SrsTransform::SrsTransform(const SpatialReference& src, const SpatialReference& dst)
	{
		set(src, dst);
	}

	SrsTransform::SrsTransform(const OGRSpatialReference& srcRef, const OGRSpatialReference& dstRef)
	{
		set(srcRef, dstRef);
	}

	SrsTransform::SrsTransform(const SpatialReference& src,
		std::vector<int> srcOrder,
		const SpatialReference& dst,
		std::vector<int> dstOrder)
	{
		OGRSpatialReference srcRef(src.getWKT().data());
		OGRSpatialReference dstRef(dst.getWKT().data());

		// Starting with version 3, the axes (X, Y, Z or lon, lat, h or whatever)
		// are mapped according to the WKT definition.  In particular, this means
		// that for EPSG:4326 the mapping is X -> lat, Y -> lon, rather than the
		// more conventional X -> lon, Y -> lat.  Setting this flag reverses things
		// such that the traditional ordering is maintained.  There are other
		// SRSes where this comes up.  See "axis order issues" in the GDAL WKT2
		// discussion for more info.
		//
		if (srcOrder.size())
			srcRef.SetDataAxisToSRSAxisMapping(srcOrder);
		if (dstOrder.size())
			dstRef.SetDataAxisToSRSAxisMapping(dstOrder);
		m_transform.reset(OGRCreateCoordinateTransformation(&srcRef, &dstRef));
	}

	void SrsTransform::set(const SpatialReference& src, const SpatialReference& dst)
	{
		set(OGRSpatialReference(src.getWKT().data()), OGRSpatialReference(dst.getWKT().data()));
	}

	void SrsTransform::set(OGRSpatialReference src, OGRSpatialReference dst)
	{
		// Starting with version 3 of GDAL, the axes (X, Y, Z or lon, lat, h or whatever)
		// are mapped according to the WKT definition.  In particular, this means
		// that for EPSG:4326 the mapping is X -> lat, Y -> lon, rather than the
		// more conventional X -> lon, Y -> lat.  Setting this flag reverses things
		// such that the traditional ordering is maintained.  There are other
		// SRSes where this comes up.  See "axis order issues" in the GDAL WKT2
		// discussion for more info.
		//
		src.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		dst.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
		m_transform.reset(OGRCreateCoordinateTransformation(&src, &dst));
	}

	OGRCoordinateTransformation* SrsTransform::get() const
	{
		return m_transform.get();
	}

	bool SrsTransform::transform(double& x, double& y, double& z) const
	{
		return m_transform && m_transform->Transform(1, &x, &y, &z);
	}

	bool SrsTransform::transform(std::vector<double>& x, std::vector<double>& y,
		std::vector<double>& z) const
	{
		if (x.size() != y.size() && y.size() != z.size())
			throw pdal_error("SrsTransform::called with vectors of different "
				"sizes.");
		int err = m_transform->Transform((int)x.size(), x.data(), y.data(), z.data());
		return (err == OGRERR_NONE);
	}
}
