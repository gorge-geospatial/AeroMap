// StageDataset.cpp
// Load dataset.
//

#include "StageDataset.h"

int StageDataset::Run()
{
	// Inputs:
	// Outputs:
	//		+ georeference
	//			coords.txt
	//			proj.txt
	//		benchmark.txt
	//		images.json
	//		img_list.txt
	//

	int status = 0;

	GetApp()->LogWrite("Load dataset...");
	BenchmarkStart();
	LogStage();

	InitGeoref();

	WriteImageListText();
	WriteImageListJson();

	// Create reconstruction object
	GetProject().CreateReconstruction();

	if (tree.georef_gcp && (arg.use_exif == false))
	{
		//	reconstruction.georeference_with_gcp(tree.georeference_gcp,
		//											tree.georeference_coords,
		//											tree.georeference_gcp_utm,
		//											tree.georeference_model_txt_geo,
		//											rerun=self.rerun())
	}
	else
	{
		//TODO:
		// this is already done here, remove?
		//	reconstruction.georeference_with_gps(tree.dataset_raw,
		//											tree.georeference_coords,
		//											tree.georeference_model_txt_geo,
		//											rerun=self.rerun())
	}

	//reconstruction.save_proj_srs(os.path.join(tree.georeference, tree.georeference_proj))
	//outputs['reconstruction'] = reconstruction

	// Try to load boundaries
	if (arg.boundary)
	{
		//	if reconstruction.is_georeferenced():
		//		outputs['boundary'] = boundary.load_boundary(args.boundary, reconstruction.get_proj_srs())
		//	else:
		//		args.boundary = None
		//		Logger::Write(__FUNCTION__, "Reconstruction is not georeferenced, but boundary file provided (will ignore boundary file)")
	}

	// If sfm-algorithm is triangulation, check if photos have OPK
	//if args.sfm_algorithm == 'triangulation':
	{
		//for p in photos:
		//	if not p.has_opk():
		//		Logger::Write(__FUNCTION__, "No omega/phi/kappa angles found in input photos (%s), switching sfm-algorithm to incremental" % p.filename)
		//		args.sfm_algorithm = 'incremental'
		//		break
	}

	//# Rolling shutter cannot be done in non-georeferenced datasets
	//if args.rolling_shutter and not reconstruction.is_georeferenced():
	{
		//	Logger::Write(__FUNCTION__, "Reconstruction is not georeferenced, disabling rolling shutter correction")
		//	args.rolling_shutter = False
	}

	GetProject().Update();		// trigger ProjectWindow update
	BenchmarkStop(true);

	return status;
}

int StageDataset::InitGeoref()
{
	// Create & initialze georeferencing folder.
	//

	int status = 0;

	AeroLib::CreateFolder(tree.georef_path);

	// write 'coords.txt'

	// Create a coordinate file containing the GPS positions of all cameras
	// to be used later in the ODM toolchain for automatic georeferecing

	int utm_zone = 0;
	char hemi = ' ';
	std::vector<VEC3> coords;
	
	for (auto image : GetProject().GetImageList())
	{
		//	if photo.latitude is None or photo.longitude is None:
		//		Logger::Write(__FUNCTION__, "GPS position not available for %s" % photo.filename)
		//		continue

		double lat = image->GetLatitude();
		double lon = image->GetLongitude();

		if (utm_zone == 0)
		{
			utm_zone = GIS::GetUTMZone(lon);
			hemi = (lat >= 0.0 ? 'N' : 'S');
		}

		double x, y;
		GIS::LatLonToXY_UTM(lat, lon, x, y, GIS::Ellipsoid::WGS_84);

		coords.push_back(VEC3(x, y, image->GetAltitude()));
	}

	if (utm_zone == 0)
	{
		Logger::Write(__FUNCTION__, "No images seem to have GPS information");
		assert(false);
	}

	// Calculate average
	double dx = 0.0;
	double dy = 0.0;
	double num = (double)coords.size();
	for (const VEC3& coord : coords)
	{
		dx += coord.x / num;
		dy += coord.y / num;
	}

	dx = (int)floor(dx);
	dy = (int)floor(dy);

	FILE* pFile = fopen(tree.georef_coords.c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "WGS84 UTM %d%c\n", utm_zone, hemi);
		fprintf(pFile, "%d %d\n", (int)dx, (int)dy);
		for (const VEC3& coord : coords)
			fprintf(pFile, "%0.15f %0.15f %0.15f\n", coord.x - dx, coord.y - dy, coord.z);

		fclose(pFile);
	}

	// write 'proj.txt'

	XString file_name = XString::CombinePath(tree.georef_path, "proj.txt");
	pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, AeroLib::GetProj(utm_zone, hemi).c_str());

		fclose(pFile);
	}

	return status;
}

