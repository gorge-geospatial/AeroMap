#ifndef IQRFILTER_H
#define IQRFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class ProgramArgs;
	class PointView;

	class IQRFilter : public Filter
	{
	public:
		IQRFilter() : Filter()
		{}

		std::string getName() const;

	private:
		double m_multiplier;
		std::string m_dimName;
		Dimension::Id m_dimId;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);

		IQRFilter& operator=(const IQRFilter&); // not implemented
		IQRFilter(const IQRFilter&); // not implemented
	};
}

#endif // #ifndef IQRFILTER_H
