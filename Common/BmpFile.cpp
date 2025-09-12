//	File:				bitmap.cpp
//	Original Author:	Tobias Franke (webmaster@cyberhead.de) 2001
//
//	Encapsulates bitmaps, including file io
//
//	Notes
//		- only supports 24 bit bitmaps
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#define open _open
#define lseek _lseek
#define read _read
#define write _write
#define close _close
#define chmod _chmod
#else
#include <unistd.h>
#endif

#include "BmpFile.h"

BmpFile::BmpFile()
	: mi_Width(0)
	, mi_Height(0)
	, mp_Data(nullptr)
{
	memset(ms_FileName, 0, sizeof(ms_FileName));
}

BmpFile::~BmpFile()
{
	if (mp_Data)
		delete mp_Data;
}

bool BmpFile::Create(int width, int height)
{
	// create a new bitmap in memory
	//

	memset(ms_FileName, 0, sizeof(ms_FileName));

	mi_Width = width;
	mi_Height = height;

	if (mp_Data)		// delete existing data, if any
		delete mp_Data;

	mp_Data = new UInt8[(mi_Width*mi_Height) * 3];

	return (mp_Data != nullptr);
}

void BmpFile::GetColor(int x, int y, UInt8* r, UInt8* g, UInt8* b)
{
	// i pass RGB values by reference explicitly because there is so much room for
	// error returning a single 32 bit value - is it AARRGGBB or BBGGRRAA ? does it
	// even have AA? The reliable way to do this is to return "red" in the "r"
	// parameter so the user knows explicitly that it is definitely the "red" value.
	//

	if (x < mi_Width && y < mi_Height)
	{
		int pixel = (y*mi_Width + x) * 3;
		*r = mp_Data[pixel];
		*g = mp_Data[pixel + 1];
		*b = mp_Data[pixel + 2];
	}
}

void BmpFile::SetColor(int x, int y, UInt8 r, UInt8 g, UInt8 b)
{
	//	see GetColor() comments
	//

	if (x < mi_Width && y < mi_Height)
	{
		int pixel = (y*mi_Width + x) * 3;
		mp_Data[pixel] = r;
		mp_Data[pixel + 1] = g;
		mp_Data[pixel + 2] = b;
	}
}

bool BmpFile::Load( const char* fileName )
{
	bmpPAL color;
	int x, y;
	bmpFHEAD filehead;		// File Header
	bmpIHEAD infohead;		// Info Header

	memset(ms_FileName, 0, sizeof(ms_FileName));
	memset(&filehead, 0, sizeof(filehead));
	memset(&infohead, 0, sizeof(infohead));

	// under windows, you HAVE to say O_BINARY or the first binary value
	// it interprets as an eof marker will just stop reading
	int inputFile = open(fileName, O_RDONLY | O_BINARY);
	if (inputFile == -1)
	{
		Logger::Write(__FUNCTION__, "Unable to open file '%s' for input. (%s)", fileName, strerror(errno));
		return false;
	}

	lseek(inputFile, 0, SEEK_SET);

	read(inputFile, &filehead.type0, 1);
	read(inputFile, &filehead.type1, 1);
	read(inputFile, &filehead.fileSize, 4);
	read(inputFile, &filehead.reserved0, 2);
	read(inputFile, &filehead.reserved1, 2);
	read(inputFile, &filehead.bitOffset, 4);

	read(inputFile, &infohead.size, 4);
	read(inputFile, &infohead.width, 4);
	read(inputFile, &infohead.height, 4);
	read(inputFile, &infohead.planes, 2);
	read(inputFile, &infohead.bpp, 2);
	read(inputFile, &infohead.compression, 4);
	read(inputFile, &infohead.imageSize, 4);
	read(inputFile, &infohead.pixMeterX, 4);
	read(inputFile, &infohead.pixMeterY, 4);
	read(inputFile, &infohead.colorsUsed, 4);
	read(inputFile, &infohead.colorsImp, 4);

	if (infohead.bpp != 24)	// unhandled format
	{
		close(inputFile);
		Logger::Write(__FUNCTION__, "Unhandled image format: %d bits per pixel.", infohead.bpp);
		return false;
	}

	mi_Width = infohead.width;		// image width, pixels - scan lines on disk may be larger with padding
	mi_Height = infohead.height;
	if (mp_Data)
		delete mp_Data;
	mp_Data = new UInt8[(mi_Width*mi_Height) * 3];		// "*3" for 24-bit bitmaps

	if (!mp_Data)
	{
		close(inputFile);
		Logger::Write(__FUNCTION__, "Unable to to allocate memory for image data.");
		return false;
	}

	// according to doc, bits are stored as scan-lines left to right from the bottom up
	// and (each scan line?) must be padded to end on 32 bit boundary

	int pad = ((mi_Width * 3) % 4 ? 4 - (mi_Width * 3) % 4 : 0);	// # of bytes needed to end on 32 bit boundary
	for (y = mi_Height - 1; y >= 0; --y)		// flip vertically so (0,0) = upper left corner
	{
		for (x = 0; x < mi_Width; ++x)
		{
			/* int size = */ read(inputFile, &color, 3);		// bytes stored BGR on disk
			SetColor(x, y, color.b, color.g, color.r);	// set pixel & map BGR -> RGB
		}
		if (pad)
		{
			UInt8 buf[4];
			/* int ctr = */ read(inputFile, buf, pad);		// returns # of items, not bytes
		}
	}

	close(inputFile);
	strncpy(ms_FileName, fileName, sizeof(ms_FileName));

	return (mp_Data != nullptr);
}

