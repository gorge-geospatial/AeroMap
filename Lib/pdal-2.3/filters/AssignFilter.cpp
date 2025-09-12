/******************************************************************************
* Copyright (c) 2017, Hobu Inc., info@hobu.co
****************************************************************************/

#include "AssignFilter.h"

#include <pdal/StageFactory.h>
#include <pdal/util/ProgramArgs.h>

#include "private/DimRange.h"
#include "private/expr/AssignStatement.h"

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.assign",
		"Assign values for a dimension range to a specified value.",
		"http://pdal.io/stages/filters.assign.html"
	};

	CREATE_STATIC_STAGE(AssignFilter, s_info)

	struct AssignRange : public DimRange
	{
		void parse(const std::string& r);
		double m_value;
	};

	struct AssignArgs
	{
		std::vector<AssignRange> m_assignments;
		DimRange m_condition;
		std::vector<expr::AssignStatement> m_statements;
	};

	void AssignRange::parse(const std::string& r)
	{
		std::string::size_type pos, count;
		const char* start;
		char* end;

		pos = subParse(r);
		count = Utils::extractSpaces(r, pos);
		pos += count;

		if (r[pos] != '=')
			throw error("Missing '=' assignment separator.");
		pos++;

		count = Utils::extractSpaces(r, pos);
		pos += count;

		// Extract value
		start = r.data() + pos;
		m_value = std::strtod(start, &end);
		if (start == end)
			throw error("Missing value to assign following '='.");
		pos += (end - start);

		if (pos != r.size())
			throw error("Invalid characters following valid range.");
	}

	std::istream& operator>>(std::istream& in, AssignRange& r)
	{
		std::string s;

		std::getline(in, s);
		try
		{
			r.parse(s);
		}
		catch (DimRange::error&)
		{
			in.setstate(std::ios_base::failbit);
		}
		return in;
	}

	std::ostream& operator<<(std::ostream& out, const AssignRange& r)
	{
		out << (const DimRange&)r;
		out << "=" << r.m_name;
		return out;
	}

	AssignFilter::AssignFilter() : m_args(new AssignArgs)
	{}


	AssignFilter::~AssignFilter()
	{}

	void AssignFilter::addArgs(ProgramArgs& args)
	{
		args.add("assignment", "Values to assign to dimensions based on range.", m_args->m_assignments);
		args.add("condition", "Condition for assignment based on range.", m_args->m_condition);
		args.add("value", "Value to assign to dimension based on expression.", m_args->m_statements);
	}

	void AssignFilter::prepared(PointTableRef table)
	{
		PointLayoutPtr layout(table.layout());

		m_args->m_condition.m_id = layout->findDim(m_args->m_condition.m_name);
		for (auto& r : m_args->m_assignments)
		{
			r.m_id = layout->findDim(r.m_name);
			if (r.m_id == Dimension::Id::Unknown)
				throwError("Invalid dimension name in 'assignment' option: '" +
					r.m_name + "'.");
		}
		for (expr::AssignStatement& expr : m_args->m_statements)
		{
			auto status = expr.prepare(layout);
			if (!status)
				throwError(status.what());
		}
	}

	bool AssignFilter::processOne(PointRef& point)
	{
		if (m_args->m_condition.m_id != Dimension::Id::Unknown)
		{
			double condVal = point.getFieldAs<double>(m_args->m_condition.m_id);
			if (!m_args->m_condition.valuePasses(condVal))
				return true;
		}
		for (AssignRange& r : m_args->m_assignments)
			if (r.valuePasses(point.getFieldAs<double>(r.m_id)))
				point.setField(r.m_id, r.m_value);
		for (expr::AssignStatement& expr : m_args->m_statements)
			if (expr.conditionalExpr().eval(point))
				point.setField(expr.identExpr().eval(), expr.valueExpr().eval(point));

		return true;
	}

	void AssignFilter::filter(PointView& view)
	{
		PointRef point(view, 0);
		for (PointId id = 0; id < view.size(); ++id)
		{
			point.setPointId(id);
			processOne(point);
		}
	}
}
