// DEM.cpp
// Port of odm dem/commands.py
//

#include <gdal_alg.h>

#include "PointCloud.h"
#include "DEM.h"

void DEM::create_dem(
	XString input_point_cloud, XString dem_type, std::vector<double> radius_steps,
	XString output_type, bool gapfill, XString outdir, double resolution, int max_workers, int max_tile_size,
	int decimation, bool keep_unfilled_copy, bool apply_smoothing, int max_tiles)
{
	// Create DEM from multiple radii, and optionally gapfill.
	//

	AeroLib::CreateFolder(outdir);

	RectD extent = PointCloud::get_extent(input_point_cloud);
	Logger::Write(__FUNCTION__, "Point cloud bounds are [minx: %0.3f, maxx: %0.3f] [miny: %0.3f, maxy: %0.3f]", extent.x0, extent.x1, extent.y0, extent.y1);
	double ext_width = extent.DX();
	double ext_height = extent.DY();

	int w = (int)ceil(ext_width / resolution);
	int h = (int)ceil(ext_height / resolution);

	// Set a floor, no matter the resolution parameter
	// (sometimes a wrongly estimated scale of the model can cause the resolution
	// to be set unrealistically low, causing errors)
	const int RES_FLOOR = 64;
	if (w < RES_FLOOR && h < RES_FLOOR)
	{
		int prev_w = w;
		int prev_h = h;

		if (w >= h)
		{
			w = RES_FLOOR;
			h = (int)ceil(ext_height / ext_width * RES_FLOOR);
		}
		else
		{
			w = (int)ceil(ext_width / ext_height * RES_FLOOR);
			h = RES_FLOOR;
		}
		double floor_ratio = (double)prev_w / (double)w;
		resolution *= floor_ratio;
		for (int i = 0; i < radius_steps.size(); ++i)
			radius_steps[i] *= floor_ratio;

		Logger::Write(__FUNCTION__, "Really low resolution DEM requested (%dx%d) will set floor at %d pixels.", prev_w, prev_h, RES_FLOOR);
		Logger::Write(__FUNCTION__, "Resolution changed to %0.1f. The scale of this reconstruction might be off.", resolution);
	}

	int final_dem_pixels = w * h;

	// what's the basis of this calc?
	int num_splits = (int)std::max(1.0, ceil(log(ceil(final_dem_pixels / float(max_tile_size * max_tile_size))) / log(2.0)));
	int num_tiles = num_splits * num_splits;
	Logger::Write(__FUNCTION__, "DEM resolution is %dx%d, max tile size is %d, will split DEM generation into %d tiles", w, h, max_tile_size, num_tiles);

	double tile_bounds_width = ext_width / (double)num_splits;
	double tile_bounds_height = ext_height / (double)num_splits;

	struct TileType
	{
		XString filename;
		double radius;
		double minx;
		double maxx;
		double miny;
		double maxy;
	};
	std::vector<TileType> tiles;

	for (double r : radius_steps)
	{
		double minx = extent.x0;
		double maxx = 0.0;

		for (int x = 0; x < num_splits; ++x)
		{
			double miny = extent.y0;
			double maxy = 0.0;

			if (x == num_splits - 1)
				maxx = extent.x1;
			else
				maxx = minx + tile_bounds_width;

			for (int y = 0; y < num_splits; ++y)
			{
				if (y == num_splits - 1)
					maxy = extent.y1;
				else
					maxy = miny + tile_bounds_height;

				XString root_name = XString::Format("%s_r%0.17f_x%d_y%d.tif", dem_type.c_str(), r, x, y);
				XString file_name = XString::CombinePath(outdir, root_name);

				TileType tile;
				tile.filename = file_name;
				tile.radius = r;
				tile.minx = minx;
				tile.maxx = maxx;
				tile.miny = miny;
				tile.maxy = maxy;
				tiles.push_back(tile);

				miny = maxy;
			}
			minx = maxx;
		}
	}

	// Safety check
	if (max_tiles > 0)
	{
		//if len(tiles) > max_tiles and (final_dem_pixels * 4) > get_total_memory():
		//    raise system.ExitException("Max tiles limit exceeded (%s). This is a strong indicator that the reconstruction failed and we would probably run out of memory trying to process this" % max_tiles)
	}

	// why do we do this?
	// Sort tiles by increasing radius
	//tiles.sort(key=lambda t: float(t['radius']), reverse=True)

	//def process_tile(q):
	//    Logger::Write(__FUNCTION__, "Generating %s (%s, radius: %s, resolution: %s)" % (q['filename'], output_type, q['radius'], resolution))
	//    d = pdal.json_gdal_base(q['filename'], output_type, q['radius'], resolution, q['bounds'])
	//    if dem_type == 'dtm':
	//        d = pdal.json_add_classification_filter(d, 2)
	//    if decimation is not None:
	//        d = pdal.json_add_decimation_filter(d, decimation)
	//    pdal.json_add_readers(d, [input_point_cloud])
	//    pdal.run_pipeline(d)
	//parallel_map(process_tile, tiles, max_workers)

	XString pipe_file = XString::CombinePath(GetProject().GetDroneOutputPath(), "pdal_pipe.json");

	int ctr = 0;
	for (TileType tile : tiles)
	{
		GetApp()->LogWrite("Processing tile %d/%d...", ++ctr, tiles.size());

		// create pdal pipeline

		FILE* pFile = fopen(pipe_file.c_str(), "wt");
		if (pFile)
		{
			fprintf(pFile, "[\n");

			// reader
			fprintf(pFile, "	\"%s\",\n", input_point_cloud.c_str());       // infer file type from name

			// filters
			if (decimation > 1)
			{
				fprintf(pFile, "	{\n");
				fprintf(pFile, "	    \"type\": \"filters.decimation\",\n");
				fprintf(pFile, "	    \"step\": %d\n", decimation);
				fprintf(pFile, "	},\n");
			}
			// TODO: filters out all points?
			// if dtm
			//{
			//	'type': 'filters.range',
			//	'limits' : 'Classification[2:2]'
			//},

			// writer
			fprintf(pFile, "	{\n");
			fprintf(pFile, "		\"type\": \"writers.gdal\", \n");
			fprintf(pFile, "		\"resolution\": %0.2f, \n", resolution);
			fprintf(pFile, "		\"radius\": %0.17f, \n", tile.radius);
			fprintf(pFile, "		\"filename\": \"%s\", \n", tile.filename.c_str());
			fprintf(pFile, "		\"output_type\": \"%s\", \n", output_type.c_str());
			fprintf(pFile, "		\"data_type\": \"float\", \n");
			// format is ([minx, maxx],[miny,maxy])
			fprintf(pFile, "		\"bounds\": \"([%0.15f,%0.15f],[%0.15f,%0.15f])\"\n", tile.minx, tile.maxx, tile.miny, tile.maxy);
			fprintf(pFile, "	}\n");
			fprintf(pFile, "]\n");

			fclose(pFile);
		}

		// run pipeline
		QStringList args;
		args.push_back("pipeline");
		args.push_back("-i");
		args.push_back(pipe_file.c_str());
		AeroLib::RunProgram(tree.prog_pdal, args);
	}

	XString output_file = dem_type + ".tif";
	XString output_path = XString::CombinePath(outdir, output_file);

	// Verify tile results
	//for t in tiles:
	//    if not os.path.exists(t['filename']):
	//        raise Exception("Error creating %s, %s failed to be created" % (output_file, t['filename']))

	// Create virtual raster
	XString tiles_vrt_path = XString::CombinePath(outdir, "tiles.vrt");
	XString tiles_file_list = XString::CombinePath(outdir, "tiles_list.txt");
	FILE* pFile = fopen(tiles_file_list.c_str(), "wt");
	if (pFile)
	{
		for (TileType tile : tiles)
			fprintf(pFile, "%s\n", tile.filename.c_str());
		fclose(pFile);
	}

	QStringList args;
	args.push_back("-input_file_list");
	args.push_back(tiles_file_list.c_str());
	args.push_back(tiles_vrt_path.c_str());
	AeroLib::RunProgram(tree.prog_gdal_buildvrt, args);
	//run('gdalbuildvrt -input_file_list "%s" "%s" ' % (tiles_file_list, tiles_vrt_path))

	XString merged_vrt_path = XString::CombinePath(outdir, "merged.vrt");
	XString geotiff_tmp_path = XString::CombinePath(outdir, "tiles.tmp.tif");
	XString geotiff_small_path = XString::CombinePath(outdir, "tiles.small.tif");
	XString geotiff_small_filled_path = XString::CombinePath(outdir, "tiles.small_filled.tif");
	XString geotiff_path = XString::CombinePath(outdir, "tiles.tif");

	// Build GeoTIFF

	if (gapfill)
	{
		// Sometimes gdal_fillnodata.py behaves strangely 
		// when reading data directly from a .VRT
		// so we need to convert to GeoTIFF first.
		args.clear();
		args.push_back("-co");
		if (max_workers > 0)
			args.push_back(XString::Format("NUM_THREADS=%d", max_workers).c_str());
		else
			args.push_back("NUM_THREADS=ALL_CPUS");
		args.push_back("-co");
		args.push_back("BIGTIFF=IF_SAFER");
		args.push_back("--config");
		args.push_back("GDAL_CACHEMAX");
		args.push_back("34.85%");
		args.push_back(tiles_vrt_path.c_str());
		args.push_back(geotiff_tmp_path.c_str());
		AeroLib::RunProgram(tree.prog_gdal_translate, args);
		// cmd: gdal_translate 
		//			-co NUM_THREADS=16 -co BIGTIFF=IF_SAFER --config GDAL_CACHEMAX 34.85%
		//          "d:\test\odm_meshing\tmp\tiles.vrt" 
		//          "d:\test\odm_meshing\tmp\tiles.tmp.tif"

		// Scale to 10% size
		args.clear();
		args.push_back("-co");
		if (max_workers > 0)
			args.push_back(XString::Format("NUM_THREADS=%d", max_workers).c_str());
		else
			args.push_back("NUM_THREADS=ALL_CPUS");
		args.push_back("-co");
		args.push_back("BIGTIFF=IF_SAFER");
		args.push_back("--config");
		args.push_back("GDAL_CACHEMAX");
		args.push_back("34.85%");
		args.push_back("-outsize");
		args.push_back("10%");
		args.push_back("0");
		args.push_back(geotiff_tmp_path.c_str());
		args.push_back(geotiff_small_path.c_str());
		AeroLib::RunProgram(tree.prog_gdal_translate, args);
		// cmd: gdal_translate -co NUM_THREADS=16 -co BIGTIFF=IF_SAFER --config GDAL_CACHEMAX 34.85% 
		//          -outsize 10% 0 
		//          "d:\test\odm_meshing\tmp\tiles.tmp.tif" 
		//          "d:\test\odm_meshing\tmp\tiles.small.tif"

		// Fill scaled
		gdal_fillnodata(1, "GTiff", geotiff_small_path, geotiff_small_filled_path);

		// Merge filled scaled DEM with unfilled DEM using bilinear interpolation
		args.clear();
		args.push_back("-resolution");
		args.push_back("highest");
		args.push_back("-r");
		args.push_back("bilinear");
		args.push_back(merged_vrt_path.c_str());
		args.push_back(geotiff_small_filled_path.c_str());
		args.push_back(geotiff_tmp_path.c_str());
		AeroLib::RunProgram(tree.prog_gdal_buildvrt, args);
		// cmd: gdalbuildvrt 
		//          -resolution highest -r bilinear 
		//          "d:\test\odm_meshing\tmp\merged.vrt" 
		//          "d:\test\odm_meshing\tmp\tiles.small_filled.tif" 
		//          "d:\test\odm_meshing\tmp\tiles.tmp.tif"
		args.clear();
		args.push_back("-co");
		if (max_workers > 0)
			args.push_back(XString::Format("NUM_THREADS=%d", max_workers).c_str());
		else
			args.push_back("NUM_THREADS=ALL_CPUS");
		args.push_back("-co");
		args.push_back("TILED=YES");
		args.push_back("-co");
		args.push_back("BIGTIFF=IF_SAFER");
		args.push_back("-co");
		args.push_back("COMPRESS=DEFLATE");
		args.push_back("--config");
		args.push_back("GDAL_CACHEMAX");
		args.push_back("34.85%");
		args.push_back(merged_vrt_path.c_str());
		args.push_back(geotiff_path.c_str());
		AeroLib::RunProgram(tree.prog_gdal_translate, args);
		// cmd: gdal_translate 
		//          -co NUM_THREADS=16 -co TILED=YES -co BIGTIFF=IF_SAFER -co COMPRESS=DEFLATE
		//          --config GDAL_CACHEMAX 34.85% 
		//          "d:\test\odm_meshing\tmp\merged.vrt" 
		//          "d:\test\odm_meshing\tmp\tiles.tif"
	}
	else
	{
		QStringList args;
		args.push_back("-co");
		if (max_workers > 0)
			args.push_back(XString::Format("NUM_THREADS=%d", max_workers).c_str());
		else
			args.push_back("NUM_THREADS=ALL_CPUS");
		args.push_back("-co");
		args.push_back("TILED=YES");
		args.push_back("-co");
		args.push_back("BIGTIFF=IF_SAFER");
		args.push_back("-co");
		args.push_back("COMPRESS=DEFLATE");
		args.push_back("--config");
		args.push_back("GDAL_CACHEMAX");
		args.push_back("34.85%");               //TODO: get_max_memory()
		args.push_back(tiles_vrt_path.c_str());
		args.push_back(geotiff_path.c_str());
		AeroLib::RunProgram(tree.prog_gdal_translate, args);
		// run('gdal_translate '
		//            '-co NUM_THREADS={threads} '
		//            '-co TILED=YES '
		//            '-co BIGTIFF=IF_SAFER '
		//            '-co COMPRESS=DEFLATE '
		//            '--config GDAL_CACHEMAX {max_memory}% '
		//            '"{tiles_vrt}" "{geotiff}"'.format(**kwargs))
	}

	//TODO:
	apply_smoothing = false;
	if (apply_smoothing)
	{
		// median_smoothing(geotiff_path, output_path, num_workers=max_workers)
		// os.remove(geotiff_path)
	}
	else
	{
		AeroLib::Replace(geotiff_path, output_path);
	}

	if (AeroLib::FileExists(geotiff_tmp_path))
	{
		//if (keep_unfilled_copy == false)
		//	AeroLib::RemoveFile(geotiff_tmp_path);
		//else
		//	AeroLib::Replace(geotiff_tmp_path, AeroLib::related_file_path(output_path, ".unfilled"));
	}

	//for cleanup_file in [tiles_vrt_path, tiles_file_list, merged_vrt_path, geotiff_small_path, geotiff_small_filled_path]:
	//    if os.path.exists(cleanup_file): os.remove(cleanup_file)
	//for t in tiles:
	//    if os.path.exists(t['filename']): os.remove(t['filename'])
}

