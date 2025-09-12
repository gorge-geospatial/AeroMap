/******************************************************************************
* Copyright (c) 2020, Hobu Inc.
****************************************************************************/

#pragma once

#include <memory>

namespace pdal
{
	namespace gdal
	{
		class SpatialRef
		{
			using OGRLayerH = void*;
			typedef std::shared_ptr<void> RefPtr;
		public:
			SpatialRef();
			SpatialRef(const std::string& srs);

			void setFromLayer(OGRLayerH layer);
			operator bool() const;
			OGRSpatialReferenceH get() const;
			std::string wkt() const;
			bool empty() const;

		private:
			void newRef(void* v);

			RefPtr m_ref;
		};
	}
}
