/******************************************************************************
  * Copyright (c) 2020, Hobu Inc.
  * Copyright (c) 2018, Connor Manning
****************************************************************************/

#include <nlohmann/json.h>

#include <pdal/util/FileUtils.h>

#include "Addon.h"
#include "Connector.h"

namespace pdal
{
	point_count_t Addon::points(const Key& key) const
	{
		auto it = m_hierarchy.find(key);
		return (it == m_hierarchy.end() ? 0 : it->m_count);
	}

	std::string Addon::dataDir() const
	{
		return FileUtils::getDirectory(m_filename) + "ept-data/";
	}

	std::string Addon::hierarchyDir() const
	{
		return FileUtils::getDirectory(m_filename) + "ept-hierarchy/";
	}

	AddonList Addon::store(const Connector& connector, const NL::json& spec,
		const PointLayout& layout)
	{
		AddonList addons;
		std::string filename;
		try
		{
			for (auto it : spec.items())
			{
				std::string filename = it.key();
				std::string dimName = it.value().get<std::string>();
				if (!Utils::endsWith(filename, "ept-addon.json"))
					filename += "/ept-addon.json";

				Dimension::Id id = layout.findDim(dimName);
				if (id == Dimension::Id::Unknown)
					throw pdal_error("Invalid dimension '" + dimName + "' in "
						"addon specification. Does not exist in source data.");
				Dimension::Type type = layout.dimType(id);
				std::string typestring = Dimension::toName(Dimension::base(type));
				size_t size = Dimension::size(type);

				Addon addon(dimName, filename, type);

				NL::json meta;
				meta["type"] = typestring;
				meta["size"] = size;
				meta["version"] = "1.0.0";
				meta["dataType"] = "binary";

				connector.makeDir(FileUtils::getDirectory(filename));
				connector.put(filename, meta.dump());

				addons.emplace_back(dimName, filename, type);
				addons.back().setExternalId(id);
			}
		}
		catch (NL::json::parse_error&)
		{
			throw pdal_error("Unable to parse EPT addon file '" + filename + "'.");
		}
		return addons;
	}

	AddonList Addon::load(const Connector& connector, const NL::json& spec)
	{
		AddonList addons;
		std::string filename;
		try
		{
			for (auto it : spec.items())
			{
				std::string dimName = it.key();
				std::string filename = it.value().get<std::string>();
				if (!Utils::endsWith(filename, "ept-addon.json"))
					filename += "/ept-addon.json";

				addons.push_back(loadAddon(connector, dimName, filename));
			}
		}
		catch (NL::json::parse_error&)
		{
			throw pdal_error("Unable to parse EPT addon file '" + filename + "'.");
		}
		return addons;
	}


	Addon Addon::loadAddon(const Connector& connector,
		const std::string& dimName, const std::string& filename)
	{
		NL::json info = connector.getJson(filename);
		std::string typestring = info["type"].get<std::string>();
		uint64_t size = info["size"].get<uint64_t>();
		Dimension::Type type = Dimension::type(typestring, size);

		return Addon(dimName, filename, type);
	}
}
