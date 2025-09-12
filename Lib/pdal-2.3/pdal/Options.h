#ifndef OPTIONS_H
#define OPTIONS_H

#include <pdal/pdal_internal.h>
#include <pdal/Metadata.h>
#include <pdal/util/Utils.h>

#include <map>
#include <memory>
#include <vector>

namespace pdal
{
	class Options;
	class Option;

	class Option
	{
		friend std::ostream& operator<<(std::ostream& ostr, const Option&);
	public:

		/// @name Constructors

		Option()
		{}

		/// Primary constructor
		template <typename T>
		Option(std::string const& name, const T& value) 
			: m_name(name)
		{
			m_value = Utils::toString(value);
		}

		Option(std::string const& name, const std::string& value) 
			: m_name(name)
			, m_value(value)
		{
		}

		Option(std::string const& name, const double& value) 
			: m_name(name)
		{
			m_value = Utils::toString(value, 15);
		}

		Option(std::string const& name, const bool& value)
			: m_name(name)
		{
			m_value = value ? "true" : "false";
		}

		/// @return the name for the Option instance
		std::string const& getName() const
		{
			return m_name;
		}

		static std::string::size_type parse(const std::string& name, std::string::size_type p)
		{
			std::string::size_type count = 0;

			if (std::islower(name[p++]))
			{
				count++;

				auto isname = [](char c)
				{ return (std::islower(c) || std::isdigit(c) || c == '_'); };
				count += Utils::extract(name, p, isname);
			}
			return count;
		}

		std::string toArg() const;

		// Make sure that the option name consists of lowercase characters or
		// underscores.
		static bool nameValid(const std::string& name, bool reportError);

		/// @return the value of the Option.
		std::string getValue() const
		{
			return m_value;
		}

		bool empty() const;

		void toMetadata(MetadataNode& parent) const;

		/// @name Private attributes
	private:
		std::string m_name;
		std::string m_value;
	};

	class Options
	{
		friend std::ostream&
			operator<<(std::ostream& ostr, const Options&);
	public:
		Options()
		{}

		explicit Options(const Option& opt)
		{
			add(opt);
		}

		void add(const Option& option);
		void add(const Options& options);
		void addConditional(const Option& option);
		void addConditional(const Options& option);

		// if option name not present, just returns
		void remove(const Option& option);

		void replace(const Option& option)
		{
			remove(option);
			add(option);
		}

		void toMetadata(MetadataNode& parent) const
		{
			for (std::string& k : getKeys())
			{
				StringList l = getValues(k);
				std::string vs;
				for (auto vi = l.begin(); vi != l.end(); ++vi)
				{
					if (vi != l.begin())
						vs += ", ";
					vs += *vi;
				}

				// 'userData' keys on stages and such are JSON
				if (!Utils::iequals(k, "user_data"))
					parent.add(k, vs);
				else
					parent.addWithType(k, vs, "json", "User JSON");
			}
		}

		// add an option (shortcut version, bypass need for an Option object)
		template<typename T> void add(const std::string& name, T value)
		{
			Option opt(name, value);
			add(opt);
		}

		void add(const std::string& name, const std::string& value)
		{
			Option opt(name, value);
			add(opt);
		}

		void add(const std::string& name, const bool& value)
		{
			Option opt(name, value);
			add(opt);
		}

		template<typename T> void replace(const std::string& name, T value)
		{
			Option opt(name, value);
			replace(opt);
		}

		void replace(const std::string& name, const std::string& value)
		{
			Option opt(name, value);
			replace(opt);
		}

		void replace(const std::string& name, const bool& value)
		{
			Option opt(name, value);
			replace(opt);
		}

		StringList getValues(const std::string& name) const
		{
			StringList s;

			auto ops = getOptions(name);
			for (Option& op : ops)
				s.push_back(op.getValue());
			return s;
		}

		StringList getKeys() const
		{
			StringList keys;

			for (auto it = m_options.begin(); it != m_options.end();
				it = m_options.upper_bound(it->first))
			{
				keys.push_back(it->first);
			}
			return keys;
		}

		std::vector<Option> getOptions(std::string const& name = "") const;
		StringList toCommandLine() const;
		static Options fromFile(const std::string& filename, bool throwOnOpenError = true);

	private:
		std::multimap<std::string, Option> m_options;

		static Options fromJsonFile(const std::string& filename, const std::string& s);
		static Options fromCmdlineFile(const std::string& filename, const std::string& s);
	};
	typedef std::map<std::string, Options> OptionsMap;
}

#endif // #ifndef OPTIONS_H
