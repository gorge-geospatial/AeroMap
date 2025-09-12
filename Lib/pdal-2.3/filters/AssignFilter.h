#ifndef ASSIGNFILTER_H
#define ASSIGNFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

namespace pdal
{
	struct AssignArgs;

	class AssignFilter : public Filter, 
						 public Streamable
	{
	public:
		AssignFilter();
		~AssignFilter();

		std::string getName() const { return "filters.assign"; }

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual bool processOne(PointRef& point);
		virtual void filter(PointView& view);

		AssignFilter& operator=(const AssignFilter&) = delete;
		AssignFilter(const AssignFilter&) = delete;

		std::unique_ptr<AssignArgs> m_args;
	};
}

#endif // ASSIGNFILTER_H
