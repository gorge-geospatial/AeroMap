#ifndef TINDEXREADER_H
#define TINDEXREADER_H

#include <pdal/PointView.h>
#include <pdal/Reader.h>
#include <pdal/StageFactory.h>
#include <filters/MergeFilter.h>

namespace pdal
{
	namespace gdal { class SpatialRef; }

	class TIndexReader : public Reader
	{
		struct FileInfo
		{
			std::string m_filename;
			std::string m_srs;
			std::string m_boundary;
			struct tm m_ctime;
			struct tm m_mtime;
		};

		struct FieldIndexes
		{
			int m_filename;
			int m_srs;
			int m_ctime;
			int m_mtime;
		};

	public:
		TIndexReader() : m_dataset(NULL), m_layer(NULL)
		{}

		std::string getName() const;

	private:
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual void prepared(PointTableRef table);
		virtual void ready(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);

		std::string m_layerName;
		std::string m_driverName;
		std::string m_tileIndexColumnName;
		std::string m_srsColumnName;
		std::string m_wkt;
		std::string m_tgtSrsString;
		std::string m_filterSRS;
		std::string m_attributeFilter;
		std::string m_dialect;
		BOX2D m_bounds;
		std::string m_sql;

		std::unique_ptr<gdal::SpatialRef> m_out_ref;
		void* m_dataset;
		void* m_layer;

		StageFactory m_factory;
		MergeFilter m_merge;
		PointViewSet m_pvSet;

		std::vector<FileInfo> getFiles();
		FieldIndexes getFields();
	};

}

#endif // #ifndef TINDEXREADER_H
