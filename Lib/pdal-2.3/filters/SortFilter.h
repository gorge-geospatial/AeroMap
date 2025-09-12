#ifndef SORTFILTER_H
#define SORTFILTER_H

#include <pdal/Filter.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	enum class SortOrder
	{
		ASC, // ascending
		DESC // descending
	};

	std::istream& operator >> (std::istream& in, SortOrder& order);
	std::ostream& operator << (std::ostream& in, const SortOrder& order);

	class SortFilter : public Filter
	{
	public:
		SortFilter()
		{}

		std::string getName() const;

	private:
		// Dimension on which to sort.
		Dimension::Id m_dim;
		// Dimension name.
		std::string m_dimName;

		// Sort order.
		SortOrder m_order;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual void filter(PointView& view);

		SortFilter& operator=(const SortFilter&) = delete;
		SortFilter(const SortFilter&) = delete;
	};
}

#endif // #ifndef SORTFILTER_H
