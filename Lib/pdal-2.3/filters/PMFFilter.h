#ifndef PMFFILTER_H
#define PMFFILTER_H

#include <pdal/Filter.h>

#include <memory>

namespace pdal
{
	struct PMFArgs;

	class PMFFilter : public Filter
	{
	public:
		PMFFilter();
		~PMFFilter();

		std::string getName() const override;

	private:
		std::unique_ptr<PMFArgs> m_args;

		virtual void addDimensions(PointLayoutPtr layout);
		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);

		void processGround(PointViewPtr view);

		PMFFilter& operator=(const PMFFilter&); // not implemented
		PMFFilter(const PMFFilter&);            // not implemented
	};
}

#endif // #ifndef PMFFILTER_H
