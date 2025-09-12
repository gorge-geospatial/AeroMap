/******************************************************************************
 * Copyright (c) 2020, Hobu Inc.
 ****************************************************************************/

#include <io/LasReader.h>
#include <pdal/compression/ZstdCompression.h>

#include "Connector.h"
#include "EptInfo.h"
#include "TileContents.h"
#include "VectorPointTable.h"

namespace pdal
{
	void TileContents::read()
	{
		try
		{
			if (m_info.dataType() == EptInfo::DataType::Laszip)
				readLaszip();
			else if (m_info.dataType() == EptInfo::DataType::Binary)
				readBinary();
#ifdef PDAL_HAVE_ZSTD
			else if (m_info.dataType() == EptInfo::DataType::Zstandard)
				readZstandard();
#endif
			else
				throw pdal_error("Unrecognized EPT dataType");
			//ABELL - Should check that we read the number of points specified in the
			//  overlap.
					// Read addon information after the native data, we'll possibly
					// overwrite attributes.
			for (const Addon& addon : m_addons)
				readAddon(addon);
		}
		catch (const std::exception& ex)
		{
			m_error = ex.what();
		}
		catch (...)
		{
			m_error = "Unknown exception when reading tile contents";
		}
	}

	void TileContents::readLaszip()
	{
		// If the file is remote (HTTP, S3, Dropbox, etc.), getLocalHandle will
		// download the file and `localPath` will return the location of the
		// downloaded file in a temporary directory.  Otherwise it's a no-op.
		std::string filename = m_info.dataDir() + key().toString() + ".laz";
		auto handle = m_connector.getLocalHandle(filename);

		m_table.reset(new ColumnPointTable);

		Options options;
		options.add("filename", handle.localPath());
		options.add("use_eb_vlr", true);
		options.add("nosrs", true);

		LasReader reader;
		reader.setOptions(options);

		reader.prepare(*m_table);
		reader.execute(*m_table);
	}

	void TileContents::readBinary()
	{
		std::string filename = m_info.dataDir() + key().toString() + ".bin";
		auto data(m_connector.getBinary(filename));

		VectorPointTable* vpt = new VectorPointTable(m_info.remoteLayout());
		vpt->buffer() = std::move(data);
		m_table.reset(vpt);

		transform();
	}

#ifdef PDAL_HAVE_ZSTD
	void TileContents::readZstandard()
	{
		std::string filename = m_info.dataDir() + key().toString() + ".zst";
		auto compressed(m_connector.getBinary(filename));
		std::vector<char> data;
		pdal::ZstdDecompressor dec([&data](char* pos, std::size_t size)
		{
			data.insert(data.end(), pos, pos + size);
		});

		dec.decompress(compressed.data(), compressed.size());

		VectorPointTable* vpt = new VectorPointTable(m_info.remoteLayout());
		vpt->buffer() = std::move(data);
		m_table.reset(vpt);

		transform();
	}
#else
	void TileContents::readZstandard()
	{}
#endif // PDAL_HAVE_ZSTD

	void TileContents::readAddon(const Addon& addon)
	{
		m_addonTables[addon.localId()] = nullptr;

		point_count_t addonPoints = addon.points(key());
		if (addonPoints == 0)
			return;

		// If the addon hierarchy exists, it must match the EPT data.
		if (addonPoints != size())
			throw pdal_error("Invalid addon hierarchy");

		std::string filename = addon.dataDir() + key().toString() + ".bin";
		const auto data(m_connector.getBinary(filename));

		if (size() * Dimension::size(addon.type()) != data.size())
			throw pdal_error("Invalid addon content length");

		VectorPointTable* vpt = new VectorPointTable(addon.layout());
		vpt->buffer() = std::move(data);
		m_addonTables[addon.localId()] = BasePointTablePtr(vpt);
	}

	void TileContents::transform()
	{
		using D = Dimension::Id;

		// Shorten long name.
		const XForm& xf = m_info.dimType(Dimension::Id::X).m_xform;
		const XForm& yf = m_info.dimType(Dimension::Id::Y).m_xform;
		const XForm& zf = m_info.dimType(Dimension::Id::Z).m_xform;

		PointRef p(*m_table);
		for (PointId i = 0; i < size(); ++i)
		{
			p.setPointId(i);

			// Scale the XYZ values.
			p.setField(D::X, p.getFieldAs<double>(D::X) * xf.m_scale.m_val +
				xf.m_offset.m_val);
			p.setField(D::Y, p.getFieldAs<double>(D::Y) * yf.m_scale.m_val +
				yf.m_offset.m_val);
			p.setField(D::Z, p.getFieldAs<double>(D::Z) * zf.m_scale.m_val +
				zf.m_offset.m_val);
		}
	}
}
