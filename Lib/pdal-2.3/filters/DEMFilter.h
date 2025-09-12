#ifndef DEMFILTER_H
#define DEMFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

#include <cstdint>
#include <memory>
#include <string>

namespace pdal
{
	struct DEMArgs;

	namespace gdal { class Raster; }
	class Options;
	class PointLayout;
	class PointView;

	class DEMFilter : public Filter, public Streamable
	{
	public:
		DEMFilter();
		~DEMFilter();

		std::string getName() const;

	private:

		std::unique_ptr<DEMArgs> m_args;
		std::unique_ptr<gdal::Raster> m_raster;

		virtual void ready(PointTableRef table);
		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);
		virtual bool processOne(PointRef& point);

		DEMFilter& operator=(const DEMFilter&); // not implemented
		DEMFilter(const DEMFilter&); // not implemented
	};
}

#endif // #ifndef DEMFILTER_H
