/******************************************************************************
 * Copyright (c) 2019, Helix.re
 * Contact Person : Pravin Shinde (pravin@helix.re,
 *                    https://github.com/pravinshinde825)
 ****************************************************************************/

#include "VoxelDownsizeFilter.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.voxeldownsize",
		"First Entry Voxel Filter",
		"http://pdal.io/stages/filters.voxeldownsize.html"
	};

	CREATE_STATIC_STAGE(VoxelDownsizeFilter, s_info)

	std::istream& operator>>(std::istream& in, VoxelDownsizeFilter::Mode& mode)
	{
		std::string s;
		in >> s;

		s = Utils::tolower(s);
		if (s == "center")
			mode = VoxelDownsizeFilter::Mode::Center;
		else if (s == "first")
			mode = VoxelDownsizeFilter::Mode::First;
		else
			throw pdal_error("filters.voxeldownsize: Invalid 'mode' option '" +
				s + "'. " "Valid options are 'center' and 'first'");
		return in;
	}

	std::ostream& operator<<(std::ostream& out, const VoxelDownsizeFilter::Mode& mode)
	{
		switch (mode)
		{
		case VoxelDownsizeFilter::Mode::Center:
			out << "center";
			break;
		case VoxelDownsizeFilter::Mode::First:
			out << "first";
			break;
		}
		return out;
	}

	VoxelDownsizeFilter::VoxelDownsizeFilter()
	{}

	std::string VoxelDownsizeFilter::getName() const
	{
		return s_info.name;
	}

	void VoxelDownsizeFilter::addArgs(ProgramArgs& args)
	{
		args.add("cell", "Cell size", m_cell, 0.001);
		args.add("mode", "Method for downsizing : center / first",
			m_mode, Mode::Center);
	}

	void VoxelDownsizeFilter::ready(PointTableRef)
	{
		m_populatedVoxels.clear();
	}

	PointViewSet VoxelDownsizeFilter::run(PointViewPtr view)
	{
		PointViewPtr output = view->makeNew();
		PointRef point(*view);
		for (PointId id = 0; id < view->size(); ++id)
		{
			point.setPointId(id);
			if (voxelize(point))
				output->appendPoint(*view, id);
		}

		PointViewSet viewSet;
		viewSet.insert(output);
		return viewSet;
	}

	bool VoxelDownsizeFilter::voxelize(PointRef& point)
	{
		/*
		 * Calculate the voxel coordinates for the incoming point.
		 * gx, gy, gz will be the global coordinates from (0, 0, 0).
		 */
		double x = point.getFieldAs<double>(Dimension::Id::X);
		double y = point.getFieldAs<double>(Dimension::Id::Y);
		double z = point.getFieldAs<double>(Dimension::Id::Z);
		if (m_populatedVoxels.empty())
		{
			m_originX = x - (m_cell / 2);
			m_originY = y - (m_cell / 2);
			m_originZ = z - (m_cell / 2);
		}

		// Offset by origin.
		x -= m_originX;
		y -= m_originY;
		z -= m_originZ;

		Voxel v = std::make_tuple((int)(std::floor(x / m_cell)),
			(int)(std::floor(y / m_cell)), (int)(std::floor(z / m_cell)));

		auto inserted = m_populatedVoxels.insert(v).second;
		if ((m_mode == Mode::Center) && inserted)
		{
			point.setField(Dimension::Id::X,
				(std::get<0>(v) + 0.5) * m_cell + m_originX);
			point.setField(Dimension::Id::Y,
				(std::get<1>(v) + 0.5) * m_cell + m_originY);
			point.setField(Dimension::Id::Z,
				(std::get<2>(v) + 0.5) * m_cell + m_originZ);
		}
		return inserted;
	}

	bool VoxelDownsizeFilter::processOne(PointRef& point)
	{
		return voxelize(point);
	}
}
