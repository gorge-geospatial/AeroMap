/******************************************************************************
 * Copyright (c) 2014, Hobu Inc., hobu@hobu.co
 ****************************************************************************/

#pragma once

#include <vector>

#include <pdal/util/Algorithm.h>
#include <pdal/Log.h>

namespace pdal
{
	class LasError
	{
	public:
		LasError() : m_filename("<unknown>")
		{}

		void setFilename(const std::string& filename)
		{
			m_filename = filename;
		}

		void setLog(LogPtr log) { m_log = log; }

		void returnNumWarning(int returnNum)
		{
			static std::vector<int> warned;

			if (!Utils::contains(warned, returnNum))
			{
				warned.push_back(returnNum);
				m_log->get(LogLevel::Warning) << m_filename <<
					": Found invalid value of '" << returnNum <<
					"' for point's return number.\n";
			}
		}

		void numReturnsWarning(int numReturns)
		{
			static std::vector<int> warned;

			if (!Utils::contains(warned, numReturns))
			{
				warned.push_back(numReturns);
				m_log->get(LogLevel::Warning) << m_filename << ": Found invalid value "
					"of '" << numReturns << "' for point's number of returns.\n";
			}
		}

	private:
		std::string m_filename;
		LogPtr m_log;
	};
}
