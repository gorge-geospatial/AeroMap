/******************************************************************************
 * Copyright (c) 2018, Connor Manning (connor@hobu.co)
 ****************************************************************************/

#pragma once

#include <string>

#include <pdal/PointLayout.h>
#include <pdal/PointRef.h>

namespace pdal
{
	class Loggable
	{
	public:
		virtual ~Loggable() { }

		virtual std::string toString(std::string prefix) const = 0;
	};

	class Filterable : public Loggable
	{
	public:
		virtual bool operator()(const PointRef& pr) const = 0;
	};

	class Comparable : public Loggable
	{
	public:
		virtual bool operator()(double v) const = 0;
	};

	template<typename T, typename... Args>
	std::unique_ptr<T> makeUnique(Args&&... args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}
