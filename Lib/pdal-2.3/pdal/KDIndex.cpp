/******************************************************************************
* Copyright (c) 2020, Hobu Inc.
****************************************************************************/

#include "KDIndex.h"
#include "private/KDImpl.h"

namespace pdal
{
	//
	// KD2Index
	//

	KD2Index::KD2Index(const PointView& buf) :
		m_buf(buf), m_impl(new KD2Impl(m_buf))
	{
		if (!m_buf.hasDim(Dimension::Id::X))
			throw pdal_error("KD2Index: point view missing 'X' dimension.");
		if (!m_buf.hasDim(Dimension::Id::Y))
			throw pdal_error("KD2Index: point view missing 'Y' dimension.");
	}

	KD2Index::~KD2Index()
	{}

	void KD2Index::build()
	{
		m_impl->build();
	}

	PointId KD2Index::neighbor(double x, double y) const
	{
		PointIdList ids = neighbors(x, y, 1);
		return (ids.size() ? ids[0] : 0);
	}


	PointId KD2Index::neighbor(PointId idx) const
	{
		PointIdList ids = neighbors(idx, 1);
		return (ids.size() ? ids[0] : 0);
	}

	PointId KD2Index::neighbor(PointRef& point) const
	{
		PointIdList ids = neighbors(point, 1);
		return (ids.size() ? ids[0] : 0);
	}

	PointIdList KD2Index::neighbors(double x, double y, point_count_t k) const
	{
		return m_impl->neighbors(x, y, k);
	}

	PointIdList KD2Index::neighbors(PointId idx, point_count_t k) const
	{
		double x = m_buf.getFieldAs<double>(Dimension::Id::X, idx);
		double y = m_buf.getFieldAs<double>(Dimension::Id::Y, idx);

		return neighbors(x, y, k);
	}

	PointIdList KD2Index::neighbors(PointRef& point, point_count_t k) const
	{
		double x = point.getFieldAs<double>(Dimension::Id::X);
		double y = point.getFieldAs<double>(Dimension::Id::Y);

		return neighbors(x, y, k);
	}

	void KD2Index::knnSearch(double x, double y, point_count_t k,
		PointIdList* indices, std::vector<double>* sqr_dists) const
	{
		m_impl->knnSearch(x, y, k, indices, sqr_dists);
	}

	void KD2Index::knnSearch(PointId idx, point_count_t k, PointIdList* indices,
		std::vector<double>* sqr_dists) const
	{
		double x = m_buf.getFieldAs<double>(Dimension::Id::X, idx);
		double y = m_buf.getFieldAs<double>(Dimension::Id::Y, idx);

		knnSearch(x, y, k, indices, sqr_dists);
	}

	void KD2Index::knnSearch(PointRef& point, point_count_t k, PointIdList* indices,
		std::vector<double>* sqr_dists) const
	{
		double x = point.getFieldAs<double>(Dimension::Id::X);
		double y = point.getFieldAs<double>(Dimension::Id::Y);

		knnSearch(x, y, k, indices, sqr_dists);
	}

	PointIdList KD2Index::radius(double const& x, double const& y,
		double const& r) const
	{
		return m_impl->radius(x, y, r);
	}

	PointIdList KD2Index::radius(PointId idx, double const& r) const
	{
		double x = m_buf.getFieldAs<double>(Dimension::Id::X, idx);
		double y = m_buf.getFieldAs<double>(Dimension::Id::Y, idx);

		return radius(x, y, r);
	}

	PointIdList KD2Index::radius(PointRef& point, double const& r) const
	{
		double x = point.getFieldAs<double>(Dimension::Id::X);
		double y = point.getFieldAs<double>(Dimension::Id::Y);

		return radius(x, y, r);
	}

	//
	// KD3Index
	//

	KD3Index::KD3Index(const PointView& buf) :
		m_buf(buf), m_impl(new KD3Impl(m_buf))
	{
		if (!m_buf.hasDim(Dimension::Id::X))
			throw pdal_error("KD3Index: point view missing 'X' dimension.");
		if (!m_buf.hasDim(Dimension::Id::Y))
			throw pdal_error("KD3Index: point view missing 'Y' dimension.");
		if (!m_buf.hasDim(Dimension::Id::Z))
			throw pdal_error("KD3Index: point view missing 'Z' dimension.");
	}

	KD3Index::~KD3Index()
	{}

	void KD3Index::build()
	{
		m_impl->build();
	}

