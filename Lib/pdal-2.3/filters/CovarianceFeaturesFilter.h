#ifndef COVARIANCEFEATURESFILTER_H
#define COVARIANCEFEATURESFILTER_H

#include <thread>

#include <pdal/Filter.h>

namespace pdal
{
	class CovarianceFeaturesFilter : public Filter
	{
	public:
		CovarianceFeaturesFilter() {}
		CovarianceFeaturesFilter& operator=(const CovarianceFeaturesFilter&) = delete;
		CovarianceFeaturesFilter(const CovarianceFeaturesFilter&) = delete;

		std::string getName() const;

	private:
		enum class Mode
		{
			Raw,
			SQRT,
			Normalized
		};

		int m_knn;
		int m_threads;
		StringList m_featureSetString;
		std::vector<Dimension::Id> m_extraDims;
		size_t m_stride;
		double m_radius;
		int m_minK;
		Mode m_mode;
		Arg* m_radiusArg;
		bool m_optimal;

		virtual void addDimensions(PointLayoutPtr layout);
		virtual void addArgs(ProgramArgs& args);
		virtual void filter(PointView& view);
		virtual void prepared(PointTableRef table);

		void setDimensionality(PointView& view, const PointId& id, const KD3Index& kid);

		friend std::istream& operator>>(std::istream& in, CovarianceFeaturesFilter::Mode& mode);
		friend std::ostream& operator<<(std::ostream& in, const CovarianceFeaturesFilter::Mode& mode);
	};
}

#endif // #ifndef COVARIANCEFEATURESFILTER_H
