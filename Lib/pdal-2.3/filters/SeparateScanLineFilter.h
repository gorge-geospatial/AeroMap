#ifndef SEPARATESCANLINEFILTER_H
#define SEPARATESCANLINEFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class PointView;
	class ProgramArgs;

	class SeparateScanLineFilter : public Filter
	{
	public:
		SeparateScanLineFilter();
		SeparateScanLineFilter& operator=(const SeparateScanLineFilter&) = delete;
		SeparateScanLineFilter(const SeparateScanLineFilter&) = delete;

		std::string getName() const;

	private:
		uint64_t m_groupBy;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);
	};
}

#endif // #ifndef SEPARATESCANLINEFILTER_H
