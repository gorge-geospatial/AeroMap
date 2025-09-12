#ifndef GCPFILE_H
#define GCPFILE_H

#include <vector>

#include "gdal_priv.h"
#include "cpl_conv.h"		// for CPLMalloc()
#include "ogrsf_frmts.h"

#include "XString.h"

class GcpFile
{
public:

	struct EntryType
	{
		double x, y, z;		// world coordinates
		int px, py;			// pixel coordinates
		XString file_name;	// image file name

		XString coords_key()
		{
			return XString::Format("%0.6f %0.6f %0.6f", x, y, z);
		}
		
		XString to_string()
		{
			return XString::Format("%0.6f %0.6f %0.6f %d %d %s", 
				x, y, z, px, py, file_name.c_str());
		}
	};
	std::vector<EntryType> m_entry_list;

public:

	GcpFile(XString file_name);
	~GcpFile();

	bool exists();
	int entry_count();
	void check_entries();
	XString wgs84_utm_zone();
	
	XString make_utm_copy(XString gcp_file_output, std::vector<XString>file_names = std::vector<XString>(), std::vector<XString>rejected_entries = std::vector<XString>());
	XString make_resized_copy(XString gcp_file_output, double ratio);
	XString make_filtered_copy(XString gcp_file_output, XString images_dir, int min_images = 3);

private:

	XString m_file_name;
	XString m_raw_srs;			// srs, as read from header
	OGRSpatialReference m_srs;

private:

	void read();
	void write_file(XString file_name, XString text);

	OGRSpatialReference parse_srs_header(XString header);

	bool file_in_list(XString file_name, std::vector<XString> list);
};

#endif // #ifndef GCPFILE_H
