#ifndef VOXELDOWNSIZEFILTER_H
#define VOXELDOWNSIZEFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

namespace pdal
{
	class PointLayout;
	class PointView;

	class VoxelDownsizeFilter : public Filter, public Streamable
	{
		using Voxel = std::tuple<int, int, int>;
		enum class Mode
		{
			First,
			Center
		};
	public:
		VoxelDownsizeFilter();
		VoxelDownsizeFilter& operator=(const VoxelDownsizeFilter&) = delete;
		VoxelDownsizeFilter(const VoxelDownsizeFilter&) = delete;

		std::string getName() const override;

	private:
		virtual void addArgs(ProgramArgs& args) override;
		virtual PointViewSet run(PointViewPtr view) override;
		virtual void ready(PointTableRef) override;
		virtual bool processOne(PointRef& point) override;

		bool voxelize(PointRef& point);

		double m_cell;
		double m_originX;
		double m_originY;
		double m_originZ;
		std::set<Voxel> m_populatedVoxels;
		Mode m_mode;

		friend std::istream& operator>>(std::istream& in,
			VoxelDownsizeFilter::Mode&);
		friend std::ostream& operator<<(std::ostream& out,
			const VoxelDownsizeFilter::Mode&);
	};
}

#endif // #ifndef VOXELDOWNSIZEFILTER_H
