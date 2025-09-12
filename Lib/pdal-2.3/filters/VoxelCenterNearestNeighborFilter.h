#ifndef VOXELCENTERNEARESTNEIGHBORFILTER_H
#define VOXELCENTERNEARESTNEIGHBORFILTER_H

#include <pdal/Filter.h>

#include <cstdint>
#include <string>

namespace pdal
{
	class PointLayout;
	class PointView;

	class VoxelCenterNearestNeighborFilter : public Filter
	{
	public:
		VoxelCenterNearestNeighborFilter() : Filter()
		{
		}

		std::string getName() const;

	private:
		double m_cell;

		virtual void addArgs(ProgramArgs& args);
		virtual PointViewSet run(PointViewPtr view);

		VoxelCenterNearestNeighborFilter& operator=(const VoxelCenterNearestNeighborFilter&); // not implemented
		VoxelCenterNearestNeighborFilter(const VoxelCenterNearestNeighborFilter&); // not implemented
	};
}

#endif // #ifndef VOXELCENTERNEARESTNEIGHBORFILTER_H
