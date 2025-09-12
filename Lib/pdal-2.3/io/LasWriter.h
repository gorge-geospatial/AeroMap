#ifndef LASWRITER_H
#define LASWRITER_H

#include <pdal/pdal_features.h>
#include <pdal/FlexWriter.h>
#include <pdal/Streamable.h>

#include "HeaderVal.h"
#include "LasError.h"
#include "LasHeader.h"
#include "LasUtils.h"
#include "LasSummaryData.h"

#ifdef PDAL_HAVE_LASZIP
#include <laszip/laszip_api.h>
#else
using laszip_POINTER = void*;
#endif

namespace pdal
{
	class LeInserter;
	class LasTester;
	class NitfWriter;
	class GeotiffSupport;
	class LazPerfVlrCompressor;

	struct VlrOptionInfo
	{
		std::string m_name;
		std::string m_value;
		std::string m_userId;
		uint16_t m_recordId;
		std::string m_description;
	};

	class LasWriter : public FlexWriter, public Streamable
	{
		friend class LasTester;
		friend class NitfWriter;
	public:
		std::string getName() const;

		LasWriter();
		~LasWriter();

	protected:
		void prepOutput(std::ostream* out, const SpatialReference& srs);
		void finishOutput();

	private:
		LasHeader m_lasHeader;
		std::unique_ptr<LasSummaryData> m_summaryData;
		laszip_POINTER m_laszip;
		LazPerfVlrCompressor* m_compressor;
		bool m_discardHighReturnNumbers;
		std::map<std::string, std::string> m_headerVals;
		std::vector<VlrOptionInfo> m_optionInfos;
		std::ostream* m_ostream;
		std::vector<LasVLR> m_vlrs;
		std::vector<ExtLasVLR> m_eVlrs;
		StringList m_extraDimSpec;
		std::vector<ExtraDim> m_extraDims;
		uint16_t m_extraByteLen;
		SpatialReference m_srs;
		std::string m_curFilename;
		StringList m_forwardSpec;
		std::set<std::string> m_forwards;
		bool m_forwardVlrs = false;
		LasCompression m_compression;
		std::vector<char> m_pointBuf;
		SpatialReference m_aSrs;
		int m_srsCnt;

		NumHeaderVal<uint8_t, 1, 1> m_majorVersion;
		NumHeaderVal<uint8_t, 1, 4> m_minorVersion;
		NumHeaderVal<uint8_t, 0, 10> m_dataformatId;
		// MSVC doesn't see numeric_limits::max() as constexpr so doesn't allow
		// it as defaults for templates.  Remove when possible.
		NumHeaderVal<uint16_t, 0, 65535> m_filesourceId;
		NumHeaderVal<uint16_t, 0, 31> m_globalEncoding;
		UuidHeaderVal m_projectId;
		StringHeaderVal<32> m_systemId;
		StringHeaderVal<32> m_softwareId;
		NumHeaderVal<uint16_t, 0, 366> m_creationDoy;
		// MSVC doesn't see numeric_limits::max() as constexpr so doesn't allow
		// them as defaults for templates.  Remove when possible.
		NumHeaderVal<uint16_t, 0, 65535> m_creationYear;
		StringHeaderVal<0> m_scaleX;
		StringHeaderVal<0> m_scaleY;
		StringHeaderVal<0> m_scaleZ;
		StringHeaderVal<0> m_offsetX;
		StringHeaderVal<0> m_offsetY;
		StringHeaderVal<0> m_offsetZ;
		MetadataNode m_forwardMetadata;
		bool m_writePDALMetadata;
		std::vector<ExtLasVLR> m_userVLRs;
		bool m_firstPoint;

		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual void prepared(PointTableRef table);
		virtual void readyTable(PointTableRef table);
		virtual void readyFile(const std::string& filename, const SpatialReference& srs);
		virtual bool srsOverridden() const
		{
			return m_aSrs.valid();
		}
		void prerunFile(const PointViewSet& pvSet);
		virtual void writeView(const PointViewPtr view);
		virtual bool processOne(PointRef& point);
		void spatialReferenceChanged(const SpatialReference& srs);
		virtual void doneFile();

		void handleLaszip(int result);
		void fillForwardList();
		void addUserVlrs();
		template <typename T>
		void handleHeaderForward(const std::string& s, T& headerVal, const MetadataNode& base);
		void handleHeaderForwards(MetadataNode& forward);
		void fillHeader();
		bool fillPointBuf(PointRef& point, LeInserter& ostream);
		point_count_t fillWriteBuf(const PointView& view, PointId startId, std::vector<char>& buf);
		bool writeLasZipBuf(PointRef& point);
		void writeLazPerfBuf(char* data, size_t pointLen, point_count_t numPts);
		void addForwardVlrs();
		void addMetadataVlr(MetadataNode& forward);
		void addPipelineVlr();
		void addExtraBytesVlr();
		void addSpatialRefVlrs();
		MetadataNode findVlrMetadata(MetadataNode node, uint16_t recordId, const std::string& userId);
		void readyCompression();
		void readyLasZipCompression();
		void readyLazPerfCompression();
		void openCompression();
		void addVlr(const std::string& userId, uint16_t recordId, const std::string& description, std::vector<uint8_t>& data);
		void addVlr(const ExtLasVLR& evlr);
		void deleteVlr(const std::string& userId, uint16_t recordId);
		void addGeotiffVlrs();
		bool addWktVlr();
		void finishLasZipOutput();
		void finishLazPerfOutput();
		bool processPoint(PointRef& point);

		LasWriter& operator=(const LasWriter&) = delete;
		LasWriter(const LasWriter&) = delete;
		LasWriter(const LasWriter&&) = delete;
	};
}

#endif // #ifndef LASWRITER_H