std::vector<double> DEM::get_dem_radius_steps(XString stats_file, int steps, double resolution, double multiplier)
{
	std::vector<double> radius_steps;

	double spacing = PointCloud::get_spacing(stats_file, resolution);

	radius_steps.push_back(spacing * multiplier);
	for (int i = 1; i < steps; ++i)
		radius_steps.push_back(radius_steps[i - 1] * sqrt(2.0));

	return radius_steps;
}

//def compute_euclidean_map(geotiff_path, output_path, overwrite=False):
//    if not os.path.exists(geotiff_path):
//        Logger::Write(__FUNCTION__, "Cannot compute euclidean map (file does not exist: %s)" % geotiff_path)
//        return
//
//    nodata = -9999
//    with rasterio.open(geotiff_path) as f:
//        nodata = f.nodatavals[0]
//
//    if not os.path.exists(output_path) or overwrite:
//        Logger::Write(__FUNCTION__, "Computing euclidean distance: %s" % output_path)
//
//        if gdal_proximity is not None:
//            try:
//                gdal_proximity(['gdal_proximity.py', geotiff_path, output_path, '-values', str(nodata)])
//            except Exception as e:
//                Logger::Write(__FUNCTION__, "Cannot compute euclidean distance: %s" % str(e))
//
//            if os.path.exists(output_path):
//                return output_path
//            else
//                Logger::Write(__FUNCTION__, "Cannot compute euclidean distance file: %s" % output_path)
//        else
//            Logger::Write(__FUNCTION__, "Cannot compute euclidean map, gdal_proximity is missing")
//
//    else
//        Logger::Write(__FUNCTION__, "Found a euclidean distance map: %s" % output_path)
//        return output_path

