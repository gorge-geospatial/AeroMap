#ifndef ELMFILTER_H
#define ELMFILTER_H

#include <pdal/Filter.h>

#include <cstdint>
#include <string>

namespace pdal
{
	class PointLayout;
	class PointView;

	class ELMFilter : public Filter
	{
	public:
		ELMFilter() : Filter()
		{
		}

		std::string getName() const;

	private:
		double m_cell;
		double m_threshold;
		uint8_t m_class;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void filter(PointView& view);

		ELMFilter& operator=(const ELMFilter&); // not implemented
		ELMFilter(const ELMFilter&);            // not implemented
	};
}

#endif // #ifndef ELMFILTER_H
