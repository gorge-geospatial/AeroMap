/******************************************************************************
 * Copyright (c) 2018, Connor Manning (connor@hobu.co)
 ****************************************************************************/

#pragma once

#include <vector>

#include <nlohmann/json.h>

#include "Support.h"

namespace pdal
{
	enum class ComparisonType
	{
		eq,
		gt,
		gte,
		lt,
		lte,
		ne,
		in,
		nin
	};

	inline bool isComparisonType(const std::string& s)
	{
		return s == "$eq" || s == "$gt" || s == "$gte" || s == "$lt" ||
			s == "$lte" || s == "$ne" || s == "$in" || s == "$nin";
	}

	inline ComparisonType toComparisonType(const std::string& s)
	{
		if (s == "$eq")         return ComparisonType::eq;
		else if (s == "$gt")    return ComparisonType::gt;
		else if (s == "$gte")   return ComparisonType::gte;
		else if (s == "$lt")    return ComparisonType::lt;
		else if (s == "$lte")   return ComparisonType::lte;
		else if (s == "$ne")    return ComparisonType::ne;
		else if (s == "$in")    return ComparisonType::in;
		else if (s == "$nin")   return ComparisonType::nin;
		else throw pdal_error("Invalid comparison type: " + s);
	}

	inline std::string typeToString(ComparisonType c)
	{
		switch (c)
		{
		case ComparisonType::eq: return "$eq";
		case ComparisonType::gt: return "$gt";
		case ComparisonType::gte: return "$gte";
		case ComparisonType::lt: return "$lt";
		case ComparisonType::lte: return "$lte";
		case ComparisonType::ne: return "$ne";
		case ComparisonType::in: return "$in";
		case ComparisonType::nin: return "$nin";
		default: throw pdal_error("Invalid comparison type enum");
		}
	}

	inline bool isSingle(ComparisonType co)
	{
		return co != ComparisonType::in && co != ComparisonType::nin;
	}

	inline bool isMultiple(ComparisonType co)
	{
		return !isSingle(co);
	}

	// Represents the RHS portion of a comparison specification.  It may be either
	// a constant or a Dimension name.
	class Operand
	{
	public:
		Operand(const PointLayout& layout, NL::json json)
		{
			if (json.is_string())
			{
				m_id = layout.findDim(json);
				if (m_id == Dimension::Id::Unknown)
				{
					throw pdal_error("Invalid dimension: " +
						json.get<std::string>());
				}
			}
			else if (json.is_number())
			{
				m_value = json;
			}
			else
			{
				throw pdal_error("Invalid comparison operand: " + json.dump(4));
			}
		}

		double get(const pdal::PointRef& pr) const
		{
			if (m_id == Dimension::Id::Unknown)
				return m_value;
			else
				return pr.getFieldAs<double>(m_id);
		}

		std::string toString() const
		{
			if (m_id == Dimension::Id::Unknown)
				return std::to_string(m_value);
			else
				return Dimension::name(m_id);
		}

	private:
		double m_value = 0;
		Dimension::Id m_id = Dimension::Id::Unknown;
	};

	using Operands = std::vector<Operand>;

	class Comparison : public Filterable
	{
	public:
		Comparison(Dimension::Id dimId) : m_dimId(dimId) { }

		static std::unique_ptr<Comparison> create(const PointLayout& layout,
			std::string dimName, const NL::json& json);

	protected:
		virtual ComparisonType type() const = 0;

		const Dimension::Id m_dimId;
	};

	class ComparisonSingle : public Comparison
	{
	public:
		ComparisonSingle(Dimension::Id id, Operand op)
			: Comparison(id)
			, m_operand(op)
		{ }

		virtual bool operator()(const pdal::PointRef& pr) const override
		{
			return compare(pr.getFieldAs<double>(m_dimId), m_operand.get(pr));
		}

		virtual std::string toString(std::string pre) const override
		{
			std::ostringstream ss;
			ss << pre << Dimension::name(m_dimId) << " ";
			ss << pre << typeToString(type()) << " " << m_operand.toString() <<
				std::endl;
			return ss.str();
		}