//def median_smoothing(geotiff_path, output_path, smoothing_iterations=1, window_size=512, num_workers=1):
//    """ Apply median smoothing """
//    start = datetime.now()
//
//    if not os.path.exists(geotiff_path):
//        raise Exception('File %s does not exist!' % geotiff_path)
//
//    # Prepare temporary files
//    folder_path, output_filename = os.path.split(output_path)
//    basename, ext = os.path.splitext(output_filename)
//
//    output_dirty_in = os.path.join(folder_path, "{}.dirty_1{}".format(basename, ext))
//    output_dirty_out = os.path.join(folder_path, "{}.dirty_2{}".format(basename, ext))
//
//    Logger::Write(__FUNCTION__, 'Starting smoothing...')
//
//    with rasterio.open(geotiff_path, num_threads=num_workers) as img, rasterio.open(output_dirty_in, "w+", BIGTIFF="IF_SAFER", num_threads=num_workers, **img.profile) as imgout, rasterio.open(output_dirty_out, "w+", BIGTIFF="IF_SAFER", num_threads=num_workers, **img.profile) as imgout2:
//        nodata = img.nodatavals[0]
//        dtype = img.dtypes[0]
//        shape = img.shape
//        for i in range(smoothing_iterations):
//            Logger::Write(__FUNCTION__, "Smoothing iteration %s" % str(i + 1))
//            rows, cols = numpy.meshgrid(numpy.arange(0, shape[0], window_size), numpy.arange(0, shape[1], window_size))
//            rows = rows.flatten()
//            cols = cols.flatten()
//            rows_end = numpy.minimum(rows + window_size, shape[0])
//            cols_end= numpy.minimum(cols + window_size, shape[1])
//            windows = numpy.dstack((rows, cols, rows_end, cols_end)).reshape(-1, 4)
//
//            filter = functools.partial(ndimage.median_filter, size=9, output=dtype, mode='nearest')
//
//            # We cannot read/write to the same file from multiple threads without causing race conditions.
//            # To safely read/write from multiple threads, we use a lock to protect the DatasetReader/Writer.
//            read_lock = threading.Lock()
//            write_lock = threading.Lock()
//
//            # threading backend and GIL released filter are important for memory efficiency and multi-core performance
//            Parallel(n_jobs=num_workers, backend='threading')(delayed(window_filter_2d)(img, imgout, nodata , window, 9, filter, read_lock, write_lock) for window in windows)
//
//            # Between each iteration we swap the input and output temporary files
//            #img_in, img_out = img_out, img_in
//            if (i == 0):
//                img = imgout
//                imgout = imgout2
//            else
//                img, imgout = imgout, img
//
//    # If the number of iterations was even, we need to swap temporary files
//    if (smoothing_iterations % 2 != 0):
//        output_dirty_in, output_dirty_out = output_dirty_out, output_dirty_in
//
//    # Cleaning temporary files
//    if os.path.exists(output_dirty_out):
//        os.replace(output_dirty_out, output_path)
//    if os.path.exists(output_dirty_in):
//        os.remove(output_dirty_in)
//
//    Logger::Write(__FUNCTION__, 'Completed smoothing to create %s in %s' % (output_path, datetime.now() - start))
//    return output_path

