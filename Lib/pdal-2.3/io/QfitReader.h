#ifndef QFITREADER_H
#define QFITREADER_H

#include <memory>
#include <vector>

#include <pdal/Reader.h>
#include <pdal/Options.h>
#include <pdal/util/IStream.h>

namespace pdal
{
	enum QFIT_Format_Type
	{
		QFIT_Format_10 = 10,
		QFIT_Format_12 = 12,
		QFIT_Format_14 = 14,
		QFIT_Format_Unknown = 128
	};

	class QfitReader : public pdal::Reader
	{
	public:
		QfitReader();

		std::string getName() const;

	private:
		QFIT_Format_Type m_format;
		std::ios::off_type m_point_bytes;
		std::size_t m_offset;
		uint32_t m_size;
		bool m_flip_x;
		double m_scale_z;
		bool m_littleEndian;
		point_count_t m_numPoints;
		std::unique_ptr<IStream> m_istream;
		point_count_t m_index;

		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void ready(PointTableRef table);
		virtual point_count_t read(PointViewPtr buf, point_count_t count);
		virtual void done(PointTableRef table);

		QfitReader& operator=(const QfitReader&) = delete;
		QfitReader(const QfitReader&) = delete;
	};
}

#endif // #ifndef QFITREADER_H
