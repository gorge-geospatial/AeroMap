/******************************************************************************
* Copyright (c) 2020, Hobu Inc.
****************************************************************************/

#include <string>

#include <ogr_srs_api.h>
#include <ogr_api.h>
#include <cpl_conv.h>

#include "SpatialRef.h"

namespace pdal
{
	namespace gdal
	{
		SpatialRef::SpatialRef()
		{
			newRef(OSRNewSpatialReference(""));
		}

		SpatialRef::SpatialRef(const std::string& srs)
		{
			newRef(OSRNewSpatialReference(""));
			if (OSRSetFromUserInput(get(), srs.data()) != OGRERR_NONE)
				m_ref.reset();
		}

		void SpatialRef::setFromLayer(OGRLayerH layer)
		{
			if (layer)
			{
				OGRSpatialReferenceH s = OGR_L_GetSpatialRef(layer);
				if (s)
				{
					OGRSpatialReferenceH clone = OSRClone(s);
					newRef(clone);
				}
			}
		}

		SpatialRef::operator bool() const
		{
			return m_ref.get() != NULL;
		}

		OGRSpatialReferenceH SpatialRef::get() const
		{
			return m_ref.get();
		}

		std::string SpatialRef::wkt() const
		{
			std::string output;

			if (m_ref.get())
			{
				char* pszWKT = NULL;
				OSRExportToWkt(m_ref.get(), &pszWKT);
				bool valid = (bool)*pszWKT;
				output = pszWKT;
				CPLFree(pszWKT);
			}
			return output;
		}

		bool SpatialRef::empty() const
		{
			return wkt().empty();
		}

		void SpatialRef::newRef(void* v)
		{
			m_ref = RefPtr(v, [](void* t) { OSRDestroySpatialReference(t); });
		}
	}
}