	PointId KD3Index::neighbor(double x, double y, double z) const
	{
		PointIdList ids = neighbors(x, y, z, 1);
		return (ids.size() ? ids[0] : 0);
	}

	PointId KD3Index::neighbor(PointId idx) const
	{
		PointIdList ids = neighbors(idx, 1);
		return (ids.size() ? ids[0] : 0);
	}

	PointId KD3Index::neighbor(PointRef& point) const
	{
		PointIdList ids = neighbors(point, 1);
		return (ids.size() ? ids[0] : 0);
	}

	PointIdList KD3Index::neighbors(double x, double y, double z,
		point_count_t k, size_t stride) const
	{
		return m_impl->neighbors(x, y, z, k, stride);
	}

	PointIdList KD3Index::neighbors(PointId idx, point_count_t k,
		size_t stride) const
	{
		double x = m_buf.getFieldAs<double>(Dimension::Id::X, idx);
		double y = m_buf.getFieldAs<double>(Dimension::Id::Y, idx);
		double z = m_buf.getFieldAs<double>(Dimension::Id::Z, idx);

		return neighbors(x, y, z, k, stride);
	}

	PointIdList KD3Index::neighbors(PointRef& point, point_count_t k,
		size_t stride) const
	{
		double x = point.getFieldAs<double>(Dimension::Id::X);
		double y = point.getFieldAs<double>(Dimension::Id::Y);
		double z = point.getFieldAs<double>(Dimension::Id::Z);

		return neighbors(x, y, z, k, stride);
	}

	void KD3Index::knnSearch(double x, double y, double z, point_count_t k,
		PointIdList* indices, std::vector<double>* sqr_dists) const
	{
		m_impl->knnSearch(x, y, z, k, indices, sqr_dists);
	}

	void KD3Index::knnSearch(PointId idx, point_count_t k, PointIdList* indices,
		std::vector<double>* sqr_dists) const
	{
		double x = m_buf.getFieldAs<double>(Dimension::Id::X, idx);
		double y = m_buf.getFieldAs<double>(Dimension::Id::Y, idx);
		double z = m_buf.getFieldAs<double>(Dimension::Id::Z, idx);

		knnSearch(x, y, z, k, indices, sqr_dists);
	}

	void KD3Index::knnSearch(PointRef& point, point_count_t k,
		PointIdList* indices, std::vector<double>* sqr_dists) const
	{
		double x = point.getFieldAs<double>(Dimension::Id::X);
		double y = point.getFieldAs<double>(Dimension::Id::Y);
		double z = point.getFieldAs<double>(Dimension::Id::Z);

		knnSearch(x, y, z, k, indices, sqr_dists);
	}

	PointIdList KD3Index::radius(double x, double y, double z, double r) const
	{
		return m_impl->radius(x, y, z, r);
	}

	PointIdList KD3Index::radius(PointId idx, double r) const
	{
		double x = m_buf.getFieldAs<double>(Dimension::Id::X, idx);
		double y = m_buf.getFieldAs<double>(Dimension::Id::Y, idx);
		double z = m_buf.getFieldAs<double>(Dimension::Id::Z, idx);

		return radius(x, y, z, r);
	}

	PointIdList KD3Index::radius(PointRef& point, double r) const
	{
		double x = point.getFieldAs<double>(Dimension::Id::X);
		double y = point.getFieldAs<double>(Dimension::Id::Y);
		double z = point.getFieldAs<double>(Dimension::Id::Z);

		return radius(x, y, z, r);
	}

	//
	// KDFlexIndex
	//

	KDFlexIndex::KDFlexIndex(const PointView& buf, const Dimension::IdList& dims) :
		m_buf(buf), m_dims(dims), m_impl(new KDFlexImpl(m_buf, m_dims))
	{}

	KDFlexIndex::~KDFlexIndex()
	{}

	void KDFlexIndex::build()
	{
		m_impl->build();
	}

	PointId KDFlexIndex::neighbor(PointRef& point) const
	{
		PointIdList ids = neighbors(point, 1);
		return (ids.size() ? ids[0] : 0);
	}

	PointIdList KDFlexIndex::neighbors(PointRef& point, point_count_t k, size_t stride) const
	{
		return m_impl->neighbors(point, k, stride);
	}

	PointIdList KDFlexIndex::radius(PointId idx, double r) const
	{
		return m_impl->radius(idx, r);
	}
}
