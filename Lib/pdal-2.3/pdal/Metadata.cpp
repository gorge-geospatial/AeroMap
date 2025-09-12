/******************************************************************************
 * Copyright (c) 2012, Howard Butler (howard@hobu.co)
 ****************************************************************************/

#include <pdal/Metadata.h>
#include <pdal/SpatialReference.h>
#include <pdal/util/Bounds.h>

namespace pdal
{
	template <>
	void MetadataNodeImpl::setValue(const SpatialReference& ref)
	{
		m_type = "spatialreference";
		m_value = Utils::toString(ref);
	}

	std::string Metadata::inferType(const std::string& val)
	{
		size_t pos;

		long l = 0;
		try
		{
			pos = 0;
			l = std::stol(val, &pos);
		}
		catch (std::invalid_argument&)
		{
		}
		if (pos == val.length())
			return (l < 0 ? "nonNegativeInteger" : "integer");

		try
		{
			pos = 0;
			std::stod(val, &pos);
		}
		catch (std::invalid_argument&)
		{
		}

		if (pos == val.length())
			return "double";

		BOX2D b2d;
		std::istringstream iss1(val);
		iss1 >> b2d;
		if (iss1.good())
			return "bounds";

		BOX3D b3d;
		std::istringstream iss2(val);
		iss2 >> b3d;
		if (iss2.good())
			return "bounds";

		if (val == "true" || val == "false")
			return "boolean";

		try
		{
			SpatialReference s(val);
			return "spatialreference";
		}
		catch (pdal_error&)
		{
		}

		Uuid uuid(val);
		if (!uuid.isNull())
			return "uuid";

		return "string";
	}
}
