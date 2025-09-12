#ifndef DELAUNAYFILTER_H
#define DELAUNAYFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class DelaunayFilter : public Filter
	{
	public:
		DelaunayFilter& operator=(const DelaunayFilter&) = delete;
		DelaunayFilter(const DelaunayFilter&) = delete;
		DelaunayFilter();
		virtual ~DelaunayFilter();

		std::string getName() const;

	private:
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef DELAUNAYFILTER_H
