/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
****************************************************************************/

#include "GeotiffSupport.h"

#include <sstream>

#include <geo_normalize.h>
#include <geo_simpletags.h>

namespace pdal
{
	// Utility functor with accompanying to print GeoTIFF directory.
	struct geotiff_dir_printer
	{
		geotiff_dir_printer() {}

		std::string output() const { return m_oss.str(); }
		std::string::size_type size() const { return m_oss.str().size(); }

		void operator()(char* data, void* /*aux*/)
		{

			if (0 != data)
			{
				m_oss << data;
			}
		}

	private:
		std::stringstream m_oss;
	};
}

PDAL_C_START

// These functions are available from GDAL, but they
// aren't exported.
char* GTIFGetOGISDefn(GTIF*, GTIFDefn*);
int GTIFSetFromOGISDefn(GTIF*, const char*);
void VSIFree(void* data);

int PDALGeoTIFFPrint(char* data, void* aux)
{
	pdal::geotiff_dir_printer* printer = reinterpret_cast<pdal::geotiff_dir_printer*>(aux);
	(*printer)(data, 0);
	return static_cast<int>(printer->size());
}

PDAL_C_END

#include <io/LasVLR.h>

namespace pdal
{
	namespace
	{

		struct GeotiffCtx
		{
		public:
			GeotiffCtx() : gtiff(nullptr)
			{
				tiff = ST_Create();
			}

			~GeotiffCtx()
			{
				if (gtiff)
					GTIFFree(gtiff);
				ST_Destroy(tiff);
			}

			ST_TIFF* tiff;
			GTIF* gtiff;
		};

	}

#pragma pack(push)
#pragma pack(1)
	struct Entry
	{
		uint16_t key;
		uint16_t location;
		uint16_t count;
		uint16_t offset;
	};
#pragma pack(pop)

	GeotiffSrs::GeotiffSrs(const std::vector<uint8_t>& directoryRec,
		const std::vector<uint8_t>& doublesRec,
		const std::vector<uint8_t>& asciiRec, LogPtr log) : m_log(log)
	{
		GeotiffCtx ctx;

		if (directoryRec.empty())
			return;

		// Make sure struct is 16 bytes.
#pragma pack(push)
#pragma pack(1)
		struct ShortKeyHeader
		{
			uint16_t dirVersion;
			uint16_t keyRev;
			uint16_t minorRev;
			uint16_t numKeys;
		};
#pragma pack(pop)

		const ShortKeyHeader* header = (const ShortKeyHeader*)directoryRec.data();
		size_t declaredSize = (header->numKeys + 1) * 4;
		if (directoryRec.size() < declaredSize)
			return;

		validateDirectory((const Entry*)(header + 1), header->numKeys,
			doublesRec.size() / sizeof(double), asciiRec.size());

		uint8_t* dirData = const_cast<uint8_t*>(directoryRec.data());
		ST_SetKey(ctx.tiff, GEOTIFF_DIRECTORY_RECORD_ID,
			(1 + header->numKeys) * 4, STT_SHORT, (void*)dirData);

		if (doublesRec.size())
		{
			uint8_t* doubleData = const_cast<uint8_t*>(doublesRec.data());
			ST_SetKey(ctx.tiff, GEOTIFF_DOUBLES_RECORD_ID,
				(int)(doublesRec.size() / sizeof(double)), STT_DOUBLE,
				(void*)doubleData);
		}

		if (asciiRec.size())
		{
			uint8_t* asciiData = const_cast<uint8_t*>(asciiRec.data());
			ST_SetKey(ctx.tiff, GEOTIFF_ASCII_RECORD_ID,
				(int)asciiRec.size(), STT_ASCII, (void*)asciiData);
		}

		ctx.gtiff = GTIFNewSimpleTags(ctx.tiff);
		if (!ctx.gtiff)
			throw Geotiff::error("Couldn't create Geotiff tags from "
				"Geotiff definition.");

		GTIFDefn sGTIFDefn;
		if (GTIFGetDefn(ctx.gtiff, &sGTIFDefn))
		{
			char* wkt = GTIFGetOGISDefn(ctx.gtiff, &sGTIFDefn);
			if (wkt)
			{
				m_srs.set(wkt);
				VSIFree(wkt);
			}
		}

		geotiff_dir_printer geotiff_printer;
		GTIFPrint(ctx.gtiff, PDALGeoTIFFPrint, &geotiff_printer);

		m_gtiff_print_string = geotiff_printer.output();

	}


	void GeotiffSrs::validateDirectory(const Entry* ent, size_t numEntries,
		size_t numDoubles, size_t asciiSize)
	{
		for (size_t i = 0; i < numEntries; ++i)
		{
			if (ent->count == 0)
				m_log->get(LogLevel::Warning) << "Geotiff directory contains " <<
				"key " << ent->key << " with 0 count." << std::endl;
			if (ent->location == 0 && ent->count != 1)
				m_log->get(LogLevel::Error) << "Geotiff directory contains key " <<
				ent->key << " with short entry and more than one value." <<
				std::endl;
			if (ent->location == GEOTIFF_DIRECTORY_RECORD_ID)
				if (ent->offset + ent->count > numDoubles)
					m_log->get(LogLevel::Error) << "Geotiff directory contains " <<
					"key " << ent->key << " with count/offset outside of valid "
					"range of doubles record." << std::endl;
			if (ent->location == GEOTIFF_ASCII_RECORD_ID)
				if (ent->offset + ent->count > asciiSize)
					m_log->get(LogLevel::Error) << "Geotiff directory contains " <<
					" key " << ent->key << " with count/offset outside of "
					"valid range of ascii record." << std::endl;
			ent++;
		}
	}


	GeotiffTags::GeotiffTags(const SpatialReference& srs)
	{
		if (srs.empty())
			return;

		GeotiffCtx ctx;
		ctx.gtiff = GTIFNewSimpleTags(ctx.tiff);

		// Set tiff tags from WKT
		if (!GTIFSetFromOGISDefn(ctx.gtiff, srs.getWKT().c_str()))
			throw Geotiff::error("Could not set m_gtiff from WKT");
		GTIFWriteKeys(ctx.gtiff);

		auto sizeFromType = [](int type, int count) -> size_t
		{
			if (type == STT_ASCII)
				return count;
			else if (type == STT_SHORT)
				return 2 * count;
			else if (type == STT_DOUBLE)
				return 8 * count;
			return 8 * count;
		};

		int count;
		int st_type;
		uint8_t* data;
		if (ST_GetKey(ctx.tiff, GEOTIFF_DIRECTORY_RECORD_ID,
			&count, &st_type, (void**)&data))
		{
			size_t size = sizeFromType(st_type, count);
			m_directoryRec.resize(size);
			std::copy(data, data + size, m_directoryRec.begin());
		}
		if (ST_GetKey(ctx.tiff, GEOTIFF_DOUBLES_RECORD_ID,
			&count, &st_type, (void**)&data))
		{
			size_t size = sizeFromType(st_type, count);
			m_doublesRec.resize(size);
			std::copy(data, data + size, m_doublesRec.begin());
		}
		if (ST_GetKey(ctx.tiff, GEOTIFF_ASCII_RECORD_ID,
			&count, &st_type, (void**)&data))
		{
			size_t size = sizeFromType(st_type, count);
			m_asciiRec.resize(size);
			std::copy(data, data + size, m_asciiRec.begin());
		}
	}
}
