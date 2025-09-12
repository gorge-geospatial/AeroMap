#ifndef OGRWRITER_H
#define OGRWRITER_H

#include <pdal/PointView.h>
#include <pdal/FlexWriter.h>
#include <pdal/Streamable.h>
#include <pdal/util/ProgramArgs.h>

#include <gdal_priv.h>
#include <ogr_feature.h>

namespace pdal
{
	class OGRWriter : public FlexWriter, public Streamable
	{
	public:
		std::string getName() const;

		OGRWriter();

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual void prepared(PointTableRef table);
		virtual void readyTable(PointTableRef table);
		virtual void readyFile(const std::string& filename,
			const SpatialReference& srs);
		virtual void writeView(const PointViewPtr view);
		virtual bool processOne(PointRef& point);
		virtual void doneFile();

		// I don't think this needs to be deleted.
		GDALDriver* m_driver;
		GDALDataset* m_ds;
		OGRLayer* m_layer;
		OGRFeature* m_feature;
		OGRwkbGeometryType m_geomType;
		OGRMultiPoint m_multiPoint;
		std::string m_outputFilename;
		std::string m_driverName;
		size_t m_multiCount;
		size_t m_curCount;
		std::string m_measureDimName;
		Dimension::Id m_measureDim;
	};
}

#endif // #ifndef OGRWRITER_H
