/******************************************************************************
* Copyright (c) 2016, Hobu Inc., info@hobu.co
****************************************************************************/

#include <pdal/PDALUtils.h>
#include <pdal/util/Algorithm.h>

#include "PtsReader.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"readers.pts",
		"Pts Reader",
		"http://pdal.io/stages/readers.pts.html",
		{ "pts" }
	};

	CREATE_STATIC_STAGE(PtsReader, s_info)

	std::string PtsReader::getName() const { return s_info.name; }

	void PtsReader::initialize(PointTableRef table)
	{
		m_istream = Utils::openFile(m_filename);
		if (!m_istream)
			throwError("Unable to open file '" + m_filename + "'.");

		std::string buf;
		std::getline(*m_istream, buf);

		// Very first line is point count
		if (!Utils::fromString(buf, m_PointCount))
		{
			throwError("Unable to read expected point count at top of the file '" + m_filename + "'.");
		}

		// Peek second line to determine dimensions to add.
		// Expect points in the formats:
		//      X Y Z,
		//      X Y Z Intensity,
		//      X Y Z Intensity R G B
		std::getline(*m_istream, buf);
		StringList fields = Utils::split2(buf, m_separator);
		switch (fields.size())
		{
		case 3:
			m_dims.push_back(Dimension::Id::X);
			m_dims.push_back(Dimension::Id::Y);
			m_dims.push_back(Dimension::Id::Z);
			break;
		case 4:
			m_dims.push_back(Dimension::Id::X);
			m_dims.push_back(Dimension::Id::Y);
			m_dims.push_back(Dimension::Id::Z);
			m_dims.push_back(Dimension::Id::Intensity);
			break;
		case 7:
			m_dims.push_back(Dimension::Id::X);
			m_dims.push_back(Dimension::Id::Y);
			m_dims.push_back(Dimension::Id::Z);
			m_dims.push_back(Dimension::Id::Intensity);
			m_dims.push_back(Dimension::Id::Red);
			m_dims.push_back(Dimension::Id::Green);
			m_dims.push_back(Dimension::Id::Blue);
			break;
		default:
			throwError("Invalid number of fields for the first point in file '" + m_filename + "'.");
		}

		Utils::closeFile(m_istream);
	}


	void PtsReader::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDims(m_dims);
	}


	void PtsReader::ready(PointTableRef table)
	{
		m_istream = Utils::openFile(m_filename);
		if (!m_istream)
			throwError("Unable to open file '" + m_filename + "'.");

		// Skip header line.
		std::string buf;
		std::getline(*m_istream, buf);
	}


	point_count_t PtsReader::read(PointViewPtr view, point_count_t numPts)
	{
		PointId idx = view->size();

		point_count_t cnt = 0;
		size_t line = 1;

		// Continue reading while count less than max points and count less than 
		// the expected point count.
		while (m_istream->good() && cnt < numPts && cnt < m_PointCount)
		{
			std::string buf;
			StringList fields;

			std::getline(*m_istream, buf);
			line++;
			if (buf.empty())
				continue;

			fields = Utils::split2(buf, m_separator);
			if (fields.size() != m_dims.size())
			{
				log()->get(LogLevel::Error) << "Line " << line <<
					" in '" << m_filename << "' contains " << fields.size() <<
					" fields when " << m_dims.size() << " were expected.  "
					"Ignoring." << std::endl;
				continue;
			}

			double d;
			for (size_t i = 0; i < fields.size(); ++i)
			{
				if (!Utils::fromString(fields[i], d))
				{
					log()->get(LogLevel::Error) << "Can't convert "
						"field '" << fields[i] << "' to numeric value on line " <<
						line << " in '" << m_filename << "'.  Setting to 0." <<
						std::endl;
					d = 0;
				}
				if (i == 3) // Intensity field in PTS is -2048 to 2047, we map to 0 4095
				{
					d += 2048;
				}
				view->setField(m_dims[i], idx, d);
			}
			cnt++;
			idx++;
		}

		if (cnt < m_PointCount)
		{
			log()->get(LogLevel::Warning) << "Expected " << m_PointCount
				<< " points but only " << cnt << " were found." << std::endl;
		}

		return cnt;
	}


	void PtsReader::done(PointTableRef table)
	{
		Utils::closeFile(m_istream);
	}

}
