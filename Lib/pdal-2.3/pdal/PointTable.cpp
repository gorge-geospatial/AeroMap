/******************************************************************************
* Copyright (c) 2014, Hobu Inc.
****************************************************************************/

#include <pdal/ArtifactManager.h>
#include <pdal/PointTable.h>

namespace pdal
{
	BasePointTable::BasePointTable(PointLayout& layout)
		: m_metadata(new Metadata())
		, m_layoutRef(layout)
	{
	}

	BasePointTable::~BasePointTable()
	{
	}

	MetadataNode BasePointTable::privateMetadata(const std::string& name)
	{
		MetadataNode mp = m_metadata->m_private;
		MetadataNode m = mp.findChild(name);
		if (!m.valid())
			m = mp.add(name);
		return m;
	}

	void BasePointTable::addSpatialReference(const SpatialReference& spatialRef)
	{
		auto it = std::find(m_spatialRefs.begin(), m_spatialRefs.end(), spatialRef);

		// If not found, add to the beginning.
		if (it == m_spatialRefs.end())
			m_spatialRefs.push_front(spatialRef);
		// If not the first element, move the found element to the front.
		else if (it != m_spatialRefs.begin())
			m_spatialRefs.splice(m_spatialRefs.begin(), m_spatialRefs, it);
	}

	ArtifactManager& BasePointTable::artifactManager()
	{
		if (!m_artifactManager)
			m_artifactManager.reset(new ArtifactManager);

		return *m_artifactManager;
	}

	void SimplePointTable::setFieldInternal(Dimension::Id id, PointId idx, const void* value)
	{
		const Dimension::Detail* d = m_layoutRef.dimDetail(id);
		const char* src = (const char*)value;
		char* dst = getDimension(d, idx);
		std::copy(src, src + d->size(), dst);
	}

	void SimplePointTable::getFieldInternal(Dimension::Id id, PointId idx, void* value) const
	{
		const Dimension::Detail* d = m_layoutRef.dimDetail(id);
		const char* src = getDimension(d, idx);
		char* dst = (char*)value;
		std::copy(src, src + d->size(), dst);
	}

	RowPointTable::~RowPointTable()
	{
		for (auto vi = m_blocks.begin(); vi != m_blocks.end(); ++vi)
			delete[] * vi;
	}

	PointId RowPointTable::addPoint()
	{
		if (m_numPts % m_blockPtCnt == 0)
		{
			size_t size = pointsToBytes(m_blockPtCnt);
			char* buf = new char[size];
			memset(buf, 0, size);
			m_blocks.push_back(buf);
		}
		return m_numPts++;
	}

	char* RowPointTable::getPoint(PointId idx)
	{
		char* buf = m_blocks[idx / m_blockPtCnt];
		return buf + pointsToBytes(idx % m_blockPtCnt);
	}

	MetadataNode BasePointTable::toMetadata() const
	{
		return layout()->toMetadata();
	}
}
