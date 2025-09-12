#ifndef LITREEFILTER_H
#define LITREEFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class LiTreeFilter : public Filter
	{
	public:
		LiTreeFilter() : Filter() {}
		LiTreeFilter& operator=(const LiTreeFilter&) = delete;
		LiTreeFilter(const LiTreeFilter&) = delete;

		std::string getName() const;

	private:
		std::vector<int> m_localMax;
		point_count_t m_minSize;
		double m_minHag;
		double m_dummyRadius;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual void filter(PointView& view);

		PointId locateHighestPoint(PointView& view, PointIdList const& Ui);
		PointId locateDummyPoint(PointView& view, PointIdList const& Ui,
			PointId t0);
		void segmentTree(PointView& view, PointIdList& Ui, int64_t& tree_id);
		void classifyPoint(PointId ui, PointView& view, PointIdList& Ni, PointIdList& Pi);
		void computeLocalMax(PointView& view);
	};
}

#endif // #ifndef LITREEFILTER_H
