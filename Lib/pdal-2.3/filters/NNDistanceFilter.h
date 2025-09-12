#ifndef NNDISTANCEFILTER_H
#define NNDISTANCEFILTER_H

#include <pdal/Filter.h>

namespace pdal
{
	class PointLayout;
	class PointView;
	class ProgramArgs;

	class NNDistanceFilter : public Filter
	{
	public:
		NNDistanceFilter();

		NNDistanceFilter& operator=(const NNDistanceFilter&) = delete;
		NNDistanceFilter(const NNDistanceFilter&) = delete;

		std::string getName() const;

	private:
		enum class Mode
		{
			Kth,
			Average
		};

		size_t m_k;
		Mode m_mode;

		virtual void addArgs(ProgramArgs& args);
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void filter(PointView& view);

		friend std::istream& operator>>(std::istream& in,
			NNDistanceFilter::Mode& mode);
		friend std::ostream& operator<<(std::ostream& in,
			const NNDistanceFilter::Mode& mode);
	};
}

#endif // #ifndef NNDISTANCEFILTER_H
