#ifndef LOFFILTER_H
#define LOFFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class PointLayout;
	class PointView;
	class ProgramArgs;

	class LOFFilter : public Filter
	{
	public:
		LOFFilter() : Filter()
		{}
		LOFFilter& operator=(const LOFFilter&) = delete;
		LOFFilter(const LOFFilter&) = delete;

		std::string getName() const;

	private:
		size_t m_minpts;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef LOFFILTER_H
