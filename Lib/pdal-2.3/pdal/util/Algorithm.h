/******************************************************************************
* Copyright (c) 2014, Hobu Inc., hobu.inc@gmail.com
****************************************************************************/

#pragma once

#include <algorithm>
#include <map>
#include <vector>

namespace pdal
{
	namespace Utils
	{
		/**
		  Determine if a container contains a value.

		  \param cont  Container.
		  \param val  Value.
		  \return \c true if the value is in the container, \c false otherwise.
		*/
		template<typename CONTAINER, typename VALUE>
		bool contains(const CONTAINER& cont, const VALUE& val)
		{
			return std::find(cont.begin(), cont.end(), val) != cont.end();
		}

		/**
		  Determine if a map contains a key.

		  \param c  Map.
		  \param v  Key value.
		  \return \c true if the value is in the container, \c false otherwise.
		*/
		template<typename KEY, typename VALUE>
		bool contains(const std::map<KEY, VALUE>& c, const KEY& v)
		{
			return c.find(v) != c.end();
		}

		/**
		  Remove all instances of a value from a container.

		  \param cont  Container.
		  \param v  Value to remove.
		*/
		template<typename CONTAINER, typename VALUE>
		void remove(CONTAINER& cont, const VALUE& val)
		{
			cont.erase(std::remove(cont.begin(), cont.end(), val), cont.end());
		}


		/**
		  Remove all instances matching a unary predicate from a container.

		  \param cont  Container.
		  \param p  Predicate indicating whether a value should be removed.
		*/
		template<typename CONTAINER, typename PREDICATE>
		void remove_if(CONTAINER& cont, PREDICATE p)
		{
			cont.erase(std::remove_if(cont.begin(), cont.end(), p), cont.end());
		}
	}
}
