#ifndef RETURNSFILTER_H
#define RETURNSFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class PointView;
	class ProgramArgs;

	class ReturnsFilter : public Filter
	{
	public:
		ReturnsFilter() {}

		static const int returnFirst = 1;
		static const int returnIntermediate = 2;
		static const int returnLast = 4;
		static const int returnOnly = 8;

		std::string getName() const;

	private:
		StringList m_returnsString;
		int m_outputTypes;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);

		ReturnsFilter& operator=(const ReturnsFilter&) = delete; // not implemented
		ReturnsFilter(const ReturnsFilter&) = delete;            // not implemented
	};
}

#endif // #ifndef RETURNSFILTER_H
