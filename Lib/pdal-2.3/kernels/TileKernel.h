#ifndef TILEKERNEL_H
#define TILEKERNEL_H

#include <map>

#include <pdal/Kernel.h>
#include <filters/SplitterFilter.h>

namespace pdal
{
	class TileKernel : public Kernel
	{
		using Coord = std::pair<int, int>;
		using Readers = std::map<std::string, Streamable*>;

	public:
		TileKernel();
		std::string getName() const override;
		int execute() override;

	private:
		void addSwitches(ProgramArgs& args);
		void validateSwitches(ProgramArgs& args);
		Streamable* prepareReader(const std::string& filename);
		void process(const Readers& readers);
		void checkReaders(const Readers& readers);
		void adder(PointRef& point, int xpos, int ypos);

		std::string m_inputFile;
		std::string m_outputFile;
		double m_length;
		double m_xOrigin;
		double m_yOrigin;
		double m_buffer;
		std::map<Coord, Streamable*> m_writers;
		FixedPointTable m_table;
		SplitterFilter m_splitter;
		Streamable* m_repro;
		SpatialReference m_outSrs;
		std::string::size_type m_hashPos;
	};
}

#endif // #ifndef TILEKERNEL_H
