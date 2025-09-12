#ifndef SMRFILTER_H
#define SMRFILTER_H

#include <pdal/Filter.h>

#include <memory>
#include <string>

namespace pdal
{
	struct SMRArgs;

	class SMRFilter : public Filter
	{
	public:
		SMRFilter();
		~SMRFilter();
		SMRFilter& operator=(const SMRFilter&) = delete;
		SMRFilter(const SMRFilter&) = delete;

		std::string getName() const;

	private:
		int m_rows;
		int m_cols;
		BOX2D m_bounds;
		SpatialReference m_srs;
		std::unique_ptr<SMRArgs> m_args;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void prepared(PointTableRef table);
		virtual void ready(PointTableRef table);
		virtual PointViewSet run(PointViewPtr view);

		void classifyGround(PointViewPtr, std::vector<double>&);
		std::vector<int> createLowMask(std::vector<double> const&);
		std::vector<int> createNetMask();
		std::vector<int> createObjMask(std::vector<double> const&);
		std::vector<double> createZImin(PointViewPtr view);
		std::vector<double> createZInet(std::vector<double> const&, std::vector<int> const&);
		std::vector<double> createZIpro(PointViewPtr, std::vector<double> const&,
			std::vector<int> const&,
			std::vector<int> const&,
			std::vector<int> const&);
		void knnfill(PointViewPtr, std::vector<double>&);
		std::vector<int> progressiveFilter(std::vector<double> const&, double, double);
	};
}

#endif // #ifndef SMRFILTER_H
