#ifndef ITERATIVECLOSESTPOINT_H
#define ITERATIVECLOSESTPOINT_H

#include <pdal/Filter.h>

namespace pdal
{
	class IterativeClosestPoint : public Filter
	{
	public:
		IterativeClosestPoint() : Filter(), m_fixed(nullptr), m_complete(false)
		{}

		std::string getName() const;

	private:
		int m_max_iters;
		int m_max_similar;
		double m_rotation_threshold;
		double m_translation_threshold;
		double m_mse_abs;
		Arg* m_maxdistArg;
		double m_maxdist;
		Arg* m_matrixArg;
		std::string m_matrixStr;
		std::vector<double> m_vec;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);
		virtual void done(PointTableRef _);
		PointViewPtr icp(PointViewPtr fixed, PointViewPtr moving) const;

		PointViewPtr m_fixed;
		bool m_complete;

		IterativeClosestPoint&
			operator=(const IterativeClosestPoint&);             // not implemented
		IterativeClosestPoint(const IterativeClosestPoint&); // not implemented
	};
}

#endif // #ifndef ITERATIVECLOSESTPOINT_H
