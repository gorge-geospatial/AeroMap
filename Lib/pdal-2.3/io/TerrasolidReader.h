#ifndef TERRASOLIDREADER_H
#define TERRASOLIDREADER_H

#include <pdal/Options.h>
#include <pdal/Reader.h>
#include <pdal/util/IStream.h>

#include <memory>
#include <vector>

namespace pdal
{
	enum TERRASOLID_Format_Type
	{
		TERRASOLID_Format_1 = 20010712,
		TERRASOLID_Format_2 = 20020715,
		TERRASOLID_Format_Unknown = 999999999
	};

	struct TerraSolidHeader
	{
		TerraSolidHeader() :
			HdrSize(0),
			HdrVersion(0),
			RecogVal(0),
			PntCnt(0),
			Units(0),
			OrgX(0),
			OrgY(0),
			OrgZ(0),
			Time(0),
			Color(0)
		{}

		int32_t HdrSize;
		int32_t HdrVersion;
		int32_t RecogVal;
		char RecogStr[4];
		int32_t PntCnt;
		int32_t Units;
		double OrgX;
		double OrgY;
		double OrgZ;
		int32_t Time;
		int32_t Color;
	};

	typedef std::unique_ptr<TerraSolidHeader> TerraSolidHeaderPtr;

	class TerrasolidReader : public pdal::Reader
	{
	public:
		TerrasolidReader() : pdal::Reader(),
			m_format(TERRASOLID_Format_Unknown)
		{}
		std::string getName() const;

		point_count_t getNumPoints() const
		{
			return m_header->PntCnt;
		}

		const TerraSolidHeader& getHeader() const { return *m_header; }

		// this is called by the stage's iterator
		uint32_t processBuffer(PointViewPtr view, std::istream& stream,
			uint64_t numPointsLeft) const;

	private:
		TerraSolidHeaderPtr m_header;
		TERRASOLID_Format_Type m_format;
		uint32_t m_size;
		bool m_haveColor;
		bool m_haveTime;
		uint32_t m_baseTime;
		std::unique_ptr<IStream> m_istream;
		point_count_t m_index;

		virtual void initialize();
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void ready(PointTableRef table);
		virtual point_count_t read(PointViewPtr view, point_count_t count);
		virtual void done(PointTableRef table);
		virtual bool eof()
		{
			return m_index >= getNumPoints();
		}

		TerrasolidReader& operator=(const TerrasolidReader&); // not implemented
		TerrasolidReader(const TerrasolidReader&); // not implemented
	};
}

#endif // #ifndef TERRASOLIDREADER_H
