#ifndef CSFILTER_H
#define CSFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	struct CSArgs;

	class CSFilter : public Filter
	{
	public:
		CSFilter();

		CSFilter& operator=(const CSFilter&) = delete;
		CSFilter(const CSFilter&) = delete;

		std::string getName() const override;

	private:
		std::unique_ptr<CSArgs> m_args;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);

		void classifyGround(PointViewPtr, std::vector<double>&);
	};
}

#endif // #ifndef CSFILTER_H
