/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
****************************************************************************/

#pragma once

#include <pdal/PointView.h>

namespace pdal
{
	class KD2Impl;
	class KD3Impl;
	class KDFlexImpl;

	class KD2Index
	{
	public:
		KD2Index(const PointView& buf);
		~KD2Index();

		void build();
		PointId neighbor(double x, double y) const;
		PointId neighbor(PointId idx) const;
		PointId neighbor(PointRef& point) const;
		PointIdList neighbors(double x, double y, point_count_t k) const;
		PointIdList neighbors(PointId idx, point_count_t k) const;
		PointIdList neighbors(PointRef& point, point_count_t k) const;
		void knnSearch(double x, double y, point_count_t k,
			PointIdList* indices, std::vector<double>* sqr_dists) const;
		void knnSearch(PointId idx, point_count_t k, PointIdList* indices,
			std::vector<double>* sqr_dists) const;
		void knnSearch(PointRef& point, point_count_t k, PointIdList* indices,
			std::vector<double>* sqr_dists) const;
		PointIdList radius(double const& x, double const& y,
			double const& r) const;
		PointIdList radius(PointId idx, double const& r) const;
		PointIdList radius(PointRef& point, double const& r) const;

	private:
		const PointView& m_buf;
		std::unique_ptr<KD2Impl> m_impl;
	};

	class KD3Index
	{
	public:
		KD3Index(const PointView& buf);
		~KD3Index();

		void build();
		PointId neighbor(double x, double y, double z) const;
		PointId neighbor(PointId idx) const;
		PointId neighbor(PointRef& point) const;
		PointIdList neighbors(double x, double y, double z,
			point_count_t k, size_t stride = 1) const;
		PointIdList neighbors(PointId idx, point_count_t k,
			size_t stride = 1) const;
		PointIdList neighbors(PointRef& point, point_count_t k,
			size_t stride = 1) const;
		void knnSearch(double x, double y, double z, point_count_t k,
			PointIdList* indices, std::vector<double>* sqr_dists) const;
		void knnSearch(PointId idx, point_count_t k, PointIdList* indices,
			std::vector<double>* sqr_dists) const;
		void knnSearch(PointRef& point, point_count_t k,
			PointIdList* indices, std::vector<double>* sqr_dists) const;
		PointIdList radius(double x, double y, double z, double r) const;
		PointIdList radius(PointId idx, double r) const;
		PointIdList radius(PointRef& point, double r) const;

	private:
		const PointView& m_buf;
		std::unique_ptr<KD3Impl> m_impl;
	};

	class KDFlexIndex
	{
	public:
		KDFlexIndex(const PointView& buf, const Dimension::IdList& dims);
		~KDFlexIndex();

		void build();
		PointId neighbor(PointRef& point) const;
		PointIdList neighbors(PointRef& point, point_count_t k, size_t stride = 1) const;
		PointIdList radius(PointId idx, double r) const;

	private:
		const PointView& m_buf;
		const Dimension::IdList& m_dims;
		std::unique_ptr<KDFlexImpl> m_impl;
	};
}
