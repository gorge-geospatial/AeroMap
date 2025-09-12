/******************************************************************************
* Copyright (c) 2017, Hobu Inc.
****************************************************************************/

#pragma once

#include <array>
#include <deque>

namespace pdal
{

	class Triangle
	{
	public:
		Triangle(PointId a, PointId b, PointId c) : m_a(a), m_b(b), m_c(c)
		{}

		PointId m_a;
		PointId m_b;
		PointId m_c;

		friend bool operator == (const Triangle& a, const Triangle& b);
	};

	inline bool operator == (const Triangle& a, const Triangle& b)
	{
		std::array<PointId, 3> aa{ {a.m_a, a.m_b, a.m_c} };
		std::array<PointId, 3> bb{ {b.m_a, b.m_b, b.m_c} };
		std::sort(aa.begin(), aa.end());
		std::sort(bb.begin(), bb.end());
		return aa == bb;
	}

	/**
	  A mesh is a way to represent a set of points connected by edges.  Point
	  indices are into a point view.
	*/
	class Mesh
	{};


	/**
	  A mesh where the faces are triangles.
	*/
	class TriangularMesh : public Mesh
	{
	public:
		using const_iterator = std::deque<Triangle>::const_iterator;

		TriangularMesh()
		{}

		size_t size() const
		{
			return m_index.size();
		}
		void add(PointId a, PointId b, PointId c)
		{
			m_index.emplace_back(a, b, c);
		}
		const Triangle& operator[](PointId id) const
		{
			return m_index[id];
		}
		const_iterator begin() const
		{
			return m_index.begin();
		}
		const_iterator end() const
		{
			return m_index.end();
		}

	protected:
		std::deque<Triangle> m_index;
	};
}
