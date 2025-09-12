#ifndef ILVIS2METADATAREADER_H
#define ILVIS2METADATAREADER_H

#include <pdal/Metadata.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string>
#include <iostream>
#include <stdlib.h>

namespace pdal
{
	class Ilvis2MetadataReader
	{
	public:
		struct error : public std::runtime_error
		{
			error(const std::string& s) : std::runtime_error(s)
			{}
		};

		void readMetadataFile(std::string filename, MetadataNode* m);

	protected:
		// These methods are written to parse specific nodes.  It doesn't
		// do full validation, but does check to make sure things are in
		// the order it expects them to be in.

		void parseGranuleMetaDataFile(xmlNodePtr node, MetadataNode* m);
		void parseGranuleURMetaData(xmlNodePtr node, MetadataNode* m);
		void parseCollectionMetaData(xmlNodePtr node, MetadataNode* m);
		void parseDataFiles(xmlNodePtr node, MetadataNode* m);
		void parseDataFileContainer(xmlNodePtr node, MetadataNode* m);
		void parseECSDataGranule(xmlNodePtr node, MetadataNode* m);
		void parseRangeDateTime(xmlNodePtr node, MetadataNode* m);

		void parsePlatform(xmlNodePtr node, MetadataNode* m);
		void parseInstrument(xmlNodePtr node, MetadataNode* m);
		void parseSensor(xmlNodePtr node, MetadataNode* m);
		void parseSensorCharacteristic(xmlNodePtr node, MetadataNode* m);
		void parseCampaign(xmlNodePtr node, MetadataNode* m);
		void parsePSAs(xmlNodePtr node, MetadataNode* m);
		void parsePSA(xmlNodePtr node, MetadataNode* m);
		void parseXXProduct(std::string type, xmlNodePtr node, MetadataNode* m);

		void parseSpatialDomainContainer(xmlNodePtr node, MetadataNode* m);
		void parseGPolygon(xmlNodePtr node, MetadataNode* m);
		void parseBoundary(xmlNodePtr node, MetadataNode* m);
		void parsePoint(xmlNodePtr node, MetadataNode* m);

	private:
		// These private methods are mostly helper functions for proessing
		// the heirarchy and contents of the various XML node objects that
		// are returned by libxml.

		std::string extractString(xmlNodePtr node);
		double extractDouble(xmlNodePtr node);
		int extractInt(xmlNodePtr node);
		long extractLong(xmlNodePtr node);

		// These two methods are useful to help ignore "empty" text nodes
		// caused by indentation, etc.  These will simply grab the actual
		// element nodes directly.
		// Note that due to the way LIBXML parses things, a child points
		// to its own siblings; the parent only points to the first child.
		xmlNodePtr getNextElementNode(xmlNodePtr node);
		xmlNodePtr getFirstChildElementNode(xmlNodePtr node);

		bool nodeElementIs(xmlNodePtr node, std::string expected);
		void assertElementIs(xmlNodePtr node, std::string expected);
		void assertEndOfElements(xmlNodePtr node);
		int countChildElements(xmlNodePtr node, std::string childName);
	};
}

#endif // #ifndef ILVIS2METADATAREADER_H
