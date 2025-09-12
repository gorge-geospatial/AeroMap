#ifndef FAUXREADER_H
#define FAUXREADER_H

#include <random>

#include <pdal/Reader.h>
#include <pdal/Streamable.h>

namespace pdal
{
	enum class Mode
	{
		Constant,
		Ramp,
		Uniform,
		Normal,
		Grid
	};

	inline std::istream& operator>>(std::istream& in, Mode& m)
	{
		std::string s;

		in >> s;
		s = Utils::tolower(s);
		if (s == "constant")
			m = Mode::Constant;
		else if (s == "random" || s == "uniform")
			m = Mode::Uniform;
		else if (s == "ramp")
			m = Mode::Ramp;
		else if (s == "normal")
			m = Mode::Normal;
		else if (s == "grid")
			m = Mode::Grid;
		else
			in.setstate(std::ios::failbit);
		return in;
	}

	inline std::ostream& operator<<(std::ostream& out, const Mode& m)
	{
		switch (m)
		{
		case Mode::Constant:
			out << "Constant";
		case Mode::Ramp:
			out << "Ramp";
		case Mode::Uniform:
			out << "Uniform";
		case Mode::Normal:
			out << "Normal";
		case Mode::Grid:
			out << "Grid";
		}
		return out;
	}

	// The FauxReader doesn't read from disk, but instead just makes up data for its
	// points.  The reader is constructed with a given bounding box and a given
	// number of points.
	//
	// This reader knows about these fields (Dimensions):
	//    X,Y,Z - floats
	//    Time  - uint64
	//    ReturnNumber (optional) - uint8
	//    NumberOfReturns (optional) - uint8
	//
	// It supports a few modes:
	//   - "random" generates points that are randomly distributed within the
	//     given bounding box
	//   - "constant" generates its points to always be at the minimum of the
	//      bounding box
	//   - "ramp" generates its points as a linear ramp from the minimum of the
	//     bbox to the maximum
	//   - "uniform" generates points that are uniformly distributed within the
	//     given bounding box
	//   - "normal" generates points that are normally distributed with a given
	//     mean and standard deviation in each of the XYZ dimensions
	// In all these modes, however, the Time field is always set to the point
	// number.
	//
	// ReturnNumber and NumberOfReturns are not included by default, but can be
	// activated by passing a numeric value as "number_of_returns" to the
	// reader constructor.
	//
	class FauxReader : public Reader, public Streamable
	{
	public:
		FauxReader()
		{}

		std::string getName() const;

	private:
		using nd = std::normal_distribution<double>;
		using urd = std::uniform_real_distribution<double>;

		Mode m_mode;
		BOX3D m_bounds;
		double m_mean_x;
		double m_mean_y;
		double m_mean_z;
		double m_stdev_x;
		double m_stdev_y;
		double m_stdev_z;
		double m_delX;
		double m_delY;
		double m_delZ;
		uint64_t m_time;
		int m_numReturns;
		int m_returnNum;
		point_count_t m_index;
		Arg* m_seedArg;
		uint32_t m_seed;
		std::mt19937 m_generator;
		std::unique_ptr<nd> m_normalX;
		std::unique_ptr<nd> m_normalY;
		std::unique_ptr<nd> m_normalZ;
		std::unique_ptr<urd> m_uniformX;
		std::unique_ptr<urd> m_uniformY;
		std::unique_ptr<urd> m_uniformZ;

		virtual void addArgs(ProgramArgs& args);
		virtual void prepared(PointTableRef table);
		virtual void initialize();
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void ready(PointTableRef table);
		virtual bool processOne(PointRef& point);
		virtual point_count_t read(PointViewPtr view, point_count_t count);
		virtual bool eof()
		{
			return false;
		}

		FauxReader& operator=(const FauxReader&); // not implemented
		FauxReader(const FauxReader&); // not implemented
	};
}

#endif // #ifndef FAUXREADER_H
