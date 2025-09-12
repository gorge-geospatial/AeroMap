/******************************************************************************
* Copyright (c) 2016, Howard Butler (howard@hobu.co)
****************************************************************************/
#pragma once

#include <pdal/Log.h>
#include <pdal/PointRef.h>
#include <pdal/SpatialReference.h>

#include <memory>

class OGRGeometry;
using OGRGeometryH = void*;
using OGRSpatialReferenceH = void*;

namespace pdal
{
	class BOX3D;

	class Geometry
	{
	protected:
		Geometry();
		Geometry(const Geometry&);
		Geometry(Geometry&&);
		Geometry(const std::string& wkt_or_json,
			SpatialReference ref = SpatialReference());
		Geometry(OGRGeometryH g);
		Geometry(OGRGeometryH g, const SpatialReference& srs);

	public:
		Geometry& operator=(const Geometry&);
		virtual ~Geometry();

		OGRGeometryH getOGRHandle()
		{
			return m_geom.get();
		}

		virtual void update(const std::string& wkt_or_json);
		virtual bool valid() const;
		virtual void clear() = 0;
		virtual void modified();
		bool srsValid() const;
		void setSpatialReference(const SpatialReference& ref);
		SpatialReference getSpatialReference() const;
		Utils::StatusWithReason transform(SpatialReference ref);

		std::string wkt(double precision = 15, bool bOutputZ = false) const;
		std::string json(double precision = 15) const;

		BOX3D bounds() const;

		operator bool() const
		{
			return m_geom != NULL;
		}
		static void throwNoGeos();

	protected:
		std::unique_ptr<OGRGeometry> m_geom;

		friend std::ostream& operator<<(std::ostream& ostr, const Geometry& p);
		friend std::istream& operator>>(std::istream& istr,	Geometry& p);
	};
}
