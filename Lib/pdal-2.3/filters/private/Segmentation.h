#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <pdal/pdal_export.h>
#include <pdal/pdal_types.h>

#include "DimRange.h"

#include <vector>

namespace pdal
{
	class PointView;

	namespace Segmentation
	{
		class PointClasses
		{
		public:
			PointClasses() : m_classes(0)
			{}

			bool isWithheld() const
			{
				return m_classes & ClassLabel::Withheld;
			}
			bool isKeypoint() const
			{
				return m_classes & ClassLabel::Keypoint;
			}
			bool isSynthetic() const
			{
				return m_classes & ClassLabel::Synthetic;
			}
			bool isNone() const
			{
				return m_classes == 0;
			}
			uint32_t bits() const
			{
				return m_classes;
			}

		private:
			uint32_t m_classes;

			friend std::istream& operator>>(std::istream& in, PointClasses& classes);
			friend std::ostream& operator<<(std::ostream& out,
				const PointClasses& classes);
		};

		std::istream& operator>>(std::istream& in, PointClasses& classes);
		std::ostream& operator<<(std::ostream& out, const PointClasses& classes);

		/**
		  Extract clusters of points from input PointView.

		  For each point, find neighbors within a given tolerance (Euclidean distance).
		  If a neighbor already belongs to another cluster, skip it. Otherwise, add it
		  to the current cluster. Recursively visit newly added cluster points, looking
		  for neighbors to add to the cluster.

		  \param[in] view the input PointView.
		  \param[in] min_points the minimum number of points in a cluster.
		  \param[in] max_points the maximum number of points in a cluster.
		  \param[in] tolerance the tolerance for adding points to a cluster.
		  \returns a deque of clusters (themselves vectors of PointIds).
		*/
		template <class KDINDEX>
		std::deque<PointIdList> extractClusters(PointView& view, uint64_t min_points,
			uint64_t max_points, double tolerance)
		{
			// Index the incoming PointView for subsequent radius searches.
			KDINDEX kdi(view);
			kdi.build();

			// Create variables to track PointIds that have already been added to
			// clusters and to build the list of cluster indices.
			PointIdList processed(view.size(), 0);
			std::deque<PointIdList> clusters;

			for (PointId i = 0; i < view.size(); ++i)
			{
				// Points can only belong to a single cluster.
				if (processed[i])
					continue;

				// Initialize list of indices belonging to current cluster, marking the
				// seed point as processed.
				PointIdList seed_queue;
				size_t sq_idx = 0;
				seed_queue.push_back(i);
				processed[i] = 1;

				// Check each point in the cluster for additional neighbors within the
				// given tolerance, remembering that the list can grow if we add points
				// to the cluster.
				while (sq_idx < seed_queue.size())
				{
					// Find neighbors of the next cluster point.
					PointId j = seed_queue[sq_idx];
					PointIdList ids = kdi.radius(j, tolerance);

					// The case where the only neighbor is the query point.
					if (ids.size() == 1)
					{
						sq_idx++;
						continue;
					}

					// Skip neighbors that already belong to a cluster and add the rest
					// to this cluster.
					for (auto const& k : ids)
					{
						if (processed[k])
							continue;
						seed_queue.push_back(k);
						processed[k] = 1;
					}

					sq_idx++;
				}

				// Keep clusters that are within the min/max number of points.
				if (seed_queue.size() >= min_points && seed_queue.size() <= max_points)
					clusters.push_back(seed_queue);
			}

			return clusters;
		}

		void ignoreDimRange(DimRange dr, PointViewPtr input, PointViewPtr keep, PointViewPtr ignore);
		void ignoreDimRanges(std::vector<DimRange>& ranges, PointViewPtr input, PointViewPtr keep, PointViewPtr ignore);

		void ignoreClassBits(PointViewPtr input, PointViewPtr keep, PointViewPtr ignore, PointClasses classbits);

		void segmentLastReturns(PointViewPtr input, PointViewPtr last, PointViewPtr other);

		void segmentReturns(PointViewPtr input, PointViewPtr first, PointViewPtr second, StringList returns);

		PointIdList farthestPointSampling(PointView& view, point_count_t count);
	}
}

#endif // #ifndef SEGMENTATION_H
