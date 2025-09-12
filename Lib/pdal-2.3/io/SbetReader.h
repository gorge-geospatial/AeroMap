#ifndef SBETREADER_H
#define SBETREADER_H

#include <pdal/PointView.h>
#include <pdal/Reader.h>
#include <pdal/Streamable.h>
#include <pdal/util/IStream.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	class SbetReader : public Reader, public Streamable
	{
	public:
		SbetReader() : Reader()
		{}

		std::string getName() const;

	private:
		std::unique_ptr<ILeStream> m_stream;
		// Number of points in the file.
		point_count_t m_numPts;
		point_count_t m_index;
		Dimension::IdList m_dims;
		bool m_anglesAsDegrees;

		virtual bool processOne(PointRef& point);
		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void ready(PointTableRef table);
		virtual point_count_t read(PointViewPtr view, point_count_t count);
		virtual bool eof();

		void seek(PointId idx);
	};
}

#endif // #ifndef SBETREADER_H
