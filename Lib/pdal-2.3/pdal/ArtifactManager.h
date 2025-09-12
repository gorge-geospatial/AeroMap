/******************************************************************************
* Copyright (c) 2018, Hobu Inc. (info@hobu.co)
****************************************************************************/

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <pdal/Artifact.h>

namespace pdal
{
	class ArtifactManager
	{
	public:
		ArtifactManager() = default;
		ArtifactManager(const ArtifactManager&) = delete;
		ArtifactManager& operator=(const ArtifactManager&) = delete;

		bool put(const std::string& name, ArtifactPtr artifact)
		{
			return m_storage.insert(std::make_pair(name, artifact)).second;
		}

		template <typename T>
		bool replace(const std::string& name, std::shared_ptr<T> art)
		{
			auto it = m_storage.find(name);
			if (it == m_storage.end())
				return false;

			if (!std::dynamic_pointer_cast<T>(it->second))
				return false;
			it->second = art;
			return true;
		}

		template<typename T>
		bool replaceOrPut(const std::string& name, std::shared_ptr<T> art)
		{
			if (!replace(name, art))
				return put(name, art);
			return true;
		}

		bool erase(const std::string& name)
		{
			return m_storage.erase(name);
		}

		bool exists(const std::string& name)
		{
			return (m_storage.find(name) != m_storage.end());
		}

		std::vector<std::string> keys() const
		{
			std::vector<std::string> ks;
			for (auto e : m_storage)
				ks.push_back(e.first);
			return ks;
		}

		template <typename T>
		std::shared_ptr<T> get(const std::string& name)
		{
			std::shared_ptr<T> art;
			try
			{
				art = std::dynamic_pointer_cast<T>(m_storage.at(name));
			}
			catch (...)
			{
			}
			return art;
		}
	private:
		std::map<std::string, ArtifactPtr> m_storage;
	};
}
