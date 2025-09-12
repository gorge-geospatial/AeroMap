#ifndef TAILFILTER_H
#define TAILFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class TailFilter : public Filter
	{
	public:
		TailFilter()
		{}
		TailFilter& operator=(const TailFilter&) = delete;
		TailFilter(const TailFilter&) = delete;

		std::string getName() const;

	private:
		point_count_t m_count;
		bool m_invert;

		void addArgs(ProgramArgs& args)
		{
			args.add("count", "Number of points to return from end. "
				"If 'invert' is true, number of points to drop from the end.",
				m_count, point_count_t(10));
			args.add("invert", "If true, 'count' specifies the number of points "
				"at the end to drop.", m_invert);
		}

		PointViewSet run(PointViewPtr view)
		{
			if (m_count > view->size())
				log()->get(LogLevel::Warning)
				<< "Requested number of points (count=" << m_count
				<< ") exceeds number of available points.\n";
			PointViewSet viewSet;
			PointViewPtr outView = view->makeNew();
			PointId start;
			PointId end;
			if (m_invert)
			{
				start = 0;
				end = view->size() - (std::min)(m_count, view->size());
			}
			else
			{
				start = view->size() - (std::min)(m_count, view->size());
				end = view->size();
			}

			for (PointId i = start; i < end; ++i)
				outView->appendPoint(*view, i);
			viewSet.insert(outView);
			return viewSet;
		}
	};
}

#endif // #ifndef TAILFILTER_H
