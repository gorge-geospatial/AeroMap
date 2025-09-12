#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <arbiter/arbiter.h>

namespace pdal
{
	using StringMap = std::map<std::string, std::string>;

	class Connector
	{
		std::unique_ptr<arbiter::Arbiter> m_arbiter;
		StringMap m_headers;
		StringMap m_query;

	public:
		Connector();
		Connector(const StringMap& headers, const StringMap& query);

		std::string get(const std::string& path) const;
		NL::json getJson(const std::string& path) const;
		std::vector<char> getBinary(const std::string& path) const;
		arbiter::LocalHandle getLocalHandle(const std::string& path) const;
		void put(const std::string& path, const std::vector<char>& data) const;
		void put(const std::string& path, const std::string& data) const;
		void makeDir(const std::string& path) const;
	};
}

#endif // #ifndef CONNECTOR_H
