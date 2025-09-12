#ifndef GDALWRITER_H
#define GDALWRITER_H

#include <algorithm>

#include <pdal/FlexWriter.h>
#include <pdal/PointView.h>
#include <pdal/Streamable.h>
#include <pdal/util/Bounds.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{

	class GDALGrid;

	class GDALWriter : public FlexWriter, public Streamable
	{
		struct Cell
		{
			long x;
			long y;
		};
		struct Position
		{
			double x;
			double y;
		};

	public:
		std::string getName() const;

		GDALWriter() 
			: m_outputTypes(0)
			, m_expandByPoint(true)
		{
		}

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual void prepared(PointTableRef table);
		virtual void readyFile(const std::string& filename, const SpatialReference& srs);
		virtual void writeView(const PointViewPtr view);
		virtual bool processOne(PointRef& point);
		virtual void doneFile();
		void createGrid(BOX2D bounds);
		void expandGrid(BOX2D bounds);
		int width() const;
		int height() const;

		std::string m_outputFilename;
		std::string m_drivername;
		SpatialReference m_srs;
		Bounds m_bounds;
		double m_edgeLength;
		Arg* m_radiusArg;
		double m_xOrigin;
		double m_yOrigin;
		size_t m_width;
		size_t m_height;
		Arg* m_xOriginArg;
		Arg* m_yOriginArg;
		Arg* m_heightArg;
		Arg* m_widthArg;
		double m_radius;
		double m_power;
		StringList m_options;
		StringList m_outputTypeString;
		size_t m_windowSize;
		int m_outputTypes;
		std::unique_ptr<GDALGrid> m_grid;
		double m_noData;
		Dimension::Id m_interpDim;
		std::string m_interpDimString;
		Dimension::Type m_dataType;
		bool m_expandByPoint;
		bool m_fixedGrid;
		SpatialReference m_defaultSrs;
		SpatialReference m_overrideSrs;
	};
}

#endif // #ifndef GDALWRITER_H
