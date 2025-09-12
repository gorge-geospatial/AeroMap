#ifndef EPTREADER_H
#define EPTREADER_H

#include <map>
#include <string>
#include <memory>
#include <unordered_set>

#include <pdal/JsonFwd.h>
#include <pdal/Reader.h>
#include <pdal/Streamable.h>
#include <pdal/util/Bounds.h>

namespace pdal
{
	class Connector;
	class EptInfo;
	class Key;
	class TileContents;
	struct Overlap;
	using Hierarchy = std::unordered_set<Overlap>;
	using StringMap = std::map<std::string, std::string>;

	class EptReader : public Reader, public Streamable
	{
		FRIEND_TEST(EptReaderTest, getRemoteType);
		FRIEND_TEST(EptReaderTest, getCoercedType);

	public:
		EptReader();
		virtual ~EptReader();
		std::string getName() const override;

	private:
		virtual void addArgs(ProgramArgs& args) override;
		virtual void initialize() override;
		virtual QuickInfo inspect() override;
		virtual void addDimensions(PointLayoutPtr layout) override;
		virtual void ready(PointTableRef table) override;
		virtual point_count_t read(PointViewPtr view, point_count_t count) override;
		virtual bool processOne(PointRef& point) override;

		// If argument "origin" is specified, this function will clip the query
		// bounds to the bounds of the specified origin and set m_queryOriginId to
		// the selected OriginId value.  If the selected origin is not found, throw.
		void handleOriginQuery();
		void setForwards(StringMap& headers, StringMap& query);

		// Aggregate all EPT keys overlapping our query bounds and their number of
		// points from a walk through the hierarchy.  Each of these keys will be
		// downloaded during the 'read' section.
		void overlaps();
		void overlaps(Hierarchy& target, const NL::json& current, const Key& key);
		bool hasSpatialFilter() const;
		bool passesSpatialFilter(const BOX3D& tileBounds) const;
		void process(PointViewPtr dstView, const TileContents& tile, point_count_t count);
		bool processPoint(PointRef& dst, const TileContents& tile);
		void load(const Overlap& overlap);
		void checkTile(const TileContents& tile);

		struct Args;
		std::unique_ptr<Args> m_args;
		struct Private;
		std::unique_ptr<Private> m_p;

		uint64_t m_tileCount;
		int64_t m_queryOriginId = -1;

		uint64_t m_depthEnd = 0;    // Zero indicates selection of all depths.
		uint64_t m_hierarchyStep = 0;

		Dimension::Id m_nodeIdDim = Dimension::Id::Unknown;
		Dimension::Id m_pointIdDim = Dimension::Id::Unknown;

		ArtifactManager* m_artifactMgr;
		PointId m_pointId = 0;
		uint64_t m_nodeId;
	};
}

#endif // #ifndef EPTREADER_H
