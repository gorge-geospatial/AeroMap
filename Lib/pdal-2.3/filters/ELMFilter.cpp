/******************************************************************************
 * Copyright (c) 2017, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

 // PDAL implementation of the Extended Local Minimum (ELM) method as published
 // in Z. Chen, B. Devereux, B. Gao, and G. Amable, “Upward-fusion urban DTM
 // generating method using airborne Lidar data,” ISPRS J. Photogramm. Remote
 // Sens., vol. 72, pp. 121–130, 2012.

#include "ELMFilter.h"

#include <map>
#include <string>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.elm",
		"Marks low points as noise.",
		"http://pdal.io/stages/filters.elm.html"
	};

	CREATE_STATIC_STAGE(ELMFilter, s_info)

	std::string ELMFilter::getName() const
	{
		return s_info.name;
	}

	void ELMFilter::addArgs(ProgramArgs& args)
	{
		args.add("cell", "Cell size", m_cell, 10.0);
		args.add("class", "Class to use for noise points", m_class, ClassLabel::LowPoint);
		args.add("threshold", "Threshold value", m_threshold, 1.0);
	}

	void ELMFilter::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDim(Dimension::Id::Classification);
	}

	void ELMFilter::filter(PointView& view)
	{
		if (!view.size())
			return;

		BOX2D bounds;
		view.calculateBounds(bounds);

		size_t cols =
			static_cast<size_t>(((bounds.maxx - bounds.minx) / m_cell) + 1);
		size_t rows =
			static_cast<size_t>(((bounds.maxy - bounds.miny) / m_cell) + 1);

		// Make an initial pass through the input PointView to index elevation
		// values and PointIds by row and column.
		std::map<uint32_t, std::multimap<double, PointId>> hash;
		for (PointId id = 0; id < view.size(); ++id)
		{
			double x = view.getFieldAs<double>(Dimension::Id::X, id);
			double y = view.getFieldAs<double>(Dimension::Id::Y, id);
			double z = view.getFieldAs<double>(Dimension::Id::Z, id);

			size_t c = static_cast<size_t>(std::floor(x - bounds.minx) / m_cell);
			size_t r = static_cast<size_t>(std::floor(y - bounds.miny) / m_cell);

			hash[(unsigned int)(c * rows + r)].emplace(z, id);
		}

		// Count the number of points we classify as noise.
		point_count_t num(0);

		// Make a second pass through the now rasterized PointView to compute the
		// extended local minimum.
		for (size_t c = 0; c < cols; ++c)
		{
			for (size_t r = 0; r < rows; ++r)
			{
				std::multimap<double, PointId> ids(hash[(unsigned int)(c * rows + r)]);

				if (ids.size() <= 1)
					continue;

				for (auto it = ids.begin(); it != std::prev(ids.end()); ++it)
				{
					// Where the current value is sufficiently close to the next, we
					// consider that this is not a low outlier and break the current
					// loop.
					if (std::fabs(it->first - std::next(it)->first) < m_threshold)
						break;

					// Otherwise this point is classified as noise, and we proceed
					// to the next lowest value.
					view.setField(Dimension::Id::Classification, it->second,
						m_class);
					++num;
				}
			}
		}

		log()->get(LogLevel::Info)
			<< "Classified " << num
			<< " points as noise by Extended Local Minimum (ELM).\n";
	}
}
