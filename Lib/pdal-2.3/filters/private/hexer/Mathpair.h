/******************************************************************************
 * Copyright (c) 2014, Hobu Inc. (howard@hobu.co)
 ****************************************************************************/

#ifndef MATHPAIR_H
#define MATHPAIR_H

namespace hexer
{
	template <typename T>
	struct Mathpair
	{
	public:
		Mathpair() : m_x(T(0)), m_y(T(0))
		{}

		Mathpair(T x, T y) : m_x(x), m_y(y)
		{}

		T m_x;
		T m_y;

		void operator -= (const Mathpair& p)
		{
			m_x -= p.m_x;
			m_y -= p.m_y;
		}

		Mathpair& operator += (const Mathpair& p)
		{
			m_x += p.m_x;
			m_y += p.m_y;
			return *this;
		}

		friend Mathpair operator - (Mathpair p1, const Mathpair& p2)
		{
			p1 -= p2;
			return p1;
		}

		friend Mathpair operator + (Mathpair p1, const Mathpair& p2)
		{
			p1 += p2;
			return p1;
		}
	};

	typedef Mathpair<double> Point;
	typedef Mathpair<int> Coord;
}

#endif // #ifndef MATHPAIR_H
