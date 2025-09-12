#ifndef PLYWRITER_H
#define PLYWRITER_H

#include <pdal/PointView.h>
#include <pdal/Writer.h>

namespace pdal
{
	class Triangle;

	class PlyWriter : public Writer
	{
	public:
		enum class Format
		{
			Ascii,
			BinaryLe,
			BinaryBe
		};

		std::string getName() const;

		PlyWriter();

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual void ready(PointTableRef table);
		virtual void write(const PointViewPtr data);
		virtual void done(PointTableRef table);

		std::string getType(Dimension::Type type) const;
		void writeHeader(PointLayoutPtr layout) const;
		void writeValue(PointRef& point, Dimension::Id dim, Dimension::Type type);
		void writePoint(PointRef& point, PointLayoutPtr layout);
		void writeTriangle(const Triangle& t, size_t offset);

		std::ostream* m_stream;
		std::string m_filename;
		Format m_format;
		bool m_faces;
		StringList m_dimNames;
		DimTypeList m_dims;
		int m_precision;
		bool m_sizedTypes;
		Arg* m_precisionArg;
		std::vector<PointViewPtr> m_views;
	};

	inline std::istream& operator>>(std::istream& in, PlyWriter::Format& f)
	{
		std::string s;
		std::getline(in, s);
		Utils::trim(s);
		Utils::tolower(s);
		if (s == "ascii")
			f = PlyWriter::Format::Ascii;
		else if (s == "little endian" || s == "binary_little_endian")
			f = PlyWriter::Format::BinaryLe;
		else if (s == "big endian" || s == "binary_big_endian")
			f = PlyWriter::Format::BinaryBe;
		else
			in.setstate(std::ios_base::failbit);
		return in;
	}

	inline std::ostream& operator<<(std::ostream& out, const PlyWriter::Format& f)
	{
		switch (f)
		{
		case PlyWriter::Format::Ascii:
			out << "ascii";
			break;
		case PlyWriter::Format::BinaryLe:
			out << "binary_little_endian";
			break;
		case PlyWriter::Format::BinaryBe:
			out << "binary_big_endian";
			break;
		}
		return out;
	}
}

#endif // #ifndef PLYWRITER_H
