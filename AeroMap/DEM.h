#ifndef DEM_H
#define DEM_H

#include "AeroLib.h"		// application header

class DEM
{
public:
    static void create_dem(
        XString input_point_cloud, XString dem_type, std::vector<double> radius_steps /*= ['0.56']*/,
        XString output_type = "max", bool gapfill = true,
        XString outdir = "", double resolution = 0.1, int max_workers = 1, int max_tile_size = 4096,
        int decimation = 0, bool keep_unfilled_copy = false,
        bool apply_smoothing = true, int max_tiles = 0);

    static std::vector<double> get_dem_radius_steps(XString stats_file, int steps, double resolution, double multiplier = 1.0);

    static int gdal_fillnodata(int src_band, XString out_format, XString src_filename, XString dst_filename);
};

#endif // #ifndef DEM_H
