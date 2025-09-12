#ifndef XMLSCHEMA_H
#define XMLSCHEMA_H

#include <pdal/pdal_internal.h>

#define PDAL_XML_SCHEMA_VERSION "1.3"

#include <pdal/Dimension.h>
#include <pdal/Metadata.h>
#include <pdal/PointLayout.h>

#include <string>
#include <stdarg.h>
#include <vector>

#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

namespace pdal
{

	void OCISchemaGenericErrorHandler(void* ctx, const char* message, ...);
	void OCISchemaStructuredErrorHandler(void* userData, xmlErrorPtr error);

	class XMLSchema;

	struct XMLDim
	{
		friend class XMLSchema;

	public:
		XMLDim() : m_min(0.0), m_max(0.0)
		{}
		XMLDim(const DimType& dim, const std::string& name) :
			m_name(name), m_min(0.0), m_max(0.0), m_dimType(dim)
		{}

		std::string m_name;
		std::string m_description;
		uint32_t m_position;
		double m_min;
		double m_max;
		DimType m_dimType;
	};
	typedef std::vector<XMLDim> XMLDimList;
	inline bool operator < (const XMLDim& d1, const XMLDim& d2)
	{
		return d1.m_position < d2.m_position;
	}

	class PDAL_DLL XMLSchema
	{
	public:
		XMLSchema(std::string xml, std::string xsd = "",
			Orientation orientation = Orientation::PointMajor);
		XMLSchema(const XMLDimList& dims, MetadataNode m = MetadataNode(),
			Orientation orientation = Orientation::PointMajor);
		XMLSchema(const PointLayoutPtr& pointTable, MetadataNode m = MetadataNode(),
			Orientation orientation = Orientation::PointMajor);
		XMLSchema() : m_orientation(Orientation::PointMajor)
		{}

		~XMLSchema()
		{
			xmlCleanupParser();
		}

		std::string xml() const;
		DimTypeList dimTypes() const;
		XMLDimList xmlDims() const
		{
			return m_dims;
		}

		MetadataNode getMetadata() const
		{
			return m_metadata;
		}
		void setId(const std::string& name, Dimension::Id id)
		{
			xmlDim(name).m_dimType.m_id = id;
		}
		void setXForm(Dimension::Id id, XForm xform)
		{
			xmlDim(id).m_dimType.m_xform = xform;
		}
		XForm xForm(Dimension::Id id) const
		{
			return xmlDim(id).m_dimType.m_xform;
		}
		void setOrientation(Orientation orientation)
		{
			m_orientation = orientation;
		}
		Orientation orientation() const
		{
			return m_orientation;
		}

	private:
		Orientation m_orientation;
		XMLDimList m_dims;
		void* m_global_context;
		MetadataNode m_metadata;

		XMLDim& xmlDim(Dimension::Id id);
		const XMLDim& xmlDim(Dimension::Id id) const;
		XMLDim& xmlDim(const std::string& name);
		xmlDocPtr init(const std::string& xml, const std::string& xsd);
		bool validate(xmlDocPtr doc, const std::string& xsd);
		std::string remapOldNames(const std::string& input);
		bool loadMetadata(xmlNode* startNode, MetadataNode& input);
		bool load(xmlDocPtr doc);
		void writeXml(xmlTextWriterPtr w) const;
	};

} // namespace pdal


#endif // #ifndef XMLSCHEMA_H
