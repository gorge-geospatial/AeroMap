// PngFile.cpp
// Manager for PNG format image files
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <assert.h>

#include "zlib.h"
#include "Logger.h"
#include "PngFile.h"


PngFile::PngFile(const char* fileName)
	: m_image_width(0)
	, m_image_height(0)
	, m_image_rowbytes(0)
	, m_ImageSize(0)
	, m_png_ptr(nullptr)
	, m_info_ptr(nullptr)
	, m_image_channels(0)
	, mp_ImageData(nullptr)
{
	//Logger::Write(__FUNCTION__, "Compiled with libpng %s; using libpng %s.", PNG_LIBPNG_VER_STRING, png_libpng_ver);
	//Logger::Write(__FUNCTION__, "Compiled with zlib %s; using zlib %s.", ZLIB_VERSION, zlib_version);

	if (fileName != nullptr)
	{
		if (LoadFile(fileName) == false)
		{
			Logger::Write(__FUNCTION__, "Unable to load file '%s'", fileName);
		}
	}
}

PngFile::~PngFile()
{
	delete mp_ImageData;
	mp_ImageData = nullptr;

	png_destroy_read_struct(&m_png_ptr, &m_info_ptr, nullptr);

//TODO:
	//what other pnglib items need to be freed?

}

int PngFile::Save(const char* fileName)
{
	// Write png file.
	//
	// Inputs:
	//		fileName = path/file name to write
	// Outputs:
	//		return = 0 on success, else 1
	//

	XString strFileName;

	if (fileName == nullptr)
	{
		strFileName = ms_FileName;
	}
	else
	{
		strFileName = fileName;
	}
	strFileName.Trim();
	assert(strFileName.GetLength() > 0);

	// No data to write
	assert(m_image_width > 0 && m_image_height > 0);

	int code = 0;
	png_structp png_ptr = nullptr;
	png_infop info_ptr = nullptr;
	png_bytep row = nullptr;

	// Open file for writing (binary mode)
	FILE* fp = fopen(strFileName.c_str(), "wb");
	if (fp == nullptr)
	{
		Logger::Write(__FUNCTION__, "Could not open file %s for writing.", strFileName.c_str());
		code = 1;
		goto finalise;
	}
	// two libPNG structures are allocated and initialised.

	// The write structure contains information about how the PNG file will be written(or read).
	// The info structure contains information about the PNG image that will be written into the actual file.This allow programmes to find out characteristics of the image.

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
										nullptr, 
										nullptr, 
										nullptr);
	if (png_ptr == nullptr)
	{
		Logger::Write(__FUNCTION__, "Could not allocate write struct");
		code = 1;
		goto finalise;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == nullptr)
	{
		Logger::Write(__FUNCTION__, "Could not allocate info struct");
		code = 1;
		goto finalise;
	}

	// this is a form of exception handling for C. Basically, after this little block of code, 
	// if any libPNG function fails, execution will jump back to the setjmp function with a 
	// non - zero value. The if statement is then entered.

	// Within this example code, the jump point is only set once at this point. Therefore, 
	// if an 'exception' occurs, it is not possible to determine from which libPNG function
	// it was thrown.However, it is possible to repeat this block before each libPNG function
	// call, defining a new point to jump back to with an appropriate response.

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		Logger::Write(__FUNCTION__, "Error during png creation");
		code = 1;
		goto finalise;
	}

	// Various meta data for the image is now set, such as the size and the colour depth per channel.

	// A further piece of meta information is also set, an image title.There are various other bits 
	// of standard text that can be set, such as an author.

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	{
		png_uint_32 width = GetWidth();
		png_uint_32 height = GetHeight();
		png_set_IHDR(
			png_ptr, 
			info_ptr,
			width,							// width of image, pixels
			height,							// height of image, pixels
			8,								// bit depth (valid values are 1, 2, 4, 8, 16 and depends also on the color_type)
			PNG_COLOR_TYPE_RGB,				// describes which color/alpha channels are present.
			PNG_INTERLACE_NONE,				// PNG_INTERLACE_NONE, PNG_INTERLACE_ADAM7
			PNG_COMPRESSION_TYPE_BASE,		// must be PNG_COMPRESSION_TYPE_DEFAULT
			PNG_FILTER_TYPE_BASE);			// must be PNG_FILTER_TYPE_DEFAULT (if embedding in MNG datastream, can be PNG_INTRAPIXEL_DIFFERENCING)

		// Set title
		//if (title != nullptr)
		//{
		//	png_text title_text;
		//	title_text.compression = PNG_TEXT_COMPRESSION_NONE;
		//	title_text.key = "Title";
		//	title_text.text = title;
		//	png_set_text(png_ptr, info_ptr, &title_text, 1);
		//}

		png_write_info(png_ptr, info_ptr);

		// Now the image data is written one row at a time. A single row buffer is created which
		// is of the correct format.

		// Allocate memory for one row (row size always seems to be 4 bytes * width)
		row = (png_bytep)malloc(4 * width * sizeof(png_byte));

		// Write image data
		for (png_uint_32 y = 0; y < height; ++y)
		{
			for (png_uint_32 x = 0; x < width; ++x)
			{
				PixelType pix = GetPixel(x, y);
				row[x * 3] = pix.R;
				row[x * 3 + 1] = pix.G;
				row[x * 3 + 2] = pix.B;
				row[x * 3 + 3] = pix.A;
			}
			png_write_row(png_ptr, row);
		}

		// End write
		png_write_end(png_ptr, nullptr);
	}

	// The last stage is just some cleaning up.This point is jumped to if there has been an error.

