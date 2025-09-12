////////////////////////////////////////////////////////////
//
//	JpegFile - A C++ class to allow reading and writing of
//	RGB and Grayscale JPEG images. (actually, it reads all forms
//	that the JPEG lib will decode into RGB or grayscale) and
//	writes only RGB and Grayscale.
//
//	It is based on a Win32 compilation of the IJG V.6a code.
//
//	This will only work on 32-bit Windows systems. I have only
//	tried this with Win 95, VC++ 4.1.
//
//	This class Copyright 1997, Chris Losinger
//	This is free to use and modify provided my name is included.
//
//	Comments:
//	Thanks to Robert Johnson for discovering a UInt32-alignment bug
//	Thanks to Lee Bode for catching a bug in CMfcappView::OnFileGetdimensionsjpg()
//
////////////////////////////////////////////////////////////
//
//	General Usage:
//
//	#include this file.
//	link with jpeglib2.lib
//
//	All functions here are static. There is no need to have a JpegFile object.
//	There is actually nothing in a JpegFile object anyway.
//
//	So, you can do this :
//
//		bool ok = JpegFile::vertFlipBuf(buf, widthbytes, height);
//
//	instead of this :
//
//		JpegFile jpgOb;
//		bool ok = jpgOb.vertFlipBuf(buf, widthbytes, height);
//
/////
//
//	Linking usage :
//	It is sometimes necessary to set /NODEFAULTLIB:LIBC (or LIBCD) to use this
//	class.
//
/////
//
//	Error reporting:
//	The class generates message boxes in response to JPEG errors.
//
//	The JpegFile.cpp fn my_error_exit defines the behavior for
//	fatal errors : show msg box, return to caller.
//
//	Warnings are handled by jpeglib.lib - which	generates msg boxes too.
//
////////////////////////////////////////////////////////////////

/*
////////////////////////////////////////////////////////////////
//	Reading Usage :

	UInt32 height;
	UInt32 width;
	UInt8 *dataBuf;
   //read the file
   dataBuf=JpegFile::JpegFileToRGB(fileName,
								&width,
								&height);
	if (dataBuf==NULL) {
		return;
	}

	// RGB -> BGR
	JpegFile::BGRFromRGB(dataBuf, m_width, m_height);


	UInt8 *buf;
	// create a UInt32 aligned buffer from the JpegFile object
	buf = JpegFile::MakeUInt32AlignedBuf(dataBuf,
									width,
									height,
									&m_widthDW);

	// flip that buffer
	JpegFile::VertFlipBuf(m_buf, m_widthDW, m_height);

	// you now have a buffer ready to be used as a DIB

	// be sure to delete [] dataBuf;	// !!!!!!!!!!
	//			delete [] buf;


	// Writing Usage


	// this assumes your data is stored as a 24-bit RGB DIB.
	// if you have a 1,4,8,15/16 or 32 bit DIB, you'll have to
	// do some work to get it into a 24-bit RGB state.

	UInt8 *tmp=NULL;

	// assume buf is a UInt32-aligned BGR buffer, vertically flipped
	// as if read from a BMP file.

	// un-UInt32-align
	tmp=JpegFile::RGBFromUInt32Aligned(buf,
									widthPix,
									widthBytes,
									height);

	// vertical flip
	JpegFile::VertFlipBuf(tmp, widthPix * 3, height);

	// reverse BGR
	JpegFile::BGRFromRGB(tmp, widthPix, height);

	if (tmp==NULL) {
		AfxMessageBox("~UInt32 Memory Error");
		return;
	}

	// write it
	bool ok=JpegFile::RGBToJpegFile(fileName,
						tmp,
						width,
						height,
						true,
						75);
	if (!ok) {
		AfxMessageBox("Write Error");
	}

	delete [] tmp;

////////////////////////////////////////////////////////////////

*/

#include "MarkTypes.h"		// universal type definitions

#include <string>

//
//	for UInt32 aligning a buffer
//

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

class JpegFile
{
public:

	////////////////////////////////////////////////////////////////
	// read a JPEG file to an RGB buffer - 3 bytes per pixel
	// returns a ptr to a buffer .
	// caller is responsible for cleanup!!!
	// UInt8 *buf = JpegFile::JpegFileToRGB(....);
	// delete [] buf;

	static UInt8* JpegFileToRGB( std::string fileName,		// path to image
							     unsigned int *width,		// image width in pixels
							     unsigned int *height );	// image height

	////////////////////////////////////////////////////////////////
	// write a JPEG file from a 3-component, 1-byte per component buffer

	static bool RGBToJpegFile( std::string fileName,			// path
							   UInt8 *dataBuf,				// RGB buffer
							   unsigned int width,			// pixels
							   unsigned int height,			// rows
							   bool color,					// true = RGB
															// false = Grayscale
							   int quality );				// 0 - 100


	////////////////////////////////////////////////////////////////
	// fetch width / height of an image

	static bool GetJPGDimensions( std::string fileName,		// path
								  unsigned int *width,		// pixels
								  unsigned int *height );

	////////////////////////////////////////////////////////////////
	//	utility functions
	//	to do things like UInt32-align, flip, convert to grayscale, etc.
	//

	////////////////////////////////////////////////////////////////
	// allocates a UInt32-aligned buffer, copies data buffer
	// caller is responsible for delete []'ing the buffer

	static UInt8* MakeUInt32AlignedBuf( UInt8* dataBuf,					// input buf
									   unsigned int widthPix,			// input pixels
									   unsigned int height,				// lines
									   unsigned int *uiOutWidthBytes );	// new width bytes


	////////////////////////////////////////////////////////////////
	// if you have a UInt32 aligned buffer, this will copy the
	// RGBs out of it into a new buffer. new width is widthPix * 3 bytes
	// caller is responsible for delete []'ing the buffer

	static UInt8* RGBFromUInt32Aligned( UInt8* inBuf,					// input buf
									  unsigned int widthPix,			// input size
									  unsigned int widthBytes,			// input size
									  unsigned int height );

	////////////////////////////////////////////////////////////////
	// vertically flip a buffer - for BMPs
	// in-place

	// note, this routine works on a buffer of width widthBytes: not a
	// buffer of widthPixels.
	static bool VertFlipBuf( UInt8* inbuf,						// input buf
					         unsigned int widthBytes,			// input width bytes
					         unsigned int height );				// height

	// NOTE :
	// the following routines do their magic on buffers with a whole number
	// of pixels per data row! these are assumed to be non UInt32-aligned buffers.

	////////////////////////////////////////////////////////////////
	// convert RGB to grayscale	using luminance calculation
	// in-place

	static bool MakeGrayScale( UInt8* buf,						// input buf
						       unsigned int widthPix,			// width in pixels
						       unsigned int height );			// height

	////////////////////////////////////////////////////////////////
	// swap Red and Blue bytes
	// in-place

	static bool BGRFromRGB( UInt8* buf,							// input buf
					        unsigned int widthPix,				// width in pixels
					        unsigned int height );				// lines

	////////////////////////////////////////////////////////////////
	// these do nothing
	JpegFile();		// creates an empty object
	~JpegFile();	// destroys nothing
};
