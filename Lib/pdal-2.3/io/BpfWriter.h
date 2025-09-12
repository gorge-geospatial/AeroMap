#ifndef BPFWRITER_H
#define BPFWRITER_H

#include "BpfHeader.h"

#include <pdal/pdal_export.h>
#include <pdal/FlexWriter.h>
#include <pdal/util/OStream.h>

#include <vector>

namespace pdal
{
	class BpfWriter : public FlexWriter
	{
	public:
		struct CoordId
		{
			CoordId() : m_auto(false), m_val(0)
			{}

			bool m_auto;
			int m_val;
		};

		std::string getName() const;

	private:
		StringList m_outputDims; ///< List of dimensions to write
		OLeStream m_stream;
		BpfHeader m_header;
		BpfDimensionList m_dims;
		std::vector<uint8_t> m_extraData;
		std::vector<BpfUlemFile> m_bundledFiles;
		bool m_compression;
		CoordId m_coordId;
		std::string m_extraDataSpec;
		StringList m_bundledFilesSpec;
		std::string m_curFilename;

		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual void prepared(PointTableRef table);
		virtual void readyFile(const std::string& filename, const SpatialReference& srs);
		void prerunFile(const PointViewSet& pvSet);
		virtual void writeView(const PointViewPtr data);
		virtual void doneFile();

		double getAdjustedValue(const PointView* data, BpfDimension& bpfDim, PointId idx);
		void loadBpfDimensions(PointLayoutPtr layout);
		void writePointMajor(const PointView* data);
		void writeDimMajor(const PointView* data);
		void writeByteMajor(const PointView* data);
		void writeCompressedBlock(char* buf, size_t size);
	};
}

#endif // #ifndef BPFWRITER_H
