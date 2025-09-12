#ifndef RECIPROCITYFILTER_H
#define RECIPROCITYFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class PointLayout;
	class PointView;

	class ReciprocityFilter : public Filter
	{
	public:
		ReciprocityFilter() : Filter()
		{}
		ReciprocityFilter& operator=(const ReciprocityFilter&) = delete;
		ReciprocityFilter(const ReciprocityFilter&) = delete;

		std::string getName() const;

	private:
		int m_knn;
		int m_threads;

		virtual void addDimensions(PointLayoutPtr layout);
		virtual void addArgs(ProgramArgs& args);
		virtual void filter(PointView& view);

		void setReciprocity(PointView& view, const PointId& i);
	};
}

#endif // #ifndef RECIPROCITYFILTER_H