//def window_filter_2d(img, imgout, nodata, window, kernel_size, filter, read_lock, write_lock):
//    """
//    Apply a filter to dem within a window, expects to work with kernal based filters
//
//    :param img: path to the geotiff to filter
//    :param imgout: path to write the giltered geotiff to. It can be the same as img to do the modification in place.
//    :param window: the window to apply the filter, should be a list contains row start, col_start, row_end, col_end
//    :param kernel_size: the size of the kernel for the filter, works with odd numbers, need to test if it works with even numbers
//    :param filter: the filter function which takes a 2d array as input and filter results as output.
//    :param read_lock: threading lock for the read operation
//    :param write_lock: threading lock for the write operation
//    """
//    shape = img.shape[:2]
//    if window[0] < 0 or window[1] < 0 or window[2] > shape[0] or window[3] > shape[1]:
//        raise Exception('Window is out of bounds')
//    expanded_window = [ max(0, window[0] - kernel_size // 2), max(0, window[1] - kernel_size // 2), min(shape[0], window[2] + kernel_size // 2), min(shape[1], window[3] + kernel_size // 2) ]
//
//    # Read input window
//    width = expanded_window[3] - expanded_window[1]
//    height = expanded_window[2] - expanded_window[0]
//    rasterio_window = rasterio.windows.Window(col_off=expanded_window[1], row_off=expanded_window[0], width=width, height=height)
//    with read_lock:
//        win_arr = img.read(indexes=1, window=rasterio_window)
//
//    # Should have a better way to handle nodata, similar to the way the filter algorithms handle the border (reflection, nearest, interpolation, etc).
//    # For now will follow the old approach to guarantee identical outputs
//    nodata_locs = win_arr == nodata
//    win_arr = filter(win_arr)
//    win_arr[nodata_locs] = nodata
//    win_arr = win_arr[window[0] - expanded_window[0] : window[2] - expanded_window[0], window[1] - expanded_window[1] : window[3] - expanded_window[1]]
//
//    # Write output window
//    width = window[3] - window[1]
//    height = window[2] - window[0]
//    rasterio_window = rasterio.windows.Window(col_off=window[1], row_off=window[0], width=width, height=height)
//    with write_lock:
//        imgout.write(win_arr, indexes=1, window=rasterio_window)

