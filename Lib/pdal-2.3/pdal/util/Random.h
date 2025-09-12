#ifndef RANDOM_H
#define RANDOM_H

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace pdal
{
	namespace Utils
	{
		class Random
		{
		public:
			Random();
			Random(int32_t seed);
			Random(const std::vector<int32_t> seed);
			Random(const std::string& seed);

			std::mt19937& generator();

			static unsigned int quick();

		private:
			std::mt19937 m_generator;
		};
	}
}

#endif // #ifndef RANDOM_H
