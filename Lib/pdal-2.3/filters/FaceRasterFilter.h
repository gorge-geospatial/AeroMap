#ifndef FACERASTERFILTER_H
#define FACERASTERFILTER_H

#include <memory>
#include <string>

#include <pdal/Filter.h>

namespace pdal
{
	struct RasterLimits;

	class FaceRasterFilter : public pdal::Filter
	{
	public:
		FaceRasterFilter();
		FaceRasterFilter& operator=(const FaceRasterFilter&) = delete;
		FaceRasterFilter(const FaceRasterFilter&) = delete;

		std::string getName() const;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef);
		virtual void filter(PointView& view);

		std::unique_ptr<RasterLimits> m_limits;
		std::string m_meshName;
		double m_noData;
		bool m_computeLimits;
	};
}

#endif // #ifndef FACERASTERFILTER_H
