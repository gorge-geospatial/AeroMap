/******************************************************************************
* Copyright (c) 2014,  Hobu Inc., hobu@hobu.co
****************************************************************************/

#include <pdal/DbReader.h>
#include <pdal/PDALUtils.hpp>

namespace pdal
{
	void DbReader::loadSchema(PointLayoutPtr layout, const std::string& schemaString)
	{
		XMLSchema schema(schemaString);
		loadSchema(layout, schema);
	}

	void DbReader::loadSchema(PointLayoutPtr layout, const XMLSchema& schema)
	{
		m_layout = layout;
		m_dims = schema.xmlDims();

		// Always register X, Y and Z.  We will apply any scaling set
		// before handing it off to PDAL.
		using namespace Dimension;
		layout->registerDims({ Id::X, Id::Y, Id::Z });

		m_orientation = schema.orientation();
		m_packedPointSize = 0;
		for (auto di = m_dims.begin(); di != m_dims.end(); ++di)
		{
			di->m_dimType.m_id =
				layout->registerOrAssignDim(di->m_name, di->m_dimType.m_type);
			m_packedPointSize += Dimension::size(di->m_dimType.m_type);
		}
	}

	// If we start reading from a DB block with a different schema, reflect that
	// in the dimensions and size.
	void DbReader::updateSchema(const XMLSchema& schema)
	{
		m_dims = schema.xmlDims();
		m_orientation = schema.orientation();
		m_packedPointSize = 0;
		for (auto di = m_dims.begin(); di != m_dims.end(); ++di)
		{
			di->m_dimType.m_id = m_layout->findDim(di->m_name);
			m_packedPointSize += Dimension::size(di->m_dimType.m_type);
		}
	}

	DimTypeList DbReader::dbDimTypes() const
	{
		DimTypeList dimTypes;

		for (auto di = m_dims.begin(); di != m_dims.end(); ++di)
			dimTypes.push_back(di->m_dimType);
		return dimTypes;
	}

	size_t DbReader::dimOffset(Dimension::Id id) const
	{
		size_t offset = 0;
		for (auto di = m_dims.begin(); di != m_dims.end(); ++di)
		{
			if (di->m_dimType.m_id == id)
				break;
			offset += Dimension::size(di->m_dimType.m_type);
		}
		return offset;
	}

	void DbReader::writeField(PointRef& point, const char* pos, const DimType& dim)
	{
		using namespace Dimension;

		if (dim.m_id == Id::X || dim.m_id == Id::Y || dim.m_id == Id::Z)
		{
			Everything e;

			memcpy(&e, pos, Dimension::size(dim.m_type));
			double d = Utils::toDouble(e, dim.m_type);
			d = (d * dim.m_xform.m_scale.m_val) + dim.m_xform.m_offset.m_val;
			point.setField(dim.m_id, d);
		}
		else
			point.setField(dim.m_id, dim.m_type, pos);
	}

	/// Write a point's packed data into a buffer.
	/// \param[in] point PointRef to write to.
	/// \param[in] buf  Pointer to packed DB point data.
	void DbReader::writePoint(PointRef& point, const char* buf)
	{
		for (auto di = m_dims.begin(); di != m_dims.end(); ++di)
		{
			writeField(point, buf, di->m_dimType);
			buf += Dimension::size(di->m_dimType.m_type);
		}
	}
}