//def classify(lasFile, scalar, slope, threshold, window):
//    start = datetime.now()
//
//    try:
//        pdal.run_pdaltranslate_smrf(lasFile, lasFile, scalar, slope, threshold, window)
//    except:
//        Logger::Write(__FUNCTION__, "Error creating classified file %s" % lasFile)
//
//    Logger::Write(__FUNCTION__, 'Created %s in %s' % (lasFile, datetime.now() - start))
//    return lasFile

//def rectify(lasFile, reclassify_threshold=5, min_area=750, min_points=500):
//    start = datetime.now()
//
//    try:
//
//        Logger::Write(__FUNCTION__, "Rectifying {} using with [reclassify threshold: {}, min area: {}, min points: {}]".format(lasFile, reclassify_threshold, min_area, min_points))
//        run_rectification(
//            input=lasFile, output=lasFile, \
//            reclassify_plan='median', reclassify_threshold=reclassify_threshold, \
//            extend_plan='surrounding', extend_grid_distance=5, \
//            min_area=min_area, min_points=min_points)
//
//        Logger::Write(__FUNCTION__, 'Created %s in %s' % (lasFile, datetime.now() - start))
//    except Exception as e:
//        Logger::Write(__FUNCTION__, "Error rectifying ground in file %s: %s" % (lasFile, str(e)))
//
//    return lasFile
//
//error = None
//

