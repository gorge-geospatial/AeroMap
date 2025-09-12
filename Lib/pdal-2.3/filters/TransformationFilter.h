#ifndef TRANSFORMATIONFILTER_H
#define TRANSFORMATIONFILTER_H

#include <array>
#include <string>

#include <pdal/Filter.h>
#include <pdal/Streamable.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	class TransformationFilter : public Filter, public Streamable
	{
	public:
		class Transform;

		TransformationFilter();
		~TransformationFilter();
		TransformationFilter& operator=(const TransformationFilter&) = delete;
		TransformationFilter(const TransformationFilter&) = delete;

		std::string getName() const override;
		void doFilter(PointView& view, const Transform& matrix);

	private:
		virtual void addArgs(ProgramArgs& args) override;
		virtual void initialize() override;
		virtual bool processOne(PointRef& point) override;
		virtual void filter(PointView& view) override;
		virtual void spatialReferenceChanged(const SpatialReference& srs) override;

		std::unique_ptr<Transform> m_matrix;
		SpatialReference m_overrideSrs;
		bool m_invert;
	};

	class TransformationFilter::Transform
	{
	public:
		static const size_t RowSize = 4;
		static const size_t ColSize = 4;
		static const size_t Size = RowSize * ColSize;
		typedef double ValueType;
		typedef std::array<ValueType, Size> ArrayType;

		Transform();
		Transform(const ArrayType& arr);

		double operator[](size_t off) const
		{
			return m_vals[off];
		}
		double& operator[](size_t off)
		{
			return m_vals[off];
		}

	private:
		ArrayType m_vals;

		friend std::istream& operator>>(std::istream& in, pdal::TransformationFilter::Transform& xform);
		friend std::ostream& operator<<(std::ostream& out, const pdal::TransformationFilter::Transform& xform);
	};
}

#endif // #ifndef TRANSFORMATIONFILTER_H
