/******************************************************************************
* Copyright (c) 2016, Hobu Inc. (info@hobu.co)
****************************************************************************/

#include <pdal/Scaling.h>

namespace pdal
{
	void Scaling::addArgs(ProgramArgs& args)
	{
		m_xOffArg = &args.add("offset_x", "X offset", m_xXform.m_offset);
		m_yOffArg = &args.add("offset_y", "Y offset", m_yXform.m_offset);
		m_zOffArg = &args.add("offset_z", "Z offset", m_zXform.m_offset);

		m_xScaleArg = &args.add("scale_x", "X scale", m_xXform.m_scale, XForm::XFormComponent(1.0));
		m_yScaleArg = &args.add("scale_y", "Y scale", m_yXform.m_scale, XForm::XFormComponent(1.0));
		m_zScaleArg = &args.add("scale_z", "Z scale", m_zXform.m_scale, XForm::XFormComponent(1.0));
	}

	void Scaling::setAutoXForm(const PointViewSet& pvSet)
	{
		bool xmod = m_xXform.m_offset.m_auto || m_xXform.m_scale.m_auto;
		bool ymod = m_yXform.m_offset.m_auto || m_yXform.m_scale.m_auto;
		bool zmod = m_zXform.m_offset.m_auto || m_zXform.m_scale.m_auto;
		if (!xmod && !ymod && !zmod)
			return;

		double xmin = (std::numeric_limits<double>::max)();
		double xmax = (std::numeric_limits<double>::lowest)();
		double ymin = (std::numeric_limits<double>::max)();
		double ymax = (std::numeric_limits<double>::lowest)();
		double zmin = (std::numeric_limits<double>::max)();
		double zmax = (std::numeric_limits<double>::lowest)();

		for (const PointViewPtr& view : pvSet)
		{
			if (xmod)
				for (PointId idx = 0; idx < view->size(); idx++)
				{
					double x = view->getFieldAs<double>(Dimension::Id::X, idx);
					xmin = (std::min)(x, xmin);
					xmax = (std::max)(x, xmax);
				}
			if (ymod)
				for (PointId idx = 0; idx < view->size(); idx++)
				{
					double y = view->getFieldAs<double>(Dimension::Id::Y, idx);
					ymin = (std::min)(y, ymin);
					ymax = (std::max)(y, ymax);
				}
			if (zmod)
				for (PointId idx = 0; idx < view->size(); idx++)
				{
					double z = view->getFieldAs<double>(Dimension::Id::Z, idx);
					zmin = (std::min)(z, zmin);
					zmax = (std::max)(z, zmax);
				}
		}

		if (m_xXform.m_offset.m_auto)
			m_xXform.m_offset.m_val = 0.5 * xmin + 0.5 * xmax;
		if (m_yXform.m_offset.m_auto)
			m_yXform.m_offset.m_val = 0.5 * ymin + 0.5 * ymax;
		if (m_zXform.m_offset.m_auto)
			m_zXform.m_offset.m_val = 0.5 * zmin + 0.5 * zmax;

		if (m_xXform.m_scale.m_auto)
		{
			xmax = (std::max)(std::abs(xmax - m_xXform.m_offset.m_val),
				std::abs(xmin - m_xXform.m_offset.m_val));
			m_xXform.m_scale.m_val
				= xmax ? xmax / (std::numeric_limits<int>::max)() : 1.;
		}
		if (m_yXform.m_scale.m_auto)
		{
			ymax = (std::max)(std::abs(ymax - m_yXform.m_offset.m_val),
				std::abs(ymin - m_yXform.m_offset.m_val));
			m_yXform.m_scale.m_val
				= ymax ? ymax / (std::numeric_limits<int>::max)() : 1.;
		}
		if (m_zXform.m_scale.m_auto)
		{
			zmax = (std::max)(std::abs(zmax - m_zXform.m_offset.m_val),
				std::abs(zmin - m_zXform.m_offset.m_val));
			m_zXform.m_scale.m_val
				= zmax ? zmax / (std::numeric_limits<int>::max)() : 1.;
		}
	}
}
