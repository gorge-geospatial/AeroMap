#ifndef PCDHEADER_H
#define PCDHEADER_H

#include <Eigen/Dense>

#include <pdal/Dimension.h>
#include <pdal/util/OStream.h>

namespace pdal
{
	enum class PcdFieldType
	{
		unknown,
		I,
		U,
		F
	};
	std::istream& operator>>(std::istream& in, PcdFieldType& type);
	std::ostream& operator<<(std::ostream& out, PcdFieldType& type);

	enum class PcdVersion
	{
		unknown,
		PCD_V6,
		PCD_V7
	};
	std::istream& operator>>(std::istream& in, PcdVersion& version);
	std::ostream& operator<<(std::ostream& out, PcdVersion& version);

	enum class PcdDataStorage
	{
		unknown,
		ASCII,
		BINARY,
		COMPRESSED
	};
	std::istream& operator>>(std::istream& in, PcdDataStorage& storage);
	std::ostream& operator<<(std::ostream& out, PcdDataStorage& storage);

	struct PcdField
	{
		PcdField()
			: m_id(Dimension::Id::Unknown), m_size(4),
			m_type(PcdFieldType::unknown), m_count(1)
		{
		}

		PcdField(std::string& label) : PcdField()
		{
			m_id = Dimension::id(label);
			m_label = label;
		}

		std::string m_label;
		Dimension::Id m_id;
		uint32_t m_size;
		PcdFieldType m_type;
		uint32_t m_count;
	};
	typedef std::vector<PcdField> PcdFieldList;

	struct PcdHeader
	{
		PcdHeader();

		void clear();

		PcdVersion m_version;
		PcdFieldList m_fields;
		point_count_t m_width;
		point_count_t m_height;
		point_count_t m_pointCount;

		Eigen::Vector4f m_origin;
		Eigen::Quaternionf m_orientation;

		PcdDataStorage m_dataStorage;
		std::istream::pos_type m_dataOffset;
		size_t m_numLines;
	};

	std::istream& operator>>(std::istream& in, PcdHeader& header);
	std::ostream& operator<<(std::ostream& out, PcdHeader& header);
	OLeStream& operator<<(OLeStream& out, PcdHeader& header);
}

#endif // #ifndef PCDHEADER_H
