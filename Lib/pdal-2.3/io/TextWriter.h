#ifndef TEXTWRITER_H
#define TEXTWRITER_H

#include <pdal/Streamable.h>
#include <pdal/Writer.h>

namespace pdal
{
	typedef std::shared_ptr<std::ostream> FileStreamPtr;

	class TextWriter : public Writer, public Streamable
	{
		struct DimSpec
		{
			Dimension::Id id;
			size_t precision;
			std::string name;
		};

		enum class OutputType
		{
			CSV,
			GEOJSON
		};

		friend std::istream& operator >> (std::istream& in, OutputType& type);
		friend std::ostream& operator << (std::ostream& out,
			const OutputType& type);

	public:
		TextWriter()
		{}

		std::string getName() const;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void initialize(PointTableRef table);
		virtual void ready(PointTableRef table);
		virtual void write(const PointViewPtr view);
		virtual void done(PointTableRef table);
		virtual bool processOne(PointRef& point);

		void writeHeader(PointTableRef table);
		void writeFooter();
		void writeGeoJSONHeader();
		void writeCSVHeader(PointTableRef table);
		void processOneCSV(PointRef& point);
		void processOneGeoJSON(PointRef& point);

		DimSpec extractDim(std::string dim, PointTableRef table);
		bool findDim(Dimension::Id id, DimSpec& ds);

		std::string m_filename;
		OutputType m_outputType;
		std::string m_callback;
		bool m_writeAllDims;
		std::string m_dimOrder;
		bool m_writeHeader;
		std::string m_newline;
		std::string m_delimiter;
		bool m_quoteHeader;
		bool m_packRgb;
		int m_precision;
		PointId m_idx;

		FileStreamPtr m_stream;
		std::vector<DimSpec> m_dims;
		DimSpec m_xDim;
		DimSpec m_yDim;
		DimSpec m_zDim;

		TextWriter& operator=(const TextWriter&); // not implemented
		TextWriter(const TextWriter&); // not implemented
	};
}

#endif // #ifndef TEXTWRITER_H
