#ifndef REPROJECTIONFILTER_H
#define REPROJECTIONFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

#include <memory>

namespace pdal
{
	class SrsTransform;

	class ReprojectionFilter : public Filter, public Streamable
	{
	public:
		ReprojectionFilter();
		~ReprojectionFilter();

		std::string getName() const;

	private:
		ReprojectionFilter& operator=(const ReprojectionFilter&) = delete;
		ReprojectionFilter(const ReprojectionFilter&) = delete;

		virtual void addArgs(ProgramArgs& args);
		virtual void initialize();
		virtual PointViewSet run(PointViewPtr view);
		virtual bool processOne(PointRef& point);
		virtual void spatialReferenceChanged(const SpatialReference& srs);
		virtual void prepared(PointTableRef table);

		void createTransform(const SpatialReference& srs);

		SpatialReference m_inSRS;
		SpatialReference m_outSRS;
		bool m_inferInputSRS;
		std::unique_ptr<SrsTransform> m_transform;
		std::vector<std::string> m_inAxisOrderingArg;
		std::vector<std::string> m_outAxisOrderingArg;
		std::vector<int> m_inAxisOrdering;
		std::vector<int> m_outAxisOrdering;
		bool m_errorOnFailure;
	};
}

#endif // #ifndef REPROJECTIONFILTER_H
