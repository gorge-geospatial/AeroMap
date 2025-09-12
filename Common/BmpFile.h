// BmpFile.h          
// by Tobias Franke (webmaster@cyberhead.de) 2001
//
// Encapsulates bitmaps, including file io
//

#ifndef BMPFILE_H
#define BMPFILE_H

#include "MarkTypes.h"		// global type definitions
#include "Logger.h"			// message logging facility

class BmpFile
{
private:
	typedef struct _bmpFHEAD
	{
		UInt8	type0;				// 'B'
		UInt8	type1;				// 'M'
		UInt32  fileSize;			// size of bmp file, bytes
		UInt16	reserved0;			// application specific
		UInt16	reserved1;
		UInt32  bitOffset;			// starting address, of the byte where the bitmap data can be found
	
	} bmpFHEAD;
	// this doesn't guarantee "pack(1)" - depends on the linker: __attribute__ ((aligned (1)));
	
	
	typedef struct _bmpIHEAD
	{
		UInt32	size;				// size of this header (40 bytes)
		Int32	width;				// bitmap width in pixels (signed integer).
		Int32	height;				// bitmap height in pixels (signed integer).
		UInt16	planes;				// number of color planes being used. Must be set to 1.
		UInt16	bpp;				// bits per pixel, which is the color depth of the image
		UInt32	compression;		// compression method being used. See the next table for a list
		UInt32	imageSize;			// size of the raw bitmap data
		Int32	pixMeterX;			// horizontal resolution of the image. (pixel per meter, signed inter
		Int32	pixMeterY;			// vertical resolution of the image. (pixel per meter, signed intege
		UInt32	colorsUsed;			// number of colors in the color palette, or 0 to default to 2n.
		UInt32	colorsImp;			// number of important colors used, or 0 when every color is important; generally ignored.
	} bmpIHEAD;
	// this doesn't guarantee "pack(1)" - depends on the linker: __attribute__ ((aligned (1)));
	
	// The compression method field (bytes #30-33) can have the following values:
	// 	Value 	Identified by 	Compression method 	Comments
	// 	0 		BI_RGB 			none 				Most common
	// 	1 		BI_RLE8 		RLE 8-bit/pixel 	Can be used only with 8-bit/pixel bitmaps
	// 	2 		BI_RLE4 		RLE 4-bit/pixel 	Can be used only with 4-bit/pixel bitmaps
	// 	3 		BI_BITFIELDS 	Bit field 			Can be used only with 16 and 32-bit/pixel bitmaps.
	// 	4 		BI_JPEG 		JPEG 				The bitmap contains a JPEG image
	// 	5 		BI_PNG 			PNG 				The bitmap contains a PNG image
	
	typedef struct _bmpPAL
	{
		UInt8 r;
		UInt8 g;
		UInt8 b;
	} bmpPAL;
	// this doesn't guarantee "pack(1)" - depends on the linker: __attribute__ ((aligned (1)));
	
	static const int FHEAD_SIZE = 14;		// sizeof(bmpFHEAD) - just can't use "sizeof()" reliably, even with "attribute(1)", etc
	static const int IHEAD_SIZE = 40;		// sizeof(bmpIHEAD)

	int		mi_Width;
	int		mi_Height;

	char	ms_FileName[255];		// current file, "" = new file
	UInt8*	mp_Data;				// pixel data

public:
	bool Load(const char* fileName);
	bool Save(const char* fileName);

	void GetColor(int x, int y, UInt8 *r, UInt8 *g, UInt8 *b);
	void SetColor(int x, int y, UInt8  r, UInt8  g, UInt8  b);

	// in-lines

	int GetWidth() const { return mi_Width; }
	int GetHeight() const { return mi_Height; }
	UInt8* GetData() const { return mp_Data; }

	// construction / destruction

	BmpFile();
	~BmpFile();
	bool Create(int width, int height);
};

#endif // BMPFILE_H