bool BmpFile::Save(const char* fileName)
{
	bmpFHEAD filehead;		// File Header
	bmpIHEAD infohead;		// Info Header
	bmpPAL color;
	long bitsize;			// Size of the bitmap

	if (!mp_Data)			// if there's no data, don't even start writing
		return false;

	chmod(fileName, S_IREAD | S_IWRITE);
	int outputFile = open(fileName, O_CREAT | O_TRUNC | O_BINARY | O_RDWR, S_IREAD | S_IWRITE);
	if (outputFile == -1)
	{
		Logger::Write(__FUNCTION__, "Unable to open file '%s' for output. (%s)", fileName, strerror(errno));
		return false;
	}

	bitsize = (mi_Width * 24 + 7) / 8 * abs(mi_Height);

	//FILEHEADER
	filehead.type0 = 0x42;		// 'B'
	filehead.type1 = 0x4D;		// 'M'
	filehead.fileSize = FHEAD_SIZE + IHEAD_SIZE + bitsize;
	filehead.reserved0 = 0;
	filehead.reserved1 = 0;
	filehead.bitOffset = FHEAD_SIZE + IHEAD_SIZE;

	// one "packed" write() does not necessarily work - each individual field needs to be written to the required size/offset
	write(outputFile, &filehead.type0, 1);		// don't even use "sizeof()" here - explicitly tell it how many bytes to write
	write(outputFile, &filehead.type1, 1);
	write(outputFile, &filehead.fileSize, 4);
	write(outputFile, &filehead.reserved0, 2);
	write(outputFile, &filehead.reserved1, 2);
	write(outputFile, &filehead.bitOffset, 4);
	//FILEHEADER DONE

	//HEADER
	infohead.size			= IHEAD_SIZE;
	infohead.width			= mi_Width;
	infohead.height			= mi_Height;
	infohead.planes			= 1;
	infohead.bpp			= 24;			// 24bit
	infohead.compression	= 0;			// RGB
	infohead.imageSize		= bitsize;
	infohead.pixMeterX 		= 2952;
	infohead.pixMeterY		= 2952;
	infohead.colorsUsed		= 0;
	infohead.colorsImp 		= 0;

	write(outputFile, &infohead.size, 4);
	write(outputFile, &infohead.width, 4);
	write(outputFile, &infohead.height, 4);
	write(outputFile, &infohead.planes, 2);
	write(outputFile, &infohead.bpp, 2);
	write(outputFile, &infohead.compression, 4);
	write(outputFile, &infohead.imageSize, 4);
	write(outputFile, &infohead.pixMeterX, 4);
	write(outputFile, &infohead.pixMeterY, 4);
	write(outputFile, &infohead.colorsUsed, 4);
	write(outputFile, &infohead.colorsImp, 4);
	//HEADER DONE

	int pad = ((mi_Width * 3) % 4 ? 4 - (mi_Width * 3) % 4 : 0);	// # of bytes needed to end on 32 bit boundary
	for (int y = mi_Height - 1; y >= 0; y--)
	{
		for (int x = 0; x < mi_Width; x++)
		{
			GetColor(x, y, &color.r, &color.g, &color.b);
			write(outputFile, &color.b, 1);
			write(outputFile, &color.g, 1);
			write(outputFile, &color.r, 1);
		}
		if (pad)
		{
			UInt8 buf[4];
			/* int ctr = */ write(outputFile, buf, pad);
		}
	}

	close(outputFile);
	chmod(fileName, S_IREAD | S_IWRITE);

	return true;
}
