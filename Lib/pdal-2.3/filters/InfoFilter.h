#ifndef INFOFILTER_H
#define INFOFILTER_H

#include <pdal/Filter.h>
#include <pdal/Streamable.h>

namespace pdal
{
	class BOX3D;

	// This is just a pass-through filter, which collects some data about
	// the points that are fed through it
	class InfoFilter : public Filter, public Streamable
	{
		struct NearPoint
		{
			NearPoint(PointId id, double dist, std::vector<char>&& data) :
				m_id(id), m_dist(dist), m_data(data)
			{}

			PointId m_id;
			double m_dist;
			std::vector<char> m_data;

			bool operator < (const NearPoint& other) const
			{
				return m_dist < other.m_dist;
			}
		};

	public:
		InfoFilter() :
			m_pointRoot("point"), m_queryCount(10),
			m_queryZ(std::numeric_limits<double>::quiet_NaN())
		{}

		std::string getName() const;
		BOX3D bounds() const
		{
			return m_bounds;
		}

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void initialize(PointTableRef table);
		virtual void ready(PointTableRef table);
		virtual bool processOne(PointRef& point);
		virtual void prepared(PointTableRef table);
		virtual void done(PointTableRef table);
		virtual void filter(PointView& view);

		void parsePointSpec();
		void parseQuerySpec();

		MetadataNode m_pointRoot;

		std::string m_querySpec;
		point_count_t m_queryCount;
		double m_queryX;
		double m_queryY;
		double m_queryZ;
		std::list<NearPoint> m_results;

		std::string m_pointSpec;
		PointIdList m_idList;
		PointIdList::const_iterator m_idCur;
		DimTypeList m_dims;
		size_t m_pointSize;
		PointId m_count;

		BOX3D m_bounds;
	};
}

#endif // #ifndef INFOFILTER_H
