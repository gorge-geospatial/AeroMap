#ifndef MINIBALLFILTER_H
#define MINIBALLFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class MiniballFilter : public Filter
	{
	public:
		MiniballFilter()
		{}
		MiniballFilter& operator=(const MiniballFilter&) = delete;
		MiniballFilter(const MiniballFilter&) = delete;

		std::string getName() const;

	private:
		int m_knn;
		int m_threads;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void filter(PointView& view);

		void setMiniball(PointView& view, const PointId& i);
	};
}

#endif // #ifndef MINIBALLFILTER_H
