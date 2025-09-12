/******************************************************************************
* Copyright (c) 2014, Peter J. Gadomski (pete.gadomski@gmail.com)
****************************************************************************/

#include "SbetReader.h"
#include "SbetCommon.h"

#include <pdal/PointRef.h>
#include <pdal/util/FileUtils.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace pdal
{

	static StaticPluginInfo const s_info
	{
		"readers.sbet",
		"SBET Reader",
		"http://pdal.io/stages/readers.sbet.html",
		{ "sbet" }
	};

	CREATE_STATIC_STAGE(SbetReader, s_info)

	std::string SbetReader::getName() const { return s_info.name; }

	void SbetReader::addArgs(ProgramArgs& args)
	{
		args.add("angles_as_degrees", "Convert all angles to degrees", m_anglesAsDegrees, true);
	}

	void SbetReader::addDimensions(PointLayoutPtr layout)
	{
		layout->registerDims(sbet::fileDimensions());
	}

	void SbetReader::ready(PointTableRef)
	{
		size_t fileSize = FileUtils::fileSize(m_filename);
		size_t pointSize = sbet::fileDimensions().size() * sizeof(double);
		if ((fileSize == 0) || (fileSize % pointSize != 0))
			throwError("Invalid file size.");
		m_numPts = fileSize / pointSize;
		m_index = 0;
		m_stream.reset(new ILeStream(m_filename));
		m_dims = sbet::fileDimensions();
		seek(m_index);
	}

	bool SbetReader::processOne(PointRef& point)
	{
		auto radiansToDegrees = [](double radians) {
			return radians * 180.0 / M_PI;
		};
		for (auto di = m_dims.begin(); di != m_dims.end(); ++di)
		{
			double d;
			*m_stream >> d;
			Dimension::Id dim = *di;
			if (m_anglesAsDegrees && sbet::isAngularDimension(dim)) {
				d = radiansToDegrees(d);
			}
			point.setField(dim, d);
		}
		return (m_stream->good());
	}

	point_count_t SbetReader::read(PointViewPtr view, point_count_t count)
	{
		PointId nextId = view->size();
		PointId idx = m_index;
		point_count_t numRead = 0;
		seek(idx);
		while (numRead < count && idx < m_numPts)
		{
			PointRef point = view->point(nextId);
			processOne(point);
			if (m_cb)
				m_cb(*view, nextId);

			idx++;
			nextId++;
			numRead++;
		}
		m_index = idx;
		return numRead;
	}

	bool SbetReader::eof()
	{
		return m_index >= m_numPts;
	}

	void SbetReader::seek(PointId idx)
	{
		m_stream->seek(idx * sizeof(double) * sbet::fileDimensions().size());
	}
}