	protected:
		virtual bool compare(double a, double b) const = 0;

	private:
		Operand m_operand;
	};

	class ComparisonEqual : public ComparisonSingle
	{
	public:
		ComparisonEqual(Dimension::Id id, Operand op)
			: ComparisonSingle(id, op)
		{ }

	protected:
		virtual bool compare(double a, double b) const override { return a == b; }
		virtual ComparisonType type() const override { return ComparisonType::eq; }
	};

	class ComparisonGreater : public ComparisonSingle
	{
	public:
		ComparisonGreater(Dimension::Id id, Operand op)
			: ComparisonSingle(id, op)
		{ }

	protected:
		virtual bool compare(double a, double b) const override { return a > b; }
		virtual ComparisonType type() const override { return ComparisonType::gt; }
	};

	class ComparisonGreaterEqual : public ComparisonSingle
	{
	public:
		ComparisonGreaterEqual(Dimension::Id id, Operand op)
			: ComparisonSingle(id, op)
		{ }

	protected:
		virtual bool compare(double a, double b) const override { return a >= b; }
		virtual ComparisonType type() const override { return ComparisonType::gte; }
	};

	class ComparisonLess : public ComparisonSingle
	{
	public:
		ComparisonLess(Dimension::Id id, Operand op)
			: ComparisonSingle(id, op)
		{ }

	protected:
		virtual bool compare(double a, double b) const override { return a < b; }
		virtual ComparisonType type() const override { return ComparisonType::lt; }
	};

	class ComparisonLessEqual : public ComparisonSingle
	{
	public:
		ComparisonLessEqual(Dimension::Id id, Operand op)
			: ComparisonSingle(id, op)
		{ }

	protected:
		virtual bool compare(double a, double b) const override { return a <= b; }
		virtual ComparisonType type() const override { return ComparisonType::lte; }
	};

	class ComparisonNotEqual : public ComparisonSingle
	{
	public:
		ComparisonNotEqual(Dimension::Id id, Operand op)
			: ComparisonSingle(id, op)
		{ }

	protected:
		virtual bool compare(double a, double b) const override { return a != b; }
		virtual ComparisonType type() const override { return ComparisonType::ne; }
	};

	class ComparisonMulti : public Comparison
	{
	public:
		ComparisonMulti(Dimension::Id id, Operands ops)
			: Comparison(id)
			, m_operands(ops)
		{ }

		virtual std::string toString(std::string pre) const override
		{
			std::ostringstream ss;
			ss << pre << Dimension::name(m_dimId) << " ";
			ss << pre << typeToString(type()) << " ";
			for (const auto& op : m_operands) ss << op.toString() << " ";
			ss << std::endl;
			return ss.str();
		}

	protected:
		const Operands m_operands;
	};

	class ComparisonAny : public ComparisonMulti
	{
	public:
		ComparisonAny(Dimension::Id id, Operands ops)
			: ComparisonMulti(id, ops)
		{ }

	protected:
		virtual ComparisonType type() const override { return ComparisonType::in; }

		virtual bool operator()(const pdal::PointRef& pr) const override
		{
			const double val(pr.getFieldAs<double>(m_dimId));
			return std::any_of(
				m_operands.begin(),
				m_operands.end(),
				[&pr, val](const Operand& op) { return val == op.get(pr); });
		}
	};

	class ComparisonNone : public ComparisonMulti
	{
	public:
		ComparisonNone(Dimension::Id id, Operands ops)
			: ComparisonMulti(id, ops)
		{ }

	protected:
		virtual ComparisonType type() const override { return ComparisonType::nin; }

		virtual bool operator()(const pdal::PointRef& pr) const override
		{
			const double val(pr.getFieldAs<double>(m_dimId));
			return std::none_of(
				m_operands.begin(),
				m_operands.end(),
				[&pr, val](const Operand& op) { return val == op.get(pr); });
		}
	};
}
