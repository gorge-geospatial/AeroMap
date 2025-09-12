#ifndef EPTINFO_H
#define EPTINFO_H

#include <pdal/DimType.h>
#include <pdal/SpatialReference.h>
#include <pdal/util/Bounds.h>

#include "FixedPointLayout.h"

namespace pdal
{
	class Connector;

	class EptInfo
	{
	public:
		enum class DataType
		{
			Laszip,
			Binary,
			Zstandard
		};

		EptInfo(const std::string& info);
		EptInfo(const std::string& filename, const Connector& connector);

		const BOX3D& bounds() const { return m_bounds; }
		const BOX3D& boundsConforming() const { return m_boundsConforming; }
		uint64_t points() const { return m_points; }
		uint64_t span() const { return m_span; }
		DataType dataType() const { return m_dataType; }
		const SpatialReference& srs() const { return m_srs; }
		const NL::json& json() { return m_info; }
		std::map<std::string, DimType>& dims() { return m_dims; }
		DimType dimType(Dimension::Id id) const;
		PointLayout& remoteLayout() const { return m_remoteLayout; }
		std::string dataDir() const;
		std::string hierarchyDir() const;
		std::string sourcesDir() const;

	private:
		// Info comes from the values here:
		// https://entwine.io/entwine-point-tile.html#ept-json
		NL::json m_info;
		BOX3D m_bounds;
		BOX3D m_boundsConforming;
		uint64_t m_points = 0;
		std::map<std::string, DimType> m_dims;

		// The span is the length, width, and depth of the octree grid.  For
		// example, a dataset oriented as a 256*256*256 octree grid would have a
		// span of 256.
		//
		// See: https://entwine.io/entwine-point-tile.html#span
		uint64_t m_span = 0;
		DataType m_dataType;
		SpatialReference m_srs;
		std::string m_filename;
		mutable FixedPointLayout m_remoteLayout;

		void initialize();
	};
}

#endif // #ifndef EPTINFO_H
