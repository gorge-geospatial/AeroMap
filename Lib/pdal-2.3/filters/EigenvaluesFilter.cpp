/******************************************************************************
* Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

#include "EigenvaluesFilter.h"

#include <pdal/KDIndex.h>
#include <pdal/util/ProgramArgs.h>
#include <pdal/private/MathUtils.h>

#include <Eigen/Dense>

#include <string>

namespace pdal
{
	using namespace Dimension;
	using namespace Eigen;

	static StaticPluginInfo const s_info
	{
		"filters.eigenvalues",
		"Returns the eigenvalues for a given point, based on its k-nearest neighbors.",
		"http://pdal.io/stages/filters.eigenvalues.html"
	};

	CREATE_STATIC_STAGE(EigenvaluesFilter, s_info)

	struct EigenvalueArgs
	{
		int m_knn;
		bool m_normalize;
		size_t m_stride;
		double m_radius;
		Arg* m_radiusArg;
		int m_minK;
	};

	EigenvaluesFilter::EigenvaluesFilter() : m_args(new EigenvalueArgs) {}

	std::string EigenvaluesFilter::getName() const
	{
		return s_info.name;
	}

	void EigenvaluesFilter::addArgs(ProgramArgs& args)
	{
		args.add("knn", "k-Nearest neighbors", m_args->m_knn, 8);
		args.add("normalize", "Normalize eigenvalues?", m_args->m_normalize, false);
		args.add("stride", "Compute features on strided neighbors",
			m_args->m_stride, size_t(1));
		m_args->m_radiusArg = &args.add(
			"radius", "Radius for nearest neighbor search", m_args->m_radius);
		args.add("min_k", "Minimum number of neighbors in radius", m_args->m_minK,
			3);
	}

	void EigenvaluesFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Id::Eigenvalue0);
		layout->registerDim(Id::Eigenvalue1);
		layout->registerDim(Id::Eigenvalue2);
	}

	void EigenvaluesFilter::prepared(PointTableRef table)
	{
		if (m_args->m_radiusArg->set())
		{
			log()->get(LogLevel::Warning)
				<< "Radius has been set. Ignoring knn and stride values."
				<< std::endl;
			if (m_args->m_radius <= 0.0)
				log()->get(LogLevel::Error)
				<< "Radius must be greater than 0." << std::endl;
		}
		else
		{
			log()->get(LogLevel::Warning) << "No radius specified. Proceeding with "
				"knn and stride, but ignoring min_k."
				<< std::endl;
		}
	}

	void EigenvaluesFilter::filter(PointView& view)
	{
		const KD3Index& kdi = view.build3dIndex();

		for (PointRef p : view)
		{
			// find neighbors, either by radius or k nearest neighbors
			PointIdList ids;
			if (m_args->m_radiusArg->set())
			{
				ids = kdi.radius(p, m_args->m_radius);

				// if insufficient number of neighbors, eigen solver will fail
				// anyway, it may be okay to silently return without setting any of
				// the computed features?
				if (ids.size() < (size_t)m_args->m_minK)
					continue;
			}
			else
			{
				ids = kdi.neighbors(p, m_args->m_knn + 1, m_args->m_stride);
			}

			// compute covariance of the neighborhood
			Matrix3d B = math::computeCovariance(view, ids);

			// perform the eigen decomposition
			Eigen::SelfAdjointEigenSolver<Matrix3d> solver(B);
			if (solver.info() != Eigen::Success)
				throwError("Cannot perform eigen decomposition.");
			Vector3d ev = solver.eigenvalues();

			if (m_args->m_normalize)
			{
				double sum = ev[0] + ev[1] + ev[2];
				ev /= sum;
			}

			p.setField(Id::Eigenvalue0, ev[0]);
			p.setField(Id::Eigenvalue1, ev[1]);
			p.setField(Id::Eigenvalue2, ev[2]);
		}
	}
}
