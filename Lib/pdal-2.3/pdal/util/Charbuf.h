#ifndef CHARBUF_H
#define CHARBUF_H

#include <streambuf>
#include <iostream>
#include <vector>

namespace pdal
{
	/**
	  Allow a data buffer to be used at a std::streambuf.
	*/
	class Charbuf : public std::streambuf
	{
	public:
		/**
		  Construct an empty Charbuf.
		*/
		Charbuf() : m_bufOffset(0)
		{}

		/**
		  Construct a Charbuf that wraps a byte vector.

		  \param v  Byte vector to back streambuf.
		  \param bufOffset  Offset in vector (ignore bytes before offset).
		*/
		Charbuf(std::vector<char>& v, pos_type bufOffset = 0)
		{
			initialize(v.data(), v.size(), bufOffset);
		}

		/**
		  Construct a Charbuf that wraps a byte buffer.

		  \param buf  Buffer to back streambuf.
		  \param count  Size of buffer.
		  \param bufOffset  Offset in vector (ignore bytes before offset).
		*/
		Charbuf(char* buf, size_t count, pos_type bufOffset = 0)
		{
			initialize(buf, count, bufOffset);
		}

		/**
		  Set a buffer to back a Charbuf.

		  \param buf  Buffer to back streambuf.
		  \param count  Size of buffer.
		  \param bufOffset  Offset in vector (ignore bytes before offset).
		*/
		void initialize(char* buf, size_t count, pos_type bufOffset = 0);

	protected:
		/**
		  Seek to a position in the buffer.

		  \param pos  Position to seek to.
		  \param which  I/O mode [default: rw]
		  \return  Current position adjusted for buffer offset.
		*/
		std::ios::pos_type seekpos(std::ios::pos_type pos,
			std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

		/**
		  Seek to a position based on an offset from a position.

		  \param off  Offset from current position.
		  \param dir  Offset basis (beg, cur or end)
		  \param which  I/O mode [default: rw]
		  \return  Current position adjusted for buffer offset.
		*/
		std::ios::pos_type seekoff(std::ios::off_type off,
			std::ios_base::seekdir dir,
			std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

	private:
		/**
		  Offset that allows one to seek to positions not based on the beginning
		  of the backing vector, but to some other reference point.
		*/
		std::ios::pos_type m_bufOffset;

		/**
		  For the put pointer, it seems we need the beginning of the buffer
		  in order to deal with offsets.
		*/
		char* m_buf;
	};
}

#endif // #ifndef CHARBUF_H