int DEM::gdal_fillnodata(int src_band, XString out_format, XString src_filename, XString dst_filename)
{
	// The gdal_fillnodata() in odm was a thin wrapper around the python gdal_fillnodata.py - which itself is
	// a thin wrapper around the gdal function, GDALFillNodata(). Here, it's just set up & called directly.
	//

	int status = 0;

	// require separate src & dest file names
	if (src_filename.GetLength() == 0)
		return -1;
	if (dst_filename.GetLength() == 0)
		return -1;

	if (AeroLib::FileExists(src_filename) == false)
	{
		Logger::Write(__FUNCTION__, "Source file not found: '%s'", src_filename.c_str());
		return -1;
	}

	GDALDriver* pDriver = static_cast<GDALDriver*>(GDALGetDriverByName(out_format.c_str()));
	if (pDriver == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to create gdal driver: '%s'", out_format.c_str());
		return -1;
	}

	int smoothing_iterations = 0;
	double max_distance = 100.0;
	XString mask = "default";

	// Open source file

	GDALDataset* src_ds = (GDALDataset*)GDALOpen(src_filename.c_str(), GA_ReadOnly);
	if (src_ds == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open source file: '%s'", src_filename.c_str());
		return -1;
	}

	GDALRasterBand* srcband = src_ds->GetRasterBand(src_band);
	if (srcband == nullptr)
	{
		GDALClose(src_ds);

		Logger::Write(__FUNCTION__, "Unable to read source band: %d", src_band);
		return -1;
	}

	GDALRasterBand* maskband = nullptr;
	if (mask == "default")
		maskband = srcband->GetMaskBand();
	else if (mask == "none")
		maskband = nullptr;
	else
	{
		assert(false);
		// mask_ds = gdal.Open(mask)
		// maskband = mask_ds.GetRasterBand(1)
	}

	// Create output file

	// example of how to pass create options
	char** pszOptions = nullptr;
	pszOptions = CSLSetNameValue(pszOptions, "NUM_THREADS", "16");      //TODO:
	pszOptions = CSLSetNameValue(pszOptions, "BIGTIFF", "IF_SAFER");

	GDALDataset* dst_ds = pDriver->Create(dst_filename, src_ds->GetRasterXSize(), src_ds->GetRasterYSize(), 1, srcband->GetRasterDataType(), pszOptions);
	if (dst_ds == nullptr)
	{
		status = -1;
		Logger::Write(__FUNCTION__, "Unable to create GDALDataset for '%s'", dst_filename.c_str());
	}
	else
	{
		XString wkt = src_ds->GetProjectionRef();
		if (wkt.GetLength() > 0)
			dst_ds->SetProjection(wkt);

		double geoTransform[6] = { 0.0 };
		CPLErr err = src_ds->GetGeoTransform(geoTransform);
		if (err == CPLErr::CE_None)
			dst_ds->SetGeoTransform(geoTransform);

		GDALRasterBand* dstband = dst_ds->GetRasterBand(1);

		int buf_type = srcband->GetRasterDataType();
		assert(buf_type == GDT_Float32);

		// copy srcband to destband

		int nXSize = srcband->GetXSize();
		int nYSize = srcband->GetYSize();
		void* pData = CPLMalloc(nXSize * nYSize * GDALGetDataTypeSize(srcband->GetRasterDataType()) / 8);

		err = srcband->RasterIO(GF_Read, 0, 0, nXSize, nYSize, pData, nXSize, nYSize, srcband->GetRasterDataType(), 0, 0);
		if (err != CE_None)
		{
			Logger::Write(__FUNCTION__, "srcband->RasterIO(GF_Read) failed, err = %d", (int)err);
		}
		else
		{
			err = dstband->RasterIO(GF_Write, 0, 0, nXSize, nYSize, pData, nXSize, nYSize, dstband->GetRasterDataType(), 0, 0);
			if (err != CE_None)
			{
				Logger::Write(__FUNCTION__, "dstband->RasterIO(GF_Write) failed, err = %d", (int)err);
			}
		}

		CPLFree(pData);

		double ndv = srcband->GetNoDataValue();
		dstband->SetNoDataValue(ndv);

		GDALColorInterp color_interp = srcband->GetColorInterpretation();
		dstband->SetColorInterpretation(color_interp);
		if (color_interp == GDALColorInterp::GCI_PaletteIndex)
		{
			GDALColorTable* color_table = srcband->GetColorTable();
			dstband->SetColorTable(color_table);
		}

		// Invoke algorithm.

		err = GDALFillNodata(dstband, maskband, max_distance, 0, smoothing_iterations, nullptr, nullptr, nullptr);
		if (err != CPLErr::CE_None)
		{
			status = -1;
			Logger::Write(__FUNCTION__, "GDALFillNodata() call failed.");
		}
	}

	GDALClose(src_ds);
	GDALClose(dst_ds);
	//GDALClose(mask_ds);

	return status;
}
