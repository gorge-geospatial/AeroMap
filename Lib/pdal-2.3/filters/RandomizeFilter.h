#ifndef RANDOMIZEFILTER_H
#define RANDOMIZEFILTER_H

#include <pdal/Filter.h>

#include <random>

namespace pdal
{
	class RandomizeFilter : public Filter
	{
	public:
		RandomizeFilter()
		{
		}

		std::string getName() const;

	private:
		Arg* m_seedArg;
		unsigned m_seed;

		virtual void addArgs(ProgramArgs& args);
		virtual void filter(PointView& view)
		{
			if (!m_seedArg->set())
			{
				std::random_device rng;
				m_seed = rng();
			}
			std::mt19937 mt(m_seed);
			std::shuffle(view.begin(), view.end(), mt);
		}

		RandomizeFilter& operator=(const RandomizeFilter&); // not implemented
		RandomizeFilter(const RandomizeFilter&);            // not implemented
	};
}

#endif // #ifndef RANDOMIZEFILTER_H
