#ifndef FERRYFILTER_H
#define FERRYFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

#include <vector>
#include <string>

namespace pdal
{
	class FerryFilter : public Filter, public Streamable
	{
		struct Info
		{
			std::string m_fromName;
			std::string m_toName;
			Dimension::Id m_fromId;
			Dimension::Id m_toId;

			Info(const std::string& fromName, const std::string& toName) :
				m_fromName(fromName), m_toName(toName),
				m_fromId(Dimension::Id::Unknown),
				m_toId(Dimension::Id::Unknown)
			{}
		};
	public:
		FerryFilter()
		{}

		std::string getName() const;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual bool processOne(PointRef& point);
		virtual void filter(PointView& view);

		FerryFilter& operator=(const FerryFilter&) = delete;
		FerryFilter(const FerryFilter&) = delete;

		StringList m_dimSpec;
		std::vector<Info> m_dims;
	};
}

#endif // #ifndef FERRYFILTER_H