int StageDataset::WriteImageListText()
{
	// Write 'img_list.txt'
	//

	int status = 0;

	XString file_name = XString::CombinePath(GetProject().GetDroneOutputPath(), "img_list.txt");
	FILE* pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		for (auto image : GetProject().GetImageList())
		{
			// root name only
			fprintf(pFile, "%s\n", image->GetFileName().GetFileName().c_str());
		}

		fclose(pFile);
	}

	return status;
}

int StageDataset::WriteImageListJson()
{
	// Write 'images.json'
	//

	int status = 0;

	XString file_name = XString::CombinePath(GetProject().GetDroneOutputPath(), "images.json");
	FILE* pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "[\n");

		int ctr = 0;
		for (auto image : GetProject().GetImageList())
		{
			fprintf(pFile, "	{\n");
			fprintf(pFile, "		\"filename\": \"%s\",\n", image->GetFileName().GetFileName().c_str());
			fprintf(pFile, "		\"mask\": null,\n");
			fprintf(pFile, "		\"width\": %d,\n", image->GetWidth());
			fprintf(pFile, "		\"height\": %d,\n", image->GetHeight());
			fprintf(pFile, "		\"camera_make\": \"%s\",\n", image->GetMake().c_str());
			fprintf(pFile, "		\"camera_model\": \"%s\",\n", image->GetModel().c_str());
			fprintf(pFile, "		\"orientation\": %d,\n", image->GetOrientation());
			fprintf(pFile, "		\"latitude\": %0.15f,\n", image->GetLatitude());			//46.553156600003355
			fprintf(pFile, "		\"longitude\": %0.15f,\n", image->GetLongitude());
			fprintf(pFile, "		\"altitude\": %0.12f,\n", image->GetAltitude());			//980.296992481203
			fprintf(pFile, "		\"band_name\": \"%s\",\n", image->GetBandName().c_str());
			fprintf(pFile, "		\"band_index\": 0,\n");
			fprintf(pFile, "		\"capture_uuid\": null,\n");
			fprintf(pFile, "		\"fnumber\": %0.1f,\n", image->GetFNumber());
			fprintf(pFile, "		\"radiometric_calibration\": null,\n");
			fprintf(pFile, "		\"black_level\": null,\n");
			fprintf(pFile, "		\"gain\": null,\n");
			fprintf(pFile, "		\"gain_adjustment\": null,\n");
			fprintf(pFile, "		\"exposure_time\": %0.3f,\n", image->GetExposureTime());
			fprintf(pFile, "		\"iso_speed\": %d,\n", image->GetISOSpeed());
			fprintf(pFile, "		\"bits_per_sample\": null,\n");
			fprintf(pFile, "		\"vignetting_center\": null,\n");
			fprintf(pFile, "		\"vignetting_polynomial\": null,\n");
			fprintf(pFile, "		\"spectral_irradiance\": null,\n");
			fprintf(pFile, "		\"horizontal_irradiance\": null,\n");
			fprintf(pFile, "		\"irradiance_scale_to_si\": null,\n");
			fprintf(pFile, "		\"utc_time\": %I64u000.0,\n", image->GetEpoch());			//1299256936000.0
			fprintf(pFile, "		\"yaw\": null,\n");
			fprintf(pFile, "		\"pitch\": null,\n");
			fprintf(pFile, "		\"roll\": null,\n");
			fprintf(pFile, "		\"omega\": null,\n");
			fprintf(pFile, "		\"phi\": null,\n");
			fprintf(pFile, "		\"kappa\": null,\n");
			fprintf(pFile, "		\"sun_sensor\": null,\n");
			fprintf(pFile, "		\"dls_yaw\": null,\n");
			fprintf(pFile, "		\"dls_pitch\": null,\n");
			fprintf(pFile, "		\"dls_roll\": null,\n");
			fprintf(pFile, "		\"speed_x\": null,\n");
			fprintf(pFile, "		\"speed_y\": null,\n");
			fprintf(pFile, "		\"speed_z\": null,\n");
			fprintf(pFile, "		\"exif_width\": %d,\n", image->GetExifWidth());
			fprintf(pFile, "		\"exif_height\": %d,\n", image->GetExifHeight());
			fprintf(pFile, "		\"gps_xy_stddev\": null,\n");
			fprintf(pFile, "		\"gps_z_stddev\": null,\n");
			fprintf(pFile, "		\"camera_projection\": \"brown\",\n");
			fprintf(pFile, "		\"focal_ratio\": %0.16f\n", image->GetFocalRatio());		//0.8067639086097845
			fprintf(pFile, "	}%s\n", ++ctr < GetProject().GetImageCount() ? "," : "");
		}
		fprintf(pFile, "]\n");

		fclose(pFile);
	}

	return status;
}

StageDataset::StageDataset(DroneProc* pDroneProc)
	: Stage(pDroneProc)
{
	m_stage_id = Id::Dataset;
}

StageDataset::~StageDataset()
{

}
