/******************************************************************************
 * Copyright (c) 2015, Hobu Inc.
 ****************************************************************************/

#include "LasUtils.h"

#include <pdal/util/Extractor.h>
#include <pdal/util/Inserter.h>
#include <pdal/util/Utils.h>

#include <string>

namespace pdal
{

	namespace
	{
		using DT = Dimension::Type;
		const Dimension::Type lastypes[] = {
			DT::None, DT::Unsigned8, DT::Signed8, DT::Unsigned16, DT::Signed16,
			DT::Unsigned32, DT::Signed32, DT::Unsigned64, DT::Signed64,
			DT::Float, DT::Double
		};
	}

	uint8_t ExtraBytesIf::lasType()
	{
		uint8_t lastype = 0;

		for (size_t i = 0; i < sizeof(lastypes) / sizeof(lastypes[0]); ++i)
			if (m_type == lastypes[i])
			{
				lastype = (uint8_t)i;
				break;
			}
		if (m_fieldCnt == 0 || lastype == 0)
			return 0;
		return 10 * (m_fieldCnt - 1) + lastype;
	}


	void ExtraBytesIf::setType(uint8_t lastype)
	{
		m_fieldCnt = 1;
		while (lastype > 10)
		{
			m_fieldCnt++;
			lastype -= 10;
		}

		m_type = lastypes[lastype];
		if (m_type == Dimension::Type::None)
			m_fieldCnt = 0;
	}

	void ExtraBytesIf::appendTo(std::vector<uint8_t>& ebBytes)
	{
		size_t offset = ebBytes.size();
		ebBytes.resize(ebBytes.size() + sizeof(ExtraBytesSpec));
		LeInserter inserter(ebBytes.data() + offset, sizeof(ExtraBytesSpec));

		uint8_t lastype = lasType();
		uint8_t options = (uint8_t)(lastype ? 0 : m_size);

		inserter << (uint16_t)0 << lastype << options;
		inserter.put(m_name, 32);
		inserter << (uint32_t)0;			// Reserved.
		for (size_t i = 0; i < 3; ++i)
			inserter << (uint64_t)0;		// No data field.
		for (size_t i = 0; i < 3; ++i)
			inserter << (double)0.0;		// Min.
		for (size_t i = 0; i < 3; ++i)
			inserter << (double)0.0;		// Max.
		for (size_t i = 0; i < 3; ++i)
			inserter << m_scale[i];
		for (size_t i = 0; i < 3; ++i)
			inserter << m_offset[i];
		inserter.put(m_description, 32);
	}

	void ExtraBytesIf::readFrom(const char* buf)
	{
		LeExtractor extractor(buf, sizeof(ExtraBytesSpec));
		uint16_t dummy16;
		uint32_t dummy32;
		uint64_t dummy64;
		double dummyd;
		uint8_t options;
		uint8_t type;

		uint8_t SCALE_MASK = 1 << 3;
		uint8_t OFFSET_MASK = 1 << 4;

		extractor >> dummy16 >> type >> options;
		extractor.get(m_name, 32);
		extractor >> dummy32;
		for (size_t i = 0; i < 3; ++i)
			extractor >> dummy64;  // No data field.
		for (size_t i = 0; i < 3; ++i)
			extractor >> dummyd;  // Min.
		for (size_t i = 0; i < 3; ++i)
			extractor >> dummyd;  // Max.
		for (size_t i = 0; i < 3; ++i)
			extractor >> m_scale[i];
		for (size_t i = 0; i < 3; ++i)
			extractor >> m_offset[i];
		extractor.get(m_description, 32);

		setType(type);
		if (m_type == Dimension::Type::None)
			m_size = options;
		if (!(options & SCALE_MASK))
			for (size_t i = 0; i < 3; ++i)
				m_scale[i] = 1.0;
		if (!(options & OFFSET_MASK))
			for (size_t i = 0; i < 3; ++i)
				m_offset[i] = 0.0;
	}

	std::vector<ExtraDim> ExtraBytesIf::toExtraDims()
	{
		std::vector<ExtraDim> eds;

		if (m_type == Dimension::Type::None)
		{
			ExtraDim ed(m_name, (uint8_t)m_size);
			eds.push_back(ed);
		}
		else if (m_fieldCnt == 1)
		{
			ExtraDim ed(m_name, m_type, m_scale[0], m_offset[0]);
			eds.push_back(ed);
		}
		else
		{
			for (size_t i = 0; i < m_fieldCnt; ++i)
			{
				ExtraDim ed(m_name + std::to_string(i), m_type, m_scale[i], m_offset[i]);
				eds.push_back(ed);
			}
		}
		return eds;
	}

	namespace LasUtils
	{
		std::vector<IgnoreVLR> parseIgnoreVLRs(const StringList& ignored)
		{
			std::vector<IgnoreVLR> ignoredVLRs;
			for (auto& v : ignored)
			{

				StringList s = Utils::split2(v, '/');
				if (s.size() == 2)
				{
					Utils::trim(s[0]);
					Utils::trim(s[1]);
					int i = std::stoi(s[1]);
					uint16_t id = (uint16_t)i;
					IgnoreVLR v;
					v.m_userId = s[0];
					v.m_recordId = id;
					ignoredVLRs.push_back(v);
				}
				else if (s.size() == 1)
				{
					Utils::trim(s[0]);
					IgnoreVLR v;
					v.m_userId = s[0];
					v.m_recordId = 0;
					ignoredVLRs.push_back(v);
				}
				else
				{
					throw error("Invalid VLR user_id/record_id specified");
				}
			}
			return ignoredVLRs;

		}
		std::vector<ExtraDim> parse(const StringList& dimString, bool allOk)
		{
			std::vector<ExtraDim> extraDims;
			bool all = false;

			for (auto& dim : dimString)
			{
				if (dim == "all")
				{
					// We only accept all for LasWriter.
					if (!allOk)
						throw error("Invalid extra dimension specified: '" + dim +
							"'.  Need <dimension>=<type>.  See documentation "
							" for details.");
					all = true;
					continue;
				}

				StringList s = Utils::split2(dim, '=');
				if (s.size() != 2)
					throw error("Invalid extra dimension specified: '" + dim +
						"'.  Need <dimension>=<type>.  See documentation "
						" for details.");
				Utils::trim(s[0]);
				Utils::trim(s[1]);
				Dimension::Type type = Dimension::type(s[1]);
				if (type == Dimension::Type::None)
					throw error("Invalid extra dimension type specified: '" + dim +
						"'.  Need <dimension>=<type>.  See documentation "
						" for details.");
				ExtraDim ed(s[0], type);
				extraDims.push_back(ed);
			}

			if (all)
			{
				if (extraDims.size())
					throw error("Can't specify specific extra dimensions with "
						"special 'all' keyword.");
				extraDims.push_back(ExtraDim("all", Dimension::Type::None));
			}

			return extraDims;
		}
	}
}
