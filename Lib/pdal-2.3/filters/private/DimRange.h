#ifndef DIMRANGE_H
#define DIMRANGE_H

#include <string>

#include <pdal/Dimension.h>
#include <pdal/PointRef.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	struct DimRange
	{
		struct error : public std::runtime_error
		{
			error(const std::string& err) : std::runtime_error(err)
			{}
		};

		DimRange(const std::string name,
			double lower_bound,
			double upper_bound,
			bool inclusive_lower_bound,
			bool inclusive_upper_bound,
			bool negate) :
			m_name(name), m_id(Dimension::Id::Unknown),
			m_lower_bound(lower_bound), m_upper_bound(upper_bound),
			m_inclusive_lower_bound(inclusive_lower_bound),
			m_inclusive_upper_bound(inclusive_upper_bound),
			m_negate(negate)
		{}

		DimRange() : m_id(Dimension::Id::Unknown), m_lower_bound(0),
			m_upper_bound(0), m_inclusive_lower_bound(true),
			m_inclusive_upper_bound(true), m_negate(false)
		{}

		void parse(const std::string& s);
		bool valuePasses(double d) const;
		static bool pointPasses(const std::vector<DimRange>& ranges,
			PointRef& point);

		std::string m_name;
		Dimension::Id m_id;
		double m_lower_bound;
		double m_upper_bound;
		bool m_inclusive_lower_bound;
		bool m_inclusive_upper_bound;
		bool m_negate;

	protected:
		std::string::size_type subParse(const std::string& r);
	};

	bool operator < (const DimRange& r1, const DimRange& r2);
	std::istream& operator>>(std::istream& in, DimRange& r);
	std::ostream& operator<<(std::ostream& out, const DimRange& r);

	template <>
	class VArg<DimRange> : public BaseVArg
	{
	public:
		VArg(const std::string& longname, const std::string& shortname,
			const std::string& description, std::vector<DimRange>& variable,
			std::vector<DimRange> def) :
			BaseVArg(longname, shortname, description), m_var(variable),
			m_defaultVal(def)
		{
			m_var = def;
			m_defaultProvided = true;
		}

		VArg(const std::string& longname, const std::string& shortname,
			const std::string& description, std::vector<DimRange>& variable) :
			BaseVArg(longname, shortname, description), m_var(variable)
		{}

		virtual void setValue(const std::string& s)
		{
			std::vector<std::string> slist = Utils::split2(s, ',');
			for (auto& ts : slist)
				Utils::trim(ts);

			if (slist.empty())
				throw arg_val_error("Missing value for argument '" + m_longname + "'.");
			m_rawVal = s;
			if (!m_set)
				m_var.clear();
			for (auto& ts : slist)
			{
				DimRange dim;
				dim.parse(ts);
				m_var.push_back(dim);
			}
			m_set = true;
		}

		virtual void reset()
		{
			m_var = m_defaultVal;
			m_set = false;
			m_hidden = false;
		}

		virtual std::string defaultVal() const
		{
			std::string s;

			for (size_t i = 0; i < m_defaultVal.size(); ++i)
			{
				std::ostringstream oss;

				if (i > 0)
					s += ", ";
				oss << m_defaultVal[i];
				s += oss.str();
			}
			return s;
		}

	private:
		std::vector<DimRange>& m_var;
		std::vector<DimRange> m_defaultVal;
	};
}

#endif // #ifndef DIMRANGE_H
