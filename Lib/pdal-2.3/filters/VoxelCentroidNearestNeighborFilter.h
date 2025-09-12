#ifndef VOXELCENTROIDNEARESTNEIGHBORFILTER_H
#define VOXELCENTROIDNEARESTNEIGHBORFILTER_H

#include <pdal/Filter.h>

#include <cstdint>
#include <string>

namespace pdal
{
	class PointLayout;
	class PointView;

	class VoxelCentroidNearestNeighborFilter : public Filter
	{
	public:
		VoxelCentroidNearestNeighborFilter() : Filter()
		{
		}

		std::string getName() const;

	private:
		double m_cell;

		virtual void addArgs(ProgramArgs& args);
		virtual PointViewSet run(PointViewPtr view);

		VoxelCentroidNearestNeighborFilter&	operator=(const VoxelCentroidNearestNeighborFilter&); // not implemented
		VoxelCentroidNearestNeighborFilter(const VoxelCentroidNearestNeighborFilter&); // not implemented
	};
}

#endif // #ifndef VOXELCENTROIDNEARESTNEIGHBORFILTER_H
