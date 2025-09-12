#ifndef RELAXATIONDARTTHROWING_H
#define RELAXATIONDARTTHROWING_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class Options;

	class RelaxationDartThrowing : public pdal::Filter
	{
	public:
		RelaxationDartThrowing() : Filter() {}
		RelaxationDartThrowing& operator=(const RelaxationDartThrowing&) = delete;
		RelaxationDartThrowing(const RelaxationDartThrowing&) = delete;

		std::string getName() const;

	private:
		double m_decay;
		double m_startRadius;
		point_count_t m_maxSize;
		bool m_shuffle;
		Arg* m_seedArg;
		unsigned m_seed;

		virtual void addArgs(ProgramArgs& args);
		virtual PointViewSet run(PointViewPtr view);
	};
}

#endif // #ifndef RELAXATIONDARTTHROWING_H
