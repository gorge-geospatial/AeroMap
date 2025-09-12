// StageOrtho.cpp
// Generate orthophoto.
//

#include "Gsd.h"
#include "Orthophoto.h"
#include "StageOrtho.h"

int StageOrtho::Run()
{
	// Inputs:
	// Outputs:
	//		+ orthophoto
	//			orthophoto.tif
	//			orthophoto_corners.txt
	//			orthophoto_log.txt
	//

	int status = 0;

	GetApp()->LogWrite("Orthophoto...");
	BenchmarkStart();
	LogStage();

	// define paths and create working directories
	AeroLib::CreateFolder(tree.ortho_path);

	if (arg.skip_orthophoto)
	{
		Logger::Write(__FUNCTION__, "--skip-orthophoto is set, no orthophoto will be generated");
		return -1;
	}

	if ((AeroLib::FileExists(tree.orthophoto_tif) == false) || Rerun())
	{
		AeroLib::Georef georef = AeroLib::ReadGeoref();

		bool has_gcp = GetReconstruction()->has_gcp();
		double resolution = Gsd::cap_resolution(arg.ortho_resolution, tree.opensfm_reconstruction, 0.10, 1.0, arg.ignore_gsd, georef.is_valid && arg.ignore_gsd, has_gcp);
		// why this calc?
		resolution = 1.0 / (resolution / 100.0);

		std::vector<XString> models;

		XString base_dir = tree.texture_path_25d;
		if (arg.use_3dmesh)
			base_dir = tree.texture_path;

		XString model_file = tree.textured_model_obj;
		double in_paint = -1.0;
		XString bands;

		if (GetReconstruction()->is_multi())
		{
			for (auto band : GetReconstruction()->m_mc)
			{
				//primary = band['name'] == get_primary_band_name(reconstruction.multi_camera, arg.primary_band)
				//subdir = ""
				//if not primary:
				//    subdir = band['name'].lower()
				//models.append(os.path.join(base_dir, subdir, model_file))
			}
			//kwargs['bands'] = '-bands %s' % (','.join([double_quote(b['name']) for b in reconstruction.multi_camera]))

		    // If a RGB band is present,
		    // use bit depth of the first non-RGB band
		    //depth_idx = None
		    //all_bands = [b['name'].lower() for b in reconstruction.multi_camera]
		    //for b in ['rgb', 'redgreenblue']:
		    //    if b in all_bands:
		    //        for idx in range(len(all_bands)):
		    //            if all_bands[idx] != b:
		    //                depth_idx = idx
		    //                break
		    //        break

		    //if depth_idx is not None:
		    //    kwargs['depth_idx'] = '-outputDepthIdx %s' % depth_idx
		}
		else
		{
			models.push_back(XString::CombinePath(base_dir, model_file));

		    // Perform edge inpainting on georeferenced RGB datasets
			if (georef.is_valid)
				in_paint = 1.0;

		    // Thermal dataset with single band
			if (GetReconstruction()->GetImageList()[0]->GetBandName().CompareNoCase("LWIR"))
				bands = "lwir";
		}

		// run odm_orthophoto
		QStringList args;
		args.push_back("-inputFiles");
		for (XString model : models)
			args.push_back(model.c_str());
		args.push_back("-logFile");
		args.push_back(tree.orthophoto_log.c_str());
		args.push_back("-outputFile");
		if (georef.is_valid)
			args.push_back(tree.orthophoto_tif.c_str());  // Render directly to final file
		else
			args.push_back(tree.orthophoto_render.c_str());
		args.push_back("-resolution");
		args.push_back(XString::Format("%0.1f", resolution).c_str());
		args.push_back("-verbose");
		args.push_back("-outputCornerFile");
		args.push_back(tree.orthophoto_corners.c_str());
		if (in_paint > 0.0)
		{
			args.push_back("-inpaintThreshold");
			args.push_back(XString::Format("%0.1f", in_paint).c_str());
		}
		if (georef.is_valid)
		{
			args.push_back("-utm_north_offset");
			args.push_back(XString::Format("%0.1f", georef.y).c_str());
			args.push_back("-utm_east_offset");
			args.push_back(XString::Format("%0.1f", georef.x).c_str());
			args.push_back("-a_srs");
			args.push_back(XString::Format("+proj=utm +zone=%d +datum=WGS84 +units=m +no_defs +type=crs", georef.utm_zone).c_str());
		}
		if (bands.IsEmpty() == false)
		{
			args.push_back("-bands");
			args.push_back(bands.c_str());
		}
		args.push_back("-co");
		args.push_back("TILED=YES");
		args.push_back("-co");
		args.push_back("COMPRESS=DEFLATE");
		args.push_back("-co");
		args.push_back("PREDICTOR=2");
		args.push_back("-co");
		args.push_back("BIGTIFF=IF_SAFER");
		args.push_back("-co");
		args.push_back("BLOCKXSIZE=512");
		args.push_back("-co");
		args.push_back("BLOCKYSIZE=512");
		args.push_back("-co");
		args.push_back(XString::Format("NUM_THREADS=%d", arg.max_concurrency).c_str());
		args.push_back("--config");
		args.push_back("GDAL_CACHEMAX");
		args.push_back("11983939584.0");		//TODO: (get_max_memory_mb() * 1024 * 1024)
		AeroLib::RunProgram(tree.prog_odm_orthophoto, args);
		// cmd: odm_orthophoto 
		//			-inputFiles "d:/test/texture_25d/textured_model_geo.obj" 
		//			-logFile "d:/test/odm_orthophoto/odm_orthophoto_log.txt"
		//			-outputFile "d:/test/odm_orthophoto/odm_orthophoto.tif" 
		//			-resolution 20.0 
		//			-verbose
		//			-outputCornerFile "d:/test/odm_orthophoto/odm_orthophoto_corners.txt"   
		//			-inpaintThreshold 1.0 
		//			-utm_north_offset 5157982.0 -utm_east_offset 322263.0 
		//			-a_srs "+proj=utm +zone=32 +datum=WGS84 +units=m +no_defs +type=crs" 
		//			-co TILED=YES -co COMPRESS=DEFLATE -co PREDICTOR=2 -co BIGTIFF=IF_SAFER -co BLOCKXSIZE=512 -co BLOCKYSIZE=512 -co NUM_THREADS=16
		//			--config GDAL_CACHEMAX 11983939584.0 

		// Create georeferenced GeoTiff
		if (georef.is_valid)
		{
			XString bounds_file_path = XString::CombinePath(tree.georef_path, "georeferenced_model.bounds.gpkg");

			// Cutline computation, before cropping
			// We want to use the full orthophoto, not the cropped one.
			if (arg.orthophoto_cutline)
			{
			    //cutline_file = os.path.join(tree.odm_orthophoto, "cutline.gpkg")

			    //compute_cutline(tree.orthophoto_tif,
			    //                bounds_file_path,
			    //                cutline_file,
			    //                arg.max_concurrency,
			    //                scale=0.25)

			    //orthophoto.compute_mask_raster(tree.orthophoto_tif, cutline_file,
			    //                        os.path.join(tree.odm_orthophoto, "odm_orthophoto_cut.tif"),
			    //                        blend_distance=20, only_max_coords_feature=True)
			}

			Orthophoto::post_orthophoto_steps(bounds_file_path, tree.orthophoto_tif, tree.orthophoto_tiles);

			// Generate feathered orthophoto also
			if (arg.orthophoto_cutline)
			{
				//orthophoto.feather_raster(tree.orthophoto_tif,
				//        os.path.join(tree.odm_orthophoto, "odm_orthophoto_feathered.tif"),
				//        blend_distance=20)
			}
		}
		else
		{
			if (AeroLib::FileExists(tree.orthophoto_render))
			{
				//    pseudogeo.add_pseudo_georeferencing(tree.orthophoto_render)
				//    Logger::Write(__FUNCTION__, "Renaming %s --> %s" % (tree.orthophoto_render, tree.orthophoto_tif))
				//    os.replace(tree.orthophoto_render, tree.orthophoto_tif)
			}
			else
			{
				Logger::Write(__FUNCTION__, "Could not generate an orthophoto (it did not render)");
			}
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing orthophoto: '%s'", tree.orthophoto_tif.c_str());
	}

	//if io.file_exists(tree.orthophoto_render):
	//    os.remove(tree.orthophoto_render)

	GetProject().Update();		// trigger ProjectWindow update
	BenchmarkStop();

	return status;
}

StageOrtho::StageOrtho(DroneProc* pDroneProc)
	: Stage(pDroneProc)
{
	m_stage_id = Id::Orthophoto;
}

StageOrtho::~StageOrtho()
{

}
