#ifndef ZMOOTHFILTER_H
#define ZMOOTHFILTER_H

#include <memory>

#include <pdal/Filter.h>

namespace pdal
{
	class ZsmoothFilter : public Filter
	{
		struct Private;

	public:
		std::string getName() const;

		ZsmoothFilter();
		~ZsmoothFilter();

	private:
		void addArgs(ProgramArgs& args);
		void addDimensions(PointLayoutPtr layout);
		void prepared(PointTableRef table);
		void filter(PointView& view);

		std::unique_ptr<Private> m_p;
	};
}

#endif // #ifndef ZMOOTHFILTER_H
