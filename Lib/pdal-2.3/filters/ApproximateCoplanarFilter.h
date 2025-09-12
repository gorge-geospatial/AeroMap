#ifndef APPROXIMATECOPLANARFILTER_H
#define APPROXIMATECOPLANARFILTER_H

#include <pdal/Filter.h>

#include <string>

namespace pdal
{
	class PointLayout;
	class PointView;

	class ApproximateCoplanarFilter : public Filter
	{
	public:
		ApproximateCoplanarFilter() : Filter()
		{}
		ApproximateCoplanarFilter& operator=(
			const ApproximateCoplanarFilter&) = delete;
		ApproximateCoplanarFilter(const ApproximateCoplanarFilter&) = delete;

		std::string getName() const;

	private:
		int m_knn;
		double m_thresh1;
		double m_thresh2;

		virtual void addDimensions(PointLayoutPtr layout);
		virtual void addArgs(ProgramArgs& args);
		virtual void filter(PointView& view);
	};
}

#endif // #ifndef APPROXIMATECOPLANARFILTER_H
