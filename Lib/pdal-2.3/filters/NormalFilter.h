#ifndef NORMALFILTER_H
#define NORMALFILTER_H

#include <pdal/Filter.h>
#include <pdal/util/ProgramArgs.h>

#include <memory>
#include <string>

namespace pdal
{
	class Options;
	class PointLayout;
	class PointView;
	struct NormalArgs;

	struct Edge
	{
		PointId m_v0;
		PointId m_v1;
		double m_weight;

		Edge(PointId i, PointId j, double weight)
			: m_v0(i), m_v1(j), m_weight(weight)
		{
		}
	};

	struct CompareEdgeWeight
	{
		bool operator()(Edge const& lhs, Edge const& rhs)
		{
			return lhs.m_weight > rhs.m_weight;
		}
	};

	typedef std::vector<Edge> EdgeList;

	class NormalFilter : public Filter
	{
	public:
		NormalFilter();
		~NormalFilter();

		NormalFilter& operator=(const NormalFilter&) = delete;
		NormalFilter(const NormalFilter&) = delete;

		void doFilter(PointView& view, int knn = 8);

		std::string getName() const;

	private:
		std::unique_ptr<NormalArgs> m_args;
		point_count_t m_count;
		Arg* m_viewpointArg;

		void compute(PointView& view, KD3Index& kdi);
		void refine(PointView& view, KD3Index& kdi);
		void
			update(PointView& view, KD3Index& kdi, std::vector<bool> inMST,
				std::priority_queue<Edge, EdgeList, CompareEdgeWeight> edge_queue,
				PointId updateIdx);

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef NORMALFILTER_H
