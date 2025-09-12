#ifndef PTSREADER_H
#define PTSREADER_H

#include <istream>

#include <pdal/Reader.h>

namespace pdal
{
	class PtsReader : public Reader
	{
	public:
		PtsReader() : m_separator(' '), m_PointCount(0), m_istream(NULL)
		{}
		std::string getName() const;

	private:
		/**
		  Initialize the reader by opening the file and reading the header line
		  for the point count.
		  Closes the file on completion.

		  \param table  Point table being initialized.
		*/
		virtual void initialize(PointTableRef table);

		/**
		  Add dimensions fixed dimensions for PTS format
		  http://w3.leica-geosystems.com/kb/?guid=5532D590-114C-43CD-A55F-FE79E5937CB2

		  \param layout  Layout to which the dimenions are added.
		*/
		virtual void addDimensions(PointLayoutPtr layout);

		/**
		  Reopen the file in preparation for reading.

		  \param table  Point table to make ready.
		*/
		virtual void ready(PointTableRef table);

		/**
		  Read up to numPts points into the \ref view.

		  \param view  PointView in which to insert point data.
		  \param numPts  Maximum number of points to read.
		  \return  Number of points read.
		*/
		virtual point_count_t read(PointViewPtr view, point_count_t numPts);

		/**
		  Close input file.

		  \param table  PointTable we're done with.
		*/
		virtual void done(PointTableRef table);

	private:
		char m_separator;
		point_count_t m_PointCount;
		std::istream* m_istream;
		StringList m_dimNames;
		Dimension::IdList m_dims;
	};
}

#endif // #ifndef PTSREADER_H
