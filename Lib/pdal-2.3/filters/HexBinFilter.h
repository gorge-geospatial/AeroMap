#ifndef HEXBINFILTER_H
#define HEXBINFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>
#include <pdal/util/ProgramArgs.h>

namespace hexer
{
	class HexGrid;
};

namespace pdal
{
	class HexBin : public Filter, 
				   public Streamable
	{
	public:
		HexBin();
		~HexBin();

		HexBin& operator=(const HexBin&) = delete;
		HexBin(const HexBin&) = delete;

		std::string getName() const;
		hexer::HexGrid* grid() const;

	private:
		std::unique_ptr<hexer::HexGrid> m_grid;
		std::string m_xDimName;
		std::string m_yDimName;
		uint32_t m_precision;
		uint32_t m_sampleSize;
		double m_cullArea;
		Arg* m_cullArg;
		int32_t m_density;
		double m_edgeLength;
		bool m_outputTesselation;
		bool m_doSmooth;
		point_count_t m_count;
		bool m_preserve_topology;

		virtual void addArgs(ProgramArgs& args);
		virtual void ready(PointTableRef table);
		virtual void filter(PointView& view);
		virtual bool processOne(PointRef& point);
		virtual void done(PointTableRef table);
	};
}

#endif // #ifndef HEXBINFILTER_H
