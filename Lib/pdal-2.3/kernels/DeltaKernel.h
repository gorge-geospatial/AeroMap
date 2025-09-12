#ifndef DELTAKERNEL_H
#define DELTAKERNEL_H

#include <map>

#include <pdal/KDIndex.h>
#include <pdal/Kernel.h>
#include <pdal/PointView.h>

namespace pdal
{
	struct DimIndex
	{
		std::string m_name;
		Dimension::Id m_srcId;
		Dimension::Id m_candId;
		double m_min;
		double m_max;
		double m_avg;
		point_count_t m_cnt;

		DimIndex() 
			: m_srcId(Dimension::Id::Unknown)
			, m_candId(Dimension::Id::Unknown)
			, m_min((std::numeric_limits<double>::max)())
			, m_max((std::numeric_limits<double>::lowest)())
			, m_avg(0.0)
			, m_cnt(0)
		{
		}
	};
	typedef std::map<std::string, DimIndex> DimIndexMap;

	class DeltaKernel : public Kernel
	{
	public:
		std::string getName() const override;
		int execute() override;
		DeltaKernel();

	private:
		void addSwitches(ProgramArgs& args);
		PointViewPtr loadSet(const std::string& filename, PointTableRef table);
		MetadataNode dump(PointViewPtr& srcView, PointViewPtr& candView, KD3Index& index, DimIndexMap& dims);
		MetadataNode dumpDetail(PointViewPtr& srcView, PointViewPtr& candView, KD3Index& index, DimIndexMap& dims);
		void accumulate(DimIndex& d, double v);

		std::string m_sourceFile;
		std::string m_candidateFile;

		bool m_detail;
		bool m_allDims;
	};
}

#endif // #ifndef DELTAKERNEL_H
