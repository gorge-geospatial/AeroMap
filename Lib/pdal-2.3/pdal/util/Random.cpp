/******************************************************************************
* Copyright (c) 2021, Hobu Inc. (info@hobu.co)
****************************************************************************/

#include "Random.h"

namespace pdal
{
	namespace Utils
	{
		Random::Random()
		{
			std::vector<int32_t> seed;
			std::random_device rd;
			for (size_t i = 0; i < std::mt19937::state_size; ++i)
				seed.push_back(rd());
			std::seed_seq seedSeq(seed.begin(), seed.end());
			m_generator.seed(seedSeq);
		}

		Random::Random(int32_t seed)
		{
			std::seed_seq seedSeq{ seed };
			m_generator.seed(seedSeq);
		}

		Random::Random(const std::vector<int32_t> seed)
		{
			std::seed_seq seedSeq(seed.begin(), seed.end());
			m_generator.seed(seedSeq);
		}

		Random::Random(const std::string& seed)
		{
			std::vector<int32_t> v;
			int32_t s = 0;
			int i = 0;
			for (char c : seed)
			{
				s |= c << (8 * i++);
				if (i == 4)
				{
					v.push_back(s);
					i = 0;
					s = 0;
				}
			}
			if (i)
				v.push_back(s);
			std::seed_seq seedSeq(v.begin(), v.end());
			m_generator.seed(seedSeq);
		}

		std::mt19937& Random::generator()
		{
			return m_generator;
		}

		unsigned int Random::quick()
		{
			std::random_device rd;

			return rd();
		}
	}
}
