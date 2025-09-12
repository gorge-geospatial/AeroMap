/******************************************************************************
 * Copyright (c) 2018, Connor Manning
 ****************************************************************************/

#include "Connector.h"

#include <pdal/pdal_types.h>

namespace pdal
{
	Connector::Connector() 
		: m_arbiter(new arbiter::Arbiter())
	{
	}

	Connector::Connector(const StringMap& headers, const StringMap& query)
		: m_arbiter(new arbiter::Arbiter)
		, m_headers(headers)
		, m_query(query)
	{
	}

	std::string Connector::get(const std::string& path) const
	{
		if (m_arbiter->isLocal(path))
			return m_arbiter->get(path);
		else
			return m_arbiter->get(path, m_headers, m_query);
	}

	NL::json Connector::getJson(const std::string& path) const
	{
		try
		{
			return NL::json::parse(get(path));
		}
		catch (NL::json::parse_error& err)
		{
			throw pdal_error("File '" + path + "' contained invalid JSON: " +
				err.what());
		}
	}

	std::vector<char> Connector::getBinary(const std::string& path) const
	{
		if (m_arbiter->isLocal(path))
			return m_arbiter->getBinary(path);
		else
			return m_arbiter->getBinary(path, m_headers, m_query);
	}


	arbiter::LocalHandle Connector::getLocalHandle(const std::string& path) const
	{
		if (m_arbiter->isLocal(path))
			return m_arbiter->getLocalHandle(path);
		else
			return m_arbiter->getLocalHandle(path, m_headers, m_query);
	}

	void Connector::put(const std::string& path, const std::vector<char>& buf) const
	{
		if (m_arbiter->isLocal(path))
			return m_arbiter->put(path, buf);
		else
			return m_arbiter->put(path, buf, m_headers, m_query);
	}

	void Connector::put(const std::string& path, const std::string& data) const
	{
		if (m_arbiter->isLocal(path))
			return m_arbiter->put(path, data);
		else
			return m_arbiter->put(path, data, m_headers, m_query);
	}

	void Connector::makeDir(const std::string& path) const
	{
		if (m_arbiter->isLocal(path))
			arbiter::mkdirp(path);
	}
}
