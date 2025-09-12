/******************************************************************************
 * Copyright (c) 2019, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

 // PDAL implementation of the plane fit criterion presented in T. Weyrich, M.
 // Pauly, R. Keiser, S. Heinzle, S. Scandella, and M. Gross, “Post-processing
 // of Scanned 3D Surface Data,” Proc. Eurographics Symp.  Point-Based Graph.
 // 2004, pp. 85–94, 2004.

#include "PlaneFitFilter.h"

#include <pdal/KDIndex.h>
#include <pdal/util/ProgramArgs.h>
#include <pdal/private/MathUtils.h>

#include <Eigen/Dense>

#include <string>
#include <thread>
#include <vector>

namespace pdal
{
	using namespace Dimension;
	using namespace Eigen;

	static StaticPluginInfo const s_info
	{
		"filters.planefit",
		"Plane Fit (Kutz et al., 2003)",
		"http://pdal.io/stages/filters.planefit.html"
	};

	CREATE_STATIC_STAGE(PlaneFitFilter, s_info)

	std::string PlaneFitFilter::getName() const
	{
		return s_info.name;
	}

	void PlaneFitFilter::addArgs(ProgramArgs& args)
	{
		args.add("knn", "k-Nearest neighbors", m_knn, 8);
		args.add("threads", "Number of threads used to run this filter", m_threads,
			1);
	}

	void PlaneFitFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Id::PlaneFit);
	}

	void PlaneFitFilter::filter(PointView& view)
	{
		point_count_t nloops = view.size();
		std::vector<std::thread> threadList(m_threads);
		for (int t = 0; t < m_threads; t++)
		{
			threadList[t] = std::thread(std::bind(
				[&](const PointId start, const PointId end) {
				for (PointId i = start; i < end; i++)
					setPlaneFit(view, i);
			},
				t * nloops / m_threads,
				(t + 1) == m_threads ? nloops : (t + 1) * nloops / m_threads));
		}
		for (auto& t : threadList)
			t.join();
	}

	double PlaneFitFilter::absDistance(PointView& view, const PointId& i,
		Vector3d& centroid,
		Vector3d& normal)
	{
		double x = view.getFieldAs<double>(Id::X, i);
		double y = view.getFieldAs<double>(Id::Y, i);
		double z = view.getFieldAs<double>(Id::Z, i);
		Vector3d p;
		p << x - centroid[0], y - centroid[1], z - centroid[2];
		double d = normal.dot(p);
		return std::fabs(d);
	}

	void PlaneFitFilter::setPlaneFit(PointView& view, const PointId& i)
	{
		// Find k-nearest neighbors of i.
		const KD3Index& kdi = view.build3dIndex();
		PointIdList ni = kdi.neighbors(i, m_knn + 1);

		// Normal based only on neighbors, so exclude first point.
		PointIdList neighbors(ni.begin() + 1, ni.end());

		// Covariance and normal are based off demeaned coordinates, so we record
		// the centroid to properly offset the coordinates when computing point to
		// plance distance.
		Vector3d centroid = math::computeCentroid(view, neighbors);

		// Compute covariance of the neighbors.
		Matrix3d B = math::computeCovariance(view, neighbors);

		// Perform the eigen decomposition, using the eigenvector of the smallest
		// eigenvalue as the normal.
		Eigen::SelfAdjointEigenSolver<Matrix3d> solver(B);
		if (solver.info() != Eigen::Success)
			throwError("Cannot perform eigen decomposition.");
		Vector3d normal = solver.eigenvectors().col(0);

		// Compute point to plane distance of the query point.
		double d = absDistance(view, i, centroid, normal);

		// Compute mean point to plane distance of neighbors.
		double d_sum(0.0);
		for (PointId const& j : neighbors)
		{
			d_sum += absDistance(view, j, centroid, normal);
		}
		double d_bar(d_sum / m_knn);

		// Compute and set the plane fit criterion.
		view.setField(Id::PlaneFit, i, d / (d + d_bar));
	}
}
