#ifndef TILECONTENTS_H
#define TILECONTENTS_H

#include <pdal/PointView.h>
#include <pdal/pdal_types.h>

#include "Addon.h"
#include "Overlap.h"

namespace pdal
{
	class EptInfo;
	class Connector;
	class BasePointTable;
	using BasePointTablePtr = std::unique_ptr<BasePointTable>;

	class TileContents
	{
	public:
		TileContents(const Overlap& overlap, const EptInfo& info,
			const Connector& connector, const AddonList& addons) :
			m_overlap(overlap), m_info(info), m_connector(connector),
			m_addons(addons)
		{}

		BasePointTable& table() const
		{
			return *m_table;
		}
		const Key& key() const
		{
			return m_overlap.m_key;
		}
		point_count_t nodeId() const
		{
			return m_overlap.m_nodeId;
		}
		//ABELL - This is bad. We're assuming that the actual number of points we have matches
		// what our index information told us. This may not be the case because of some
		// issue. Downstream handling may depend on this being the actual number of points
		// in the tile, rather than the number that were *supposed to be* in the tile.
		point_count_t size() const
		{
			return m_overlap.m_count;
		}
		const std::string& error() const
		{
			return m_error;
		}
		BasePointTable* addonTable(Dimension::Id id) const
		{
			return const_cast<TileContents*>(this)->m_addonTables[id].get();
		}
		void read();

	private:
		Overlap m_overlap;
		const EptInfo& m_info;
		const Connector& m_connector;
		const AddonList& m_addons;
		std::string m_error;
		// Table for the base point data.
		BasePointTablePtr m_table;
		// Tables for the add on data.
		std::map<Dimension::Id, BasePointTablePtr> m_addonTables;

		void readLaszip();
		void readBinary();
		void readZstandard();
		void readAddon(const Addon& addon);
		void transform();
	};
}

#endif // #ifndef TILECONTENTS_H
