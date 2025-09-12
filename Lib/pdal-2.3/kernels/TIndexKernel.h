#ifndef TINDEXKERNEL_H
#define TINDEXKERNEL_H

#include <pdal/Stage.h>
#include <pdal/SubcommandKernel.h>
#include <pdal/util/FileUtils.h>

namespace pdal
{
	class Polygon;

	namespace gdal
	{
		class SpatialRef;
	}

	class StageFactory;

	class TIndexKernel : public SubcommandKernel
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
		std::string getName() const override;
		TIndexKernel();

	private:
		virtual void addSubSwitches(ProgramArgs& args,
			const std::string& subcommand);
		virtual void validateSwitches(ProgramArgs& args);
		virtual int execute();
		virtual StringList subcommands() const;

		void createFile();
		void mergeFile();
		bool openDataset(const std::string& filename);
		bool createDataset(const std::string& filename);
		bool openLayer(const std::string& layerName);
		bool createLayer(const std::string& layerName);
		FieldIndexes getFields();
		bool getFileInfo(StageFactory& factory, const std::string& filename, FileInfo& info);
		bool createFeature(const FieldIndexes& indexes, FileInfo& info);
		pdal::Polygon prepareGeometry(const FileInfo& fileInfo);
		void createFields();
		bool fastBoundary(Stage& reader, FileInfo& fileInfo);
		bool slowBoundary(Stage& hexer, FileInfo& fileInfo);

		bool isFileIndexed(const FieldIndexes& indexes, const FileInfo& fileInfo);

		std::string m_idxFilename;
		std::string m_filespec;
		StringList m_files;
		std::string m_layerName;
		std::string m_driverName;
		std::string m_tileIndexColumnName;
		std::string m_srsColumnName;
		std::string m_wkt;
		BOX2D m_bounds;
		bool m_absPath;

		void* m_dataset;
		void* m_layer;
		std::string m_tgtSrsString;
		std::string m_assignSrsString;
		bool m_fastBoundary;
		bool m_usestdin;
		bool m_overrideASrs;
	};
}

#endif // #ifndef TINDEXKERNEL_H
