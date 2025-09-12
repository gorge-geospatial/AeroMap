#ifndef EIGENVALUESFILTER_H
#define EIGENVALUESFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class PointLayout;
	class PointView;
	struct EigenvalueArgs;

	class EigenvaluesFilter : public Filter
	{
	public:
		EigenvaluesFilter();
		EigenvaluesFilter& operator=(const EigenvaluesFilter&) = delete;
		EigenvaluesFilter(const EigenvaluesFilter&) = delete;

		std::string getName() const;

	private:
		std::unique_ptr<EigenvalueArgs> m_args;

		virtual void addDimensions(PointLayoutPtr layout);
		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef EIGENVALUESFILTER_H
