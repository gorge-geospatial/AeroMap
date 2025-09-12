#ifndef CROPPER_H
#define CROPPER_H

#include "AeroMap.h"		// application header

class Cropper
{
public:

	Cropper(XString storage_dir, XString files_prefix = "crop");

	XString create_bounds_gpkg(XString pointcloud_path, int buffer_distance = 0, int decimation_step = 40);
	XString create_bounds_geojson(XString pointcloud_path, int buffer_distance = 0, int decimation_step = 40);

	static XString crop(XString gpkg_path, XString geotiff_path, bool keep_original = true);
	static void merge_bounds(std::vector<XString> input_bound_files, XString output_bounds, int buffer_distance = 0);

private:

	XString m_storage_dir;
	XString m_files_prefix;

private:

	XString path(XString suffix);
};

#endif // #ifndef CROPPER_H
