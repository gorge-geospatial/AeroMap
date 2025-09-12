/******************************************************************************
* Copyright (c) 2016, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

#include "ApproximateCoplanarFilter.h"

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
		"filters.approximatecoplanar",
		"Estimates the planarity of a neighborhood of points using eigenvalues.",
		"http://pdal.io/stages/filters.approximatecoplanar.html"
	};

	CREATE_STATIC_STAGE(ApproximateCoplanarFilter, s_info)
	// expands to: 
	//	static bool ApproximateCoplanarFilter_b = pdal::PluginManager<pdal::Stage>::registerPlugin<ApproximateCoplanarFilter(s_Info)

	std::string ApproximateCoplanarFilter::getName() const
	{
		return s_info.name;
	}

	void ApproximateCoplanarFilter::addArgs(ProgramArgs& args)
	{
		args.add("knn", "k-Nearest Neighbors", m_knn, 8);
		args.add("thresh1", "Threshold 1", m_thresh1, 25.0);
		args.add("thresh2", "Threshold 2", m_thresh2, 6.0);
	}

	void ApproximateCoplanarFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Id::Coplanar);
	}

	void ApproximateCoplanarFilter::filter(PointView& view)
	{
		const KD3Index& kdi = view.build3dIndex();

		for (PointRef p : view)
		{
			// find the k-nearest neighbors
			PointIdList ids = kdi.neighbors(p, m_knn);

			// compute covariance of the neighborhood
			Matrix3d B = math::computeCovariance(view, ids);

			// perform the eigen decomposition
			Eigen::SelfAdjointEigenSolver<Matrix3d> solver(B);
			if (solver.info() != Eigen::Success)
				throwError("Cannot perform eigen decomposition.");
			Vector3d ev = solver.eigenvalues();

			// test eigenvalues to label points that are approximately coplanar
			if ((ev[1] > m_thresh1 * ev[0]) && (m_thresh2 * ev[1] > ev[2]))
				p.setField(Id::Coplanar, 1u);
			else
				p.setField(Id::Coplanar, 0u);
		}
	}
}
