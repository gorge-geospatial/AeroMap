#ifndef GDALREADER_H
#define GDALREADER_H

#include <string>
#include <vector>

#include <pdal/Dimension.h>
#include <pdal/Reader.h>
#include <pdal/StageFactory.h>
#include <pdal/Streamable.h>

namespace pdal
{
	namespace gdal { class Raster; }

	typedef std::map<std::string, Dimension::Id> DimensionMap;

	class GDALReader : public Reader, public Streamable
	{
	public:
		std::string getName() const;

		GDALReader();
		~GDALReader();

	private:
		virtual void initialize();
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void ready(PointTableRef table);
		virtual point_count_t read(PointViewPtr view, point_count_t num);
		virtual void done(PointTableRef table);
		virtual bool processOne(PointRef& point);
		virtual QuickInfo inspect();
		virtual void addArgs(ProgramArgs& args);

		std::unique_ptr<gdal::Raster> m_raster;
		std::vector<Dimension::Type> m_bandTypes;
		std::vector<Dimension::Id> m_bandIds;
		std::string m_header;
		int m_width;
		int m_height;
		bool m_useMemoryCopy;
		point_count_t m_index;
		int m_row;
		int m_col;

		BOX3D m_bounds;
		StringList m_dimNames;
	};
}

#endif // #ifndef GDALREADER_H
