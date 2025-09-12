#ifndef LASUTILS_H
#define LASUTILS_H

#include <pdal/Dimension.h>
#include <pdal/DimType.h>
#include <string>

namespace pdal
{
	enum class LasCompression
	{
		LasZip,
		LazPerf,
		None
	};

	inline std::istream& operator>>(std::istream& in, LasCompression& c)
	{
		std::string s;

		in >> s;
		s = Utils::toupper(s);
		if (s == "LASZIP" || s == "TRUE")
			c = LasCompression::LasZip;
		else if (s == "LAZPERF")
			c = LasCompression::LazPerf;
		else
			c = LasCompression::None;
		return in;
	}

	inline std::ostream& operator<<(std::ostream& out, const LasCompression& c)
	{
		switch (c)
		{
		case LasCompression::LasZip:
			out << "LasZip";
			break;
		case LasCompression::LazPerf:
			out << "LazPerf";
			break;
		case LasCompression::None:
			out << "None";
			break;
		}
		return out;
	}

	struct ExtraDim
	{
		ExtraDim(const std::string name, Dimension::Type type,
			double scale = 1.0, double offset = 0.0) :
			m_name(name), m_dimType(Dimension::Id::Unknown, type, scale, offset)
		{
			m_size = (uint8_t)Dimension::size(type);
		}
		ExtraDim(const std::string name, uint8_t size) : m_name(name),
			m_dimType(Dimension::Id::Unknown, Dimension::Type::None), m_size(size)
		{}

		friend bool operator == (const ExtraDim& ed1, const ExtraDim& ed2);

		std::string m_name;
		DimType m_dimType;
		size_t m_size;
	};

	inline bool operator == (const ExtraDim& ed1, const ExtraDim& ed2)
	{
		// This is an incomplete comparison, but it should suffice since we
		// only use it to compare an ExtraDim specified in an option with
		// one created from a VLR entry.
		return (ed1.m_name == ed2.m_name &&
			ed1.m_dimType.m_type == ed2.m_dimType.m_type &&
			ed1.m_size == ed2.m_size);
	}

	// This is the structure of each record in the extra bytes spec.  Not used
	// directly for storage, but here mostly for reference.
	struct ExtraBytesSpec
	{
		char m_reserved[2];
		uint8_t m_dataType;
		uint8_t m_options;
		char m_name[32];
		char m_reserved2[4];
		uint64_t m_noData[3]; // 24 = 3*8 bytes
		double m_min[3]; // 24 = 3*8 bytes
		double m_max[3]; // 24 = 3*8 bytes
		double m_scale[3]; // 24 = 3*8 bytes
		double m_offset[3]; // 24 = 3*8 bytes
		char m_description[32];
	};

	class ExtraBytesIf
	{
	public:
		ExtraBytesIf() : m_type(Dimension::Type::None), m_fieldCnt(0), m_size(0)
		{
			for (size_t i = 0; i < 3; ++i)
			{
				m_scale[i] = 1.0;
				m_offset[i] = 0.0;
			}
		}

		ExtraBytesIf(const std::string& name, Dimension::Type type,
			const std::string& description) :
			m_type(type), m_name(name), m_description(description), m_size(0)
		{
			for (size_t i = 0; i < 3; ++i)
			{
				// Setting the scale to 0 looks wrong, but it isn't.  If the
				// scale option flag isn't set, the scale is supposed to be 0.
				// When we write the VLR, we always clear the scale flag.
				m_scale[i] = 0.0;
				m_offset[i] = 0.0;
			}
			m_fieldCnt = (m_type == Dimension::Type::None ? 0 : 1);
		}

		void appendTo(std::vector<uint8_t>& ebBytes);
		void readFrom(const char* buf);
		uint8_t lasType();
		void setType(uint8_t lastype);
		std::vector<ExtraDim> toExtraDims();

	private:
		Dimension::Type m_type;
		unsigned m_fieldCnt; // Must be 0 - 3;
		double m_scale[3];
		double m_offset[3];
		std::string m_name;
		std::string m_description;
		size_t m_size;
	};

	namespace LasUtils
	{
		struct error : public std::runtime_error
		{
			error(const std::string& err) : std::runtime_error(err)
			{}
		};

		std::vector<ExtraDim> parse(const StringList& dimString, bool allOk);

		struct IgnoreVLR
		{
			std::string m_userId;
			uint16_t m_recordId;
		};
		std::vector<IgnoreVLR> parseIgnoreVLRs(const StringList& ignored);
	}
}

#endif // #ifndef LASUTILS_H
