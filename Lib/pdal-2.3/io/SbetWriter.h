#ifndef SBETWRITER_H
#define SBETWRITER_H

#include <pdal/util/OStream.h>
#include <pdal/Writer.h>

#include "SbetCommon.h"

namespace pdal
{
	class SbetWriter : public Writer
	{
	public:
		std::string getName() const;

	private:
		std::unique_ptr<OLeStream> m_stream;
		std::string m_filename;
		bool m_anglesAreDegrees;

		virtual void addArgs(ProgramArgs& args);
		virtual void ready(PointTableRef table);
		virtual void write(const PointViewPtr view);
		virtual void done(PointTableRef table);
	};
}

#endif // #ifndef SBETWRITER_H