finalise:
	if (fp != nullptr) 
		fclose(fp);
	if (info_ptr != nullptr)
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != nullptr)
		png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
	if (row != nullptr)
		free(row);

	return code;
}

PixelType PngFile::GetPixel(UInt32 x, UInt32 y)
{
	PixelType pix;

	if (x < m_image_width && y < m_image_height)
	{
		unsigned int offset = (y * m_image_rowbytes + x * m_image_channels);
		assert(offset < m_ImageSize);

		pix.R = mp_ImageData[offset];
		if (m_image_channels > 1)
			pix.G = mp_ImageData[offset + 1];
		if (m_image_channels > 2)
			pix.B = mp_ImageData[offset + 2];
		if (m_image_channels > 3)
			pix.A = mp_ImageData[offset + 3];
	}

	return pix;
}

void PngFile::SetPixel(UInt32 x, UInt32 y, PixelType pix)
{
	if (x < m_image_width && y < m_image_height)
	{
		unsigned int offset = (y * m_image_rowbytes + x * m_image_channels);
		assert(offset < m_ImageSize);

		mp_ImageData[offset] = pix.R;
		if (m_image_channels > 1)
			mp_ImageData[offset + 1] = pix.G;
		if (m_image_channels > 2)
			mp_ImageData[offset + 2] = pix.B;
		if (m_image_channels > 3)
			mp_ImageData[offset + 3] = pix.A;
	}
}

void PngFile::SetImageData(unsigned char* pData)
{
	memcpy(mp_ImageData, pData, m_ImageSize);
}

bool PngFile::Create(UInt32 width, UInt32 height, int channels)
{
	// Create the memory structures required to support writing a png file
	//
	// Most of this is trial & error - recommend just using it
	// to create/save an image then deleting the PngFile instance

	m_image_width = width;
	m_image_height = height;
	m_image_channels = channels;

	// row size seems to be image width x 4 regardless of pixel format/size
	m_image_rowbytes = m_image_width * 4;
	m_ImageSize = m_image_rowbytes * m_image_height;

	delete mp_ImageData;
	mp_ImageData = new unsigned char[m_image_height * m_image_rowbytes];

	return mp_ImageData != nullptr;
}

