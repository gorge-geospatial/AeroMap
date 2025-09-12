#ifndef EPTADDONWRITER_H
#define EPTADDONWRITER_H

#include <cstddef>
#include <memory>
#include <unordered_set>
#include <vector>

#include <pdal/JsonFwd.h>
#include <pdal/Writer.h>

namespace pdal
{
	class Connector;
	class EptInfo;
	class Key;
	class ThreadPool;
	class Addon;
	struct Overlap;
	using Hierarchy = std::unordered_set<Overlap>;
	using AddonList = std::vector<Addon>;

	class EptAddonWriter : public Writer
	{
	public:
		EptAddonWriter();
		virtual ~EptAddonWriter();

		std::string getName() const override;

	private:
		struct Args;
		std::unique_ptr<Args> m_args;

		virtual void addArgs(ProgramArgs& args) override;
		virtual void addDimensions(PointLayoutPtr layout) override;
		virtual void prepared(PointTableRef table) override;
		virtual void ready(PointTableRef table) override;
		virtual void write(const PointViewPtr view) override;

		void writeOne(const PointViewPtr view, const Addon& addon) const;
		void writeHierarchy(const std::string& hierarchyDir, NL::json& hier,
			const Key& key) const;
		std::string getTypeString(Dimension::Type t) const;

		Dimension::Id m_nodeIdDim = Dimension::Id::Unknown;
		Dimension::Id m_pointIdDim = Dimension::Id::Unknown;

		std::unique_ptr<Connector> m_connector;
		std::unique_ptr<ThreadPool> m_pool;
		std::unique_ptr<EptInfo> m_info;
		std::unique_ptr<Hierarchy> m_hierarchy;
		AddonList m_addons;
		uint64_t m_hierarchyStep = 0;
	};
}

#endif // #ifndef EPTADDONWRITER_H
