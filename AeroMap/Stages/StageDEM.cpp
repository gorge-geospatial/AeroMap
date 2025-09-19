// StageDEM.cpp
// Generate DTM/DSM.
//

#include <opencv2/opencv.hpp>	// OpenCV

#include "Cropper.h"
#include "DEM.h"
#include "Gsd.h"
#include "Terrain.h"
#include "StageDEM.h"

int StageDEM::Run()
{
	// Inputs:
	//		+ georeference
	//			georeferenced_model.laz
	// Outputs:
	//		+ dem
	//			dtm.tif
	//			dsm.tif
	//			dtm.dat
	//			dsm.dat
	//

	int status = 0;

	GetApp()->LogWrite("DEM...");
	BenchmarkStart();
	LogStage();

	XString dem_input = tree.georef_model_laz;
	bool pc_model_found = AeroLib::FileExists(dem_input);
	bool ignore_resolution = false;
	bool pseudo_georeference = false;

	AeroLib::Georef georef = AeroLib::ReadGeoref();
	if (georef.is_valid == false)
	{
		Logger::Write(__FUNCTION__, "Not georeferenced, using ungeoreferenced point cloud.");
		ignore_resolution = true;
		pseudo_georeference = true;
	}

	bool reconstruction_has_gcp = GetReconstruction()->has_gcp();
	double resolution = Gsd::cap_resolution(
		arg.dem_resolution,						// resolution
		tree.opensfm_reconstruction,			// reconstruction json
		0.1,									// error estimate
		1.0,									// gsd scaling
		arg.ignore_gsd,							// ignore gsd
		ignore_resolution && arg.ignore_gsd,	// ignore resolution
		reconstruction_has_gcp);				// has ground control points
		
	// define paths and create working directories
	AeroLib::CreateFolder(tree.dem_path);

	if (arg.pc_classify && pc_model_found)
	{
		//pc_classify_marker = XString::CombinePath(odm_dem_root, 'pc_classify_done.txt')

		//if not io.file_exists(pc_classify_marker) or self.rerun():
		//    Logger::Write(__FUNCTION__, "Classifying {} using Simple Morphological Filter (1/2)".format(dem_input))
		//    commands.classify(dem_input,
		//                        arg.smrf_scalar,
		//                        arg.smrf_slope,
		//                        arg.smrf_threshold,
		//                        arg.smrf_window
		//                    )

		//    Logger::Write(__FUNCTION__, "Classifying {} using OpenPointClass (2/2)".format(dem_input))
		//    classify(dem_input, arg.max_concurrency)

		//    with open(pc_classify_marker, 'w') as f:
		//        f.write('Classify: smrf\n')
		//        f.write('Scalar: {}\n'.format(arg.smrf_scalar))
		//        f.write('Slope: {}\n'.format(arg.smrf_slope))
		//        f.write('Threshold: {}\n'.format(arg.smrf_threshold))
		//        f.write('Window: {}\n'.format(arg.smrf_window))
	}

	//if arg.pc_rectify:
	//    commands.rectify(dem_input)

	// Do we need to process anything here?
	if ((arg.dsm || arg.dtm) && pc_model_found)
	{
		XString dsm_output_filename = XString::CombinePath(tree.dem_path, "dsm.tif");
		XString dtm_output_filename = XString::CombinePath(tree.dem_path, "dtm.tif");

		if ((arg.dtm && AeroLib::FileExists(dtm_output_filename) == false) ||
			(arg.dsm && AeroLib::FileExists(dsm_output_filename) == false) ||
			Rerun())
		{
			std::vector<XString> products;

			if (arg.dsm || (arg.dtm && arg.dem_euclidean_map))
				products.push_back("dsm");
			if (arg.dtm)
				products.push_back("dtm");

			std::vector<double> radius_steps = DEM::get_dem_radius_steps(tree.filtered_point_cloud_stats, arg.dem_gapfill_steps, resolution);

			for (XString product : products)
			{
				XString output_type = "max";
				if (product == "dtm")
					output_type = "idw";

				int max_tiles = GetProject().GetImageCount() / 2;

				DEM::create_dem(
					dem_input,                      // input point cloud
					product,                        // dem type
					radius_steps,
					output_type,
					arg.dem_gapfill_steps > 0,      // gap fill
					tree.dem_path,                   // output directory
					resolution / 100.0,             // TODO: why /100 here?
					arg.max_concurrency,
					4096,                           // max tile size
					arg.dem_decimation,
					arg.dem_euclidean_map,          // keep unfilled copy
					true,                           // apply smoothing
					max_tiles);

				XString dem_geotiff_path = XString::CombinePath(tree.dem_path, XString::Format("%s.tif", product.c_str()));
				XString bounds_file_path = XString::CombinePath(tree.georef_path, "georeferenced_model.bounds.gpkg");

				if (arg.crop > 0.0 || arg.boundary)
				{
					// Crop DEM
					Cropper::crop(bounds_file_path, dem_geotiff_path/*, utils.get_dem_vars(args)*/, true);
				}

				if (arg.dem_euclidean_map)
				{
					XString unfilled_dem_path = AeroLib::related_file_path(dem_geotiff_path, "", ".unfilled");

					if (arg.crop > 0.0 || arg.boundary)
					{
						// Crop unfilled DEM
						Cropper::crop(bounds_file_path, unfilled_dem_path/*, utils.get_dem_vars(args)*/, true);
					}
					//    commands.compute_euclidean_map(unfilled_dem_path,
					//                        io.related_file_path(dem_geotiff_path, postfix=".euclideand"),
					//                        overwrite=True)
				}
				//if pseudo_georeference:
				//    pseudogeo.add_pseudo_georeferencing(dem_geotiff_path)

				//if arg.tiles:
				//    generate_dem_tiles(dem_geotiff_path, tree.path("%s_tiles" % product), arg.max_concurrency)

				//if arg.cog:
				//    convert_to_cogeo(dem_geotiff_path, max_workers=arg.max_concurrency)

				CreateTerrainModel(product);
			}
		}
		else
		{
			Logger::Write(__FUNCTION__, "Found existing outputs in: %s", tree.dem_path.c_str());
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "DEM will not be generated");
	}

	GetProject().Update();		// trigger ProjectWindow update
	BenchmarkStop();

	return status;
}

