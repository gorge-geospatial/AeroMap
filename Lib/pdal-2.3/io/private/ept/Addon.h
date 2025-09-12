#ifndef ADDON_H
#define ADDON_H

#include <list>
#include <string>

#include <pdal/JsonFwd.h>
#include <pdal/PointLayout.h>

#include "Overlap.h"

namespace pdal
{
	class Addon;
	class Connector;
	using AddonList = std::vector<Addon>;

	class Addon
	{
	public:
		Addon(const std::string& dimName, const std::string& filename,
			Dimension::Type type) :
			m_name(dimName), m_filename(filename), m_type(type)
		{
			m_localId = m_layout.registerOrAssignDim(dimName, type);
		}

		std::string name() const
		{
			return m_name;
		}
		std::string filename() const
		{
			return m_filename;
		}
		Dimension::Type type() const
		{
			return m_type;
		}
		// Id for the local (internal) layout
		Dimension::Id localId() const
		{
			return m_localId;
		}
		// Id for the layout to which we'll copy data (ultimate PDAL ID).
		Dimension::Id externalId() const
		{
			return m_externalId;
		}
		void setExternalId(Dimension::Id externalId)
		{
			m_externalId = externalId;
		}
		Hierarchy& hierarchy()
		{
			return m_hierarchy;
		}
		PointLayout& layout() const
		{
			return const_cast<PointLayout&>(m_layout);
		}
		point_count_t points(const Key& key) const;
		std::string dataDir() const;
		std::string hierarchyDir() const;
		static AddonList store(const Connector& connector, const NL::json& spec,
			const PointLayout& layout);
		static AddonList load(const Connector& connector, const NL::json& spec);

	private:
		std::string m_name;
		std::string m_filename;
		Dimension::Type m_type;
		Dimension::Id m_localId;
		Dimension::Id m_externalId;

		Hierarchy m_hierarchy;
		PointLayout m_layout;

		static Addon loadAddon(const Connector& connector, const std::string& dimName, const std::string& filename);
	};
}

#endif // #ifndef ADDON_H