bool PngFile::GetImage(double display_exponent)
{
	// read image data
	//
	// outputs:
	//		mp_ImageData = ptr to allocated memory with image data
	//

	// display_exponent == LUT_exponent * CRT_exponent

    double gamma;
    png_bytepp row_pointers = nullptr;

	// setjmp() must be called in every function that calls a PNG-reading
	// libpng function

	if (setjmp(png_jmpbuf(m_png_ptr)))
	{
		png_destroy_read_struct(&m_png_ptr, &m_info_ptr, nullptr);
		return false;
	}

    // expand palette images to RGB, low-bit-depth grayscale images to 8 bits,
    // transparency chunks to full alpha channel; strip 16-bit-per-sample
    // images to 8 bits per sample; and convert grayscale to RGB[A]

	if (m_color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(m_png_ptr);
	if (m_color_type == PNG_COLOR_TYPE_GRAY && m_bit_depth < 8)
		png_set_expand(m_png_ptr);
	if (png_get_valid(m_png_ptr, m_info_ptr, PNG_INFO_tRNS))
		png_set_expand(m_png_ptr);
	if (m_bit_depth == 16)
		png_set_strip_16(m_png_ptr);
	if (m_color_type == PNG_COLOR_TYPE_GRAY || m_color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(m_png_ptr);


	// unlike the example in the libpng documentation, we have *no* idea where
	// this file may have come from--so if it doesn't have a file gamma, don't
	// do any correction ("do no harm")

    if (png_get_gAMA(m_png_ptr, m_info_ptr, &gamma))
        png_set_gamma(m_png_ptr, display_exponent, gamma);


    // all transformations have been registered; now update info_ptr data,
    // get rowbytes and channels, and allocate image memory

    png_read_update_info(m_png_ptr, m_info_ptr);

    m_image_rowbytes = static_cast<unsigned long>(png_get_rowbytes(m_png_ptr, m_info_ptr));
    m_image_channels = (int)png_get_channels(m_png_ptr, m_info_ptr);

	// png_get_channels() shall return the number of data channels per pixel for the color type of the image. 
	// the number of channels shall range from 1-4 depending on the color type as given below.
	//	1 - PNG_COLOR_TYPE_GRAY or PNG_COLOR_TYPE_PALETTE
	//	2 - PNG_COLOR_TYPE_GRAY_ALPHA
	//	3 - PNG_COLOR_TYPE_RGB
	//	4 - PNG_COLOR_TYPE_RGB_ALPHA or PNG_COLOR_TYPE_RGB+filler byte 

	assert(m_image_channels >= 1 && m_image_channels <= 4);
	assert(mp_ImageData == nullptr);

	m_ImageSize = m_image_rowbytes * m_image_height;
	mp_ImageData = (unsigned char*)malloc(m_ImageSize);
	if (!mp_ImageData)
	{
		png_destroy_read_struct(&m_png_ptr, &m_info_ptr, nullptr);
		return false;
	}
	row_pointers = (png_bytepp)malloc(m_image_height*sizeof(png_bytep));
	if (!row_pointers)
	{
		png_destroy_read_struct(&m_png_ptr, &m_info_ptr, nullptr);
		free(mp_ImageData);
		mp_ImageData = nullptr;
		return false;
	}

    //Logger::Write(__FUNCTION__, "channels = %d, rowbytes = %ld, height = %ld", m_image_channels, m_image_rowbytes, m_image_height);


    // set the individual row_pointers to point at the correct offsets

    for (png_uint_32 i = 0;  i < m_image_height;  ++i)
        row_pointers[i] = mp_ImageData + i*m_image_rowbytes;


    // now we can go ahead and just read the whole image

    png_read_image(m_png_ptr, row_pointers);


	// and we're done!  (png_read_end() can be omitted if no processing of
	// post-IDAT text/time/etc. is desired)

	free(row_pointers);
	row_pointers = nullptr;

	png_read_end(m_png_ptr, nullptr);

	return true;
}

bool PngFile::LoadFile(const char* fileName)
{
	FILE* pFile = fopen(fileName, "rb");

	if (!pFile) 
	{
        Logger::Write(__FUNCTION__, "Unable to open file '%s'", fileName);
		return false;
    } 
	
	int rc = ReadInit(pFile);
    if (rc != 0)
	{
        switch (rc) {
        case 1:
			Logger::Write(__FUNCTION__, "[%s] is not a PNG file: incorrect signature", fileName);
			break;
        case 2:
			Logger::Write(__FUNCTION__, "[%s] has bad IHDR (libpng longjmp)", fileName);
			break;
        case 4:
			Logger::Write(__FUNCTION__, "insufficient memory");
			break;
        default:
			Logger::Write(__FUNCTION__, "unknown readpng_init() error");
			break;
        }
    }
	else
	{
		// read the data

		// Next set the default value for our display-system exponent, i.e.,
		// the product of the CRT exponent and the exponent corresponding to
		// the frame-buffer's lookup table (LUT), if any.  This is not an
		// exhaustive list of LUT values (e.g., OpenStep has a lot of weird
		// ones), but it should cover 99% of the current possibilities.  And
		// yes, these ifdefs are completely wasted in a Windows program...

		double LUT_exponent;              /* just the lookup table */
		double CRT_exponent = 2.2;        /* just the monitor */

#if defined(NeXT)
		LUT_exponent = 1.0 / 2.2;
		// if (some_next_function_that_returns_gamma(&next_gamma))
		//    LUT_exponent = 1.0 / next_gamma;
		//
#elif defined(sgi)
		LUT_exponent = 1.0 / 1.7;
		// there doesn't seem to be any documented function to get the
		// "gamma" value, so we do it the hard way */
		infile = fopen("/etc/config/system.glGammaVal", "r");
		if (infile) {
			double sgi_gamma;

			fgets(tmpline, 80, infile);
			fclose(infile);
			sgi_gamma = atof(tmpline);
			if (sgi_gamma > 0.0)
				LUT_exponent = 1.0 / sgi_gamma;
		}
#elif defined(Macintosh)
		LUT_exponent = 1.8 / 2.61;
		// if (some_mac_function_that_returns_gamma(&mac_gamma))
		//    LUT_exponent = mac_gamma / 2.61;
		//
#else
	    LUT_exponent = 1.0;   /* assume no LUT:  most PCs */
#endif

		// the defaults above give 1.0, 1.3, 1.5 and 2.2, respectively
		double display_exponent = LUT_exponent * CRT_exponent;

		// the defaults above give 1.0, 1.3, 1.5 and 2.2, respectively
		//double default_display_exponent = LUT_exponent * CRT_exponent;

		if (!GetImage(display_exponent))
		{
			Logger::Write(__FUNCTION__, "GetImage() failed");
		}
	}

	fclose(pFile);

	ms_FileName = fileName;

	return (rc == 0);
}

int PngFile::ReadInit(FILE* pInFile)
{
	// initialize PNG file for reading
	//
	// inputs:
	//		pInFile
	//
	// outputs:
	//		return = 0 for success
	//				 1 for bad sig
	//				 2 for bad IHDR
	//				 4 for no mem
	//				 9 for other error
	
    // first do a quick check that the file really is a PNG image; could
    // have used slightly more general png_sig_cmp() function instead

    unsigned char sig[8];
    fread(sig, 1, 8, pInFile);
    if (!png_check_sig(sig, 8))
		return 1;   // bad signature

    // could pass pointers to user-defined error handlers instead of NULLs:

    m_png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!m_png_ptr)
        return 4;   // out of memory

    m_info_ptr = png_create_info_struct(m_png_ptr);
    if (!m_info_ptr)
	{
        png_destroy_read_struct(&m_png_ptr, nullptr, nullptr);
        return 4;   // out of memory
    }

    // we could create a second info struct here (end_info), but it's only
    // useful if we want to keep pre- and post-IDAT chunk info separated
    // (mainly for PNG-aware image editors and converters)

    // setjmp() must be called in every function that calls a PNG-reading
    // libpng function

    if (setjmp(png_jmpbuf(m_png_ptr))) 
	{
        png_destroy_read_struct(&m_png_ptr, &m_info_ptr, nullptr);
        return 2;
    }

    png_init_io(m_png_ptr, pInFile);
    png_set_sig_bytes(m_png_ptr, 8);  /* we already read the 8 signature bytes */

    png_read_info(m_png_ptr, m_info_ptr);  /* read all PNG info up to image data */


    // alternatively, could make separate calls to png_get_image_width(),
    // etc., but want bit_depth and color_type for later [don't care about
    // compression_type and filter_type => NULLs]

	m_image_width = 0;
	m_image_height = 0;
    png_uint_32 rc = png_get_IHDR(m_png_ptr, m_info_ptr, &m_image_width, &m_image_height, &m_bit_depth, &m_color_type, nullptr, nullptr, nullptr);
	if (rc != 1)
	{
		Logger::Write(__FUNCTION__, "png_get_IHDR() failed");
		return 9;
	}

	// found this on stack overflow to "fix" the warning: 
	//	"libpng warning: interlace handling should be turned on when using png_read_image."
	/* int number_passes = */ png_set_interlace_handling(m_png_ptr);

    // OK, that's all we need for now; return happy

    return 0;
}