void StageDEM::CreateTerrainModel(XString model_type)
{
	// Create an AeroMap-format terrain model.
	//
	// Inputs:
	//		model_type = dtm/dsm
	//

	XString input_file = XString::CombinePath(tree.dem_path, XString::Format("%s.tif", model_type.c_str()));
	if (AeroLib::FileExists(input_file) == false)
	{
		Logger::Write(__FUNCTION__, "Input file not found: '%s'", input_file.c_str());
		return;
	}

	cv::Mat image = cv::imread(input_file.c_str(), cv::IMREAD_UNCHANGED);
	if ((image.rows == 0) || (image.cols == 0))
	{
		Logger::Write(__FUNCTION__, "Unable to load nput file: '%s'", input_file.c_str());
		return;
	}

	// reduce size so largest dimension is below 1k
	int w = image.cols;
	int h = image.rows;
	int dec = 1;
	while (std::max(w, h) >= 1000)
	{
		w /= 2;
		h /= 2;
		dec *= 2;		// decimation factor
	}

	XString output_file = (model_type == "dtm" ? tree.dtm_file : tree.dsm_file);
//TODO:
//what to use for pitch?
	if (Terrain::Create(output_file.c_str(), h, w, 10.0))
	{
		Terrain terrain(output_file.c_str());
		for (int r = 0; r < h; ++r)
		{
			for (int c = 0; c < w; ++c)
			{
				double height = image.at<float>(r * dec, c * dec);
				terrain.SetHeight(h - r - 1, c, height);
			}
		}
		
		terrain.SaveData();
	}
}

StageDEM::StageDEM(DroneProc* pDroneProc)
	: Stage(pDroneProc)
{
	m_stage_id = Id::DEM;
}

StageDEM::~StageDEM()
{

}
