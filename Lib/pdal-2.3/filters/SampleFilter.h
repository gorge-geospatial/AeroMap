#ifndef SAMPLEFILTER_H
#define SAMPLEFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

namespace pdal
{
	class SampleFilter : public Filter, public Streamable
	{
		using Voxel = std::tuple<int, int, int>;
		using Coord = std::tuple<double, double, double>;
		using CoordList = std::vector<Coord>;

	public:
		SampleFilter() : Filter() {}
		SampleFilter& operator=(const SampleFilter&) = delete;
		SampleFilter(const SampleFilter&) = delete;

		std::string getName() const;

	private:
		double m_cell;
		Arg* m_cellArg;
		double m_radius;
		double m_radiusSqr;
		Arg* m_radiusArg;
		double m_originX;
		double m_originY;
		double m_originZ;
		std::map<Voxel, CoordList> m_populatedVoxels;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual bool processOne(PointRef& point);
		virtual void ready(PointTableRef);
		virtual PointViewSet run(PointViewPtr view);

		bool voxelize(PointRef& point);
	};
}

#endif // #ifndef SAMPLEFILTER_H
