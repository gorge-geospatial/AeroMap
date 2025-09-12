#ifndef NULLWRITER_H
#define NULLWRITER_H

#include <pdal/Writer.h>

namespace pdal
{
	class NullWriter : public Writer
	{
	public:
		std::string getName() const;
	private:
		virtual void write(const PointViewPtr /*view*/)
		{}
	};
}

#endif // #ifndef NULLWRITER_H
