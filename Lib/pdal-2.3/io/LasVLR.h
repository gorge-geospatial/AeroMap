#ifndef LASVLR_H
#define LASVLR_H

#include <limits>
#include <string>
#include <vector>

#include <pdal/SpatialReference.h>
#include <pdal/util/IStream.h>
#include <pdal/util/OStream.h>

namespace pdal
{
	static const int WKT_RECORD_ID = 2112;
	static const uint16_t GEOTIFF_DIRECTORY_RECORD_ID = 34735;
	static const uint16_t GEOTIFF_DOUBLES_RECORD_ID = 34736;
	static const uint16_t GEOTIFF_ASCII_RECORD_ID = 34737;
	static const uint16_t LASZIP_RECORD_ID = 22204;
	static const uint16_t EXTRA_BYTES_RECORD_ID = 4;
	static const uint16_t PDAL_METADATA_RECORD_ID = 12;
	static const uint16_t PDAL_PIPELINE_RECORD_ID = 13;

	static const char TRANSFORM_USER_ID[] = "LASF_Projection";
	static const char SPEC_USER_ID[] = "LASF_Spec";
	static const char LIBLAS_USER_ID[] = "liblas";
	static const char LASZIP_USER_ID[] = "laszip encoded";
	static const char PDAL_USER_ID[] = "PDAL";

	class LasVLR;
	typedef std::vector<LasVLR> VlrList;

	class LasVLR
	{
	public:
		static const uint16_t MAX_DATA_SIZE;

		LasVLR(const std::string& userId, uint16_t recordId,
			const std::string& description, std::vector<uint8_t>& data) :
			m_userId(userId), m_recordId(recordId), m_description(description),
			m_data(std::move(data)), m_recordSig(0)
		{}
		LasVLR() : m_recordId(0), m_recordSig(0)
		{}

		std::string userId() const
		{
			return m_userId;
		}
		uint16_t recordId() const
		{
			return m_recordId;
		}
		std::string description() const
		{
			return m_description;
		}

		bool matches(const std::string& userId) const
		{
			return userId == m_userId;
		}
		bool matches(const std::string& userId, uint16_t recordId) const
		{
			return matches(userId) && (recordId == m_recordId);
		}

		const char* data() const
		{
			return (const char*)m_data.data();
		}
		char* data()
		{
			return (char*)m_data.data();
		}
		bool isEmpty() const
		{
			return m_data.size() == 0;
		}
		uint64_t dataLen() const
		{
			return m_data.size();
		}
		void setDataLen(uint64_t size)
		{
			m_data.resize((size_t)size);
		}
		void write(OLeStream& out, uint16_t recordSig);
		bool read(ILeStream& in, size_t limit);

		friend OLeStream& operator<<(OLeStream& out, const LasVLR& v);
		friend std::istream& operator>>(std::istream& in, LasVLR& v);
		friend std::ostream& operator<<(std::ostream& out, const LasVLR& v);

	protected:
		std::string m_userId;
		uint16_t m_recordId;
		std::string m_description;
		std::vector<uint8_t> m_data;
		uint16_t m_recordSig;
	};

	class ExtLasVLR : public LasVLR
	{
	public:
		ExtLasVLR(const std::string& userId, uint16_t recordId,
			const std::string& description, std::vector<uint8_t>& data) :
			LasVLR(userId, recordId, description, data)
		{}
		ExtLasVLR()
		{}

		bool read(ILeStream& in, uintmax_t limit);

		friend OLeStream& operator<<(OLeStream& out, const ExtLasVLR& v);
		friend std::istream& operator>>(std::istream& in, ExtLasVLR& v);
		friend std::ostream& operator<<(std::ostream& out, const ExtLasVLR& v);
	};
}

#endif // #ifndef LASVLR_H
