#ifndef PROJPIPELINEFILTER_H
#define PROJPIPELINEFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

#include <memory>

class OGRCoordinateTransformation;

namespace pdal
{
	class ProjPipelineFilter : public Filter, public Streamable
	{
	public:
		class CoordTransform;

		ProjPipelineFilter();
		~ProjPipelineFilter();

		std::string getName() const;

	private:
		ProjPipelineFilter& operator=(const ProjPipelineFilter&) = delete;
		ProjPipelineFilter(const ProjPipelineFilter&) = delete;

		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual PointViewSet run(PointViewPtr view);
		virtual bool processOne(PointRef& point);

		void createTransform(const std::string coordOperation, bool reverseTransfo);

		SpatialReference m_outSRS;
		bool m_reverseTransfo;
		std::string m_coordOperation;
		std::unique_ptr<CoordTransform> m_coordTransform;
	};


	class ProjPipelineFilter::CoordTransform
	{
	public:
		CoordTransform();
		CoordTransform(const std::string coordOperation, bool reverseTransfo);

		bool transform(double& x, double& y, double& z);
	private:
		std::unique_ptr<OGRCoordinateTransformation> m_transform;

	};
}

#endif // #ifndef PROJPIPELINEFILTER_H
