#ifndef LASREADER_H
#define LASREADER_H

#include <memory>

#include <pdal/pdal_export.h>
#include <pdal/pdal_features.h>
#include <pdal/PDALUtils.h>
#include <pdal/Reader.h>
#include <pdal/Streamable.h>

//#include "LasError.hpp"

namespace pdal
{
	class NitfReader;
	class LasHeader;
	class LeExtractor;
	class PointDimensions;
	class LazPerfVlrDecompressor;

	class LasReader : public Reader, public Streamable
	{
	protected:
		class LasStreamIf
		{
		protected:
			LasStreamIf()
			{}

		public:
			LasStreamIf(const std::string& filename)
			{
				m_istream = Utils::openFile(filename);
			}

			virtual ~LasStreamIf()
			{
				if (m_istream)
					Utils::closeFile(m_istream);
			}

			std::istream* m_istream;
		};

		friend class NitfReader;
	public:
		LasReader();
		~LasReader();
		LasReader& operator=(const LasReader&) = delete;
		LasReader(const LasReader&) = delete;

		std::string getName() const;

		const LasHeader& header() const;
		point_count_t getNumPoints() const;

	protected:
		virtual void createStream();

		std::unique_ptr<LasStreamIf> m_streamIf;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void initialize(PointTableRef table);
		virtual void initializeLocal(PointTableRef table, MetadataNode& m);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual QuickInfo inspect();
		virtual void ready(PointTableRef table);
		virtual point_count_t read(PointViewPtr view, point_count_t count);
		virtual bool processOne(PointRef& point);
		virtual void done(PointTableRef table);
		virtual bool eof();

		void handleCompressionOption();
		void setSrs(MetadataNode& m);
		void readExtraBytesVlr();
		void extractHeaderMetadata(MetadataNode& forward, MetadataNode& m);
		void extractVlrMetadata(MetadataNode& forward, MetadataNode& m);
		void loadPoint(PointRef& point);
		void loadPointV10(PointRef& point);
		void loadPointV14(PointRef& point);
		void loadPoint(PointRef& point, char* buf, size_t bufsize);
		void loadPointV10(PointRef& point, char* buf, size_t bufsize);
		void loadPointV14(PointRef& point, char* buf, size_t bufsize);
		void loadExtraDims(LeExtractor& istream, PointRef& data);
		point_count_t readFileBlock(std::vector<char>& buf,	point_count_t maxPoints);
		void handleLaszip(int result);

		struct Args;
		std::unique_ptr<Args> m_args;

		struct Private;
		std::unique_ptr<Private> m_p;
	};
}

#endif // #ifndef LASREADER_H
