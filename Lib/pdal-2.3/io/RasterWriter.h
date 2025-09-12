#ifndef RASTERWRITER_H
#define RASTERWRITER_H

#include <vector>

#include <pdal/Writer.h>
#include <pdal/PointView.h>
#include <pdal/private/Raster.h>

namespace pdal
{
	class RasterWriter : public Writer
	{
	public:
		std::string getName() const;

		RasterWriter();
		~RasterWriter();

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void write(const PointViewPtr view);
		virtual void done(PointTableRef table);

		std::string m_filename;
		std::string m_drivername;
		StringList m_options;
		StringList m_rasterNames;
		Dimension::Type m_dataType;
		double m_noData;
		std::vector<Rasterd*> m_rasters;
	};
}

#endif // #ifndef RASTERWRITER_H
