#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <string>
#include <vector>

#include <functional>

#include "HexerPath.h"

namespace hexer
{
	typedef std::function<bool(double&, double&, void*&)> PointReader;
	typedef std::function<bool(int&, int&, void*&)> HexReader;

	void process(HexGrid* grid, PointReader);
	void processHexes(HexGrid* grid, HexReader);
	double computeHexSize(const std::vector<Point>& samples, int density);
}

#endif // #ifndef PROCESSOR_H
