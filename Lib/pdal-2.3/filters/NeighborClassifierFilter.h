#ifndef NEIGHBORCLASSIFIERFILTER_H
#define NEIGHBORCLASSIFIERFILTER_H

#include <pdal/Filter.h>
#include <pdal/KDIndex.h>
#include <unordered_map>

extern "C" int32_t NeighborClassifierFilter_ExitFunc();
extern "C" PF_ExitFunc NeighborClassifierFilter_InitPlugin();

namespace pdal
{
	struct DimRange;

	class NeighborClassifierFilter : public Filter
	{
	public:
		NeighborClassifierFilter();
		~NeighborClassifierFilter();

		static void* create();
		static int32_t destroy(void*);
		std::string getName() const { return "filters.neighborclassifier"; }

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		bool doOne(PointRef& point, PointRef& temp, KD3Index& kdi);
		virtual void filter(PointView& view);
		virtual void initialize();
		virtual void ready(PointTableRef);
		void doOneNoDomain(PointRef& point, PointRef& temp, KD3Index& kdi);
		PointViewPtr loadSet(const std::string& candFileName, PointTableRef table);
		NeighborClassifierFilter& operator=(
			const NeighborClassifierFilter&) = delete;
		NeighborClassifierFilter(const NeighborClassifierFilter&) = delete;
		StringList m_domainSpec;
		std::vector<DimRange> m_domain;
		int m_k;
		Dimension::Id m_dim;
		std::string m_dimName;
		std::string m_candidateFile;
		std::unordered_map<PointId, int> m_newClass;
	};
}

#endif // #ifndef NEIGHBORCLASSIFIERFILTER_H
