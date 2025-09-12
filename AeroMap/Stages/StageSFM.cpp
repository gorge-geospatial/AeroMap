// StageSFM.cpp
// Structure from motion.
//

#include "Sfm/Features.h"
#include "Sfm/Matching.h"
#include "Sfm/Tracking.h"

#include "ConfigSFM.h"
#include "StageSFM.h"

int StageSFM::Run()
{
	// Inputs:
	// Outputs:
	//		+ opensfm
	//			+ exif
	//				[file name*].exif		
	//			camera_models.json
	//			config.yaml					
	//			image_list.txt
	//			reference_lla.json
	//		cameras.json
	//

	int status = 0;

	GetApp()->LogWrite("OpenSFM...");
	BenchmarkStart();
	LogStage();

	Setup();

	if (arg.dev_sfm)
		NewSfmPipeline();

	DetectFeatures();
	MatchFeatures();
	CreateTracks();
	Reconstruct();

	// Export reconstruction stats

	ComputeStatistics();
	ExportGeocoords();

	// Updating d:/test/opensfm/config.yaml
	// undistorted_image_max_size: 4000

	Undistort();
	ExportVisualSFM();
	ExportPly();

	GetProject().Update();		// trigger ProjectWindow update
	BenchmarkStop();

	return status;
}

void StageSFM::NewSfmPipeline()
{
	// Run experimental sfm pipeline.
	//

	AeroLib::CreateFolder(tree.sfm);

	XString file_name = GetProject().GetImageList()[0]->GetFileName();

	Features features;
	features.detect(file_name);

	XString feature_file = XString::CombinePath(tree.sfm, "features.txt");
	features.save(feature_file);

	// since outputs aren't used past here yet, just stop
	assert(false);
}

void StageSFM::DetectFeatures()
{
	XString features_dir = XString::CombinePath(tree.opensfm, "features");
	if ((AeroLib::FileExists(features_dir) == false) || Rerun())
	{
		GetApp()->LogWrite("OpenSFM: Detect features...");
		QStringList args;
		args.push_back("detect_features");
		args.push_back(tree.opensfm.c_str());
		AeroLib::RunProgram(tree.prog_opensfm, args);
		// cmd: opensfm detect_features "d:/test/opensfm"
	}
	else
	{
		Logger::Write(__FUNCTION__, "Detect features already done: '%s' exists", features_dir.c_str());
	}
}

void StageSFM::MatchFeatures()
{
	XString matches_dir = XString::CombinePath(tree.opensfm, "matches");
	if ((AeroLib::FileExists(matches_dir) == false) || Rerun())
	{
		GetApp()->LogWrite("OpenSFM: Match features...");
		QStringList args;
		args.push_back("match_features");
		args.push_back(tree.opensfm.c_str());
		AeroLib::RunProgram(tree.prog_opensfm, args);
		// cmd: opensfm match_features "d:/test/opensfm"
	}
	else
	{
		Logger::Write(__FUNCTION__, "Match features already done: '%s' exists", matches_dir.c_str());
	}
}

void StageSFM::CreateTracks()
{
	XString tracks_file = XString::CombinePath(tree.opensfm, "tracks.csv");
	if ((AeroLib::FileExists(tracks_file) == false) || Rerun())
	{
		GetApp()->LogWrite("OpenSFM: Create tracks...");
		QStringList args;
		args.push_back("create_tracks");
		args.push_back(tree.opensfm.c_str());
		AeroLib::RunProgram(tree.prog_opensfm, args);
		// cmd: opensfm create_tracks "d:/test/opensfm"
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing OpenSFM tracks file: '%s'", tracks_file.c_str());
	}
}

void StageSFM::Reconstruct()
{
	XString reconstruction_file = XString::CombinePath(tree.opensfm, "reconstruction.json");
	if ((AeroLib::FileExists(reconstruction_file) == false) || Rerun())
	{
		GetApp()->LogWrite("OpenSFM: Reconstruct...");
		QStringList args;
		args.push_back("reconstruct");
		args.push_back(tree.opensfm.c_str());
		AeroLib::RunProgram(tree.prog_opensfm, args);
		// cmd: opensfm reconstruct "d:/test/opensfm"
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing OpenSFM reconstruction file: '%s'", reconstruction_file.c_str());
	}
}

void StageSFM::ExportGeocoords()
{
	if ((AeroLib::FileExists(tree.opensfm_geocoords_reconstruction) == false) || Rerun())
	{
		AeroLib::Georef georef = AeroLib::ReadGeoref();
		if (georef.is_valid)
		{
			GetApp()->LogWrite("OpenSFM: Export geocoords...");
			QStringList args;
			args.push_back("export_geocoords");
			args.push_back("--reconstruction");
			args.push_back("--proj");
			//TODO: no 's' handling
			args.push_back(XString::Format("+proj=utm +zone=%d +datum=WGS84 +units=m +no_defs +type=crs", georef.utm_zone).c_str());
			args.push_back("--offset-x");
			args.push_back(XString::Format("%0.1f", georef.x).c_str());
			args.push_back("--offset-y");
			args.push_back(XString::Format("%0.1f", georef.y).c_str());
			args.push_back(tree.opensfm.c_str());
			AeroLib::RunProgram(tree.prog_opensfm, args);
			// cmd: opensfm export_geocoords --reconstruction 
			//			--proj "+proj=utm +zone=32 +datum=WGS84 +units=m +no_defs +type=crs"
			//			--offset-x 322263.0 --offset-y 5157982.0 "d:/test/opensfm"
		}
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing geocoords file: '%s'", tree.opensfm_geocoords_reconstruction.c_str());
	}
}

void StageSFM::ComputeStatistics()
{
	XString stats_path = XString::CombinePath(tree.opensfm, "stats/stats.json");
	if ((AeroLib::FileExists(stats_path) == false) || Rerun())
	{
		GetApp()->LogWrite("OpenSFM: Compute statistics...");
		QStringList args;
		args.push_back("compute_statistics");
		args.push_back("--diagram_max_points");
		args.push_back("100000");
		args.push_back(tree.opensfm.c_str());
		AeroLib::RunProgram(tree.prog_opensfm, args);
		// cmd: opensfm compute_statistics --diagram_max_points 100000 "d:/test/opensfm"
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing reconstruction stats: '%s'", stats_path.c_str());
	}
}

void StageSFM::Undistort()
{
	XString run_id = "nominal";		// can be 'primary' or 'nominal'
	XString done_flag_file = XString::CombinePath(tree.opensfm, XString::Format("undistorted/%s_done.txt", run_id.c_str()));
	if ((AeroLib::FileExists(done_flag_file) == false) || Rerun())
	{
		GetApp()->LogWrite("OpenSFM: Undistort...");
		QStringList args;
		args.push_back("undistort_aero");
		args.push_back(tree.opensfm.c_str());
		AeroLib::RunProgram(tree.prog_opensfm, args);
		AeroLib::Touch(done_flag_file);
	}
	else
	{
		Logger::Write(__FUNCTION__, "Already undistorted: '%s'", run_id.c_str());
	}
}

void StageSFM::ExportVisualSFM()
{
	if ((AeroLib::FileExists(tree.opensfm_reconstruction_nvm) == false) || Rerun())
	{
		GetApp()->LogWrite("OpenSFM: Export visualsfm...");
		QStringList args;
		args.push_back("export_visualsfm");
		args.push_back("--points");
		args.push_back(tree.opensfm.c_str());
		AeroLib::RunProgram(tree.prog_opensfm, args);
		// cmd: opensfm export_visualsfm --points "d:/test/opensfm"
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing OpenSFM NVM reconstruction file: '%s'", tree.opensfm_reconstruction_nvm);
	}
}

void StageSFM::ExportPly()
{
	// Skip dense reconstruction if necessary and export
	// sparse reconstruction instead
	//

	if (arg.fast_orthophoto)
	{
		XString output_file = XString::CombinePath(tree.opensfm, "reconstruction.ply");

		if ((AeroLib::FileExists(output_file) == false) || Rerun())
		{
			GetApp()->LogWrite("OpenSFM: Export ply...");
			QStringList args;
			args.push_back("export_ply");
			args.push_back("--no-cameras");
			args.push_back("--point-num-views");
			args.push_back(tree.opensfm.c_str());
			AeroLib::RunProgram(tree.prog_opensfm, args);
			//octx.run('export_ply --no-cameras --point-num-views')
		}
		else
		{
			Logger::Write(__FUNCTION__, "Found existing PLY reconstruction: '%s'", output_file.c_str());
		}
	}
}

int StageSFM::Setup()
{
	int status = 0;

	WriteExif();
	WriteImageListText();
	WriteCameraModelsJson();
	WriteReferenceLLA();
	WriteConfigYaml();

	WriteCamerasJson();

	return status;
}

int StageSFM::WriteExif()
{
	// Create & populate opensfm/exif folder
	//

	int status = 0;

	XString exif_path = XString::CombinePath(tree.opensfm, "exif");
	AeroLib::CreateFolder(exif_path);

	for (auto image : GetProject().GetImageList())
	{
		// file names look like: 'IMG_0428.JPG.exif'
		XString file_name = XString::CombinePath(exif_path, image->GetFileName().GetFileName() + ".exif");
		FILE* pFile = fopen(file_name.c_str(), "wt");
		if (pFile)
		{
			fprintf(pFile, "{\n");
			fprintf(pFile, "    \"make\": \"%s\",\n", image->GetMake().c_str());
			fprintf(pFile, "    \"model\": \"%s\",\n", image->GetModel().c_str());
			fprintf(pFile, "    \"width\": %d,\n", image->GetWidth());
			fprintf(pFile, "    \"height\": %d,\n", image->GetHeight());
			fprintf(pFile, "    \"projection_type\": \"brown\",\n");			// in/derived-from exif?
			fprintf(pFile, "    \"focal_ratio\": %0.16f,\n", image->GetFocalRatio());
			fprintf(pFile, "    \"orientation\": %d,\n", image->GetOrientation());
			fprintf(pFile, "    \"capture_time\": %I64u.0,\n", image->GetEpoch());		//1299256936.0
			fprintf(pFile, "    \"gps\": {\n");
			fprintf(pFile, "        \"latitude\": %0.15f,\n", image->GetLatitude());		//46.553156600003355
			fprintf(pFile, "        \"longitude\": %0.15f,\n", image->GetLongitude());
			fprintf(pFile, "        \"altitude\": %0.12f,\n", image->GetAltitude());		//980.296992481203
			fprintf(pFile, "        \"dop\": 10.0\n");		//TODO: odm gets 10, i get 0
			fprintf(pFile, "    },\n");
			fprintf(pFile, "    \"camera\": \"%s\"\n", image->GetCameraString(true).c_str());
			fprintf(pFile, "}\n");

			fclose(pFile);
		}
	}

	return status;
}

int StageSFM::WriteImageListText()
{
	// Write 'opensfm/image_list.txt'
	//
	
	int status = 0;

	XString file_name = XString::CombinePath(tree.opensfm, "image_list.txt");
	FILE* pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		for (auto image : GetProject().GetImageList())
		{
			// full path name, opensfm uses this to find images
			fprintf(pFile, "%s\n", image->GetFileName().c_str());
		}

		fclose(pFile);
	}

	return status;
}

int StageSFM::WriteCameraModelsJson()
{
	// Write 'opensfm/camera_models.json'
	//
	// Believe intent is 1 entry for each camera detected in input
	// image dataset. Making simplifying assumption all images 
	// captured with same camera.
	//

	int status = 0;

	XString file_name = XString::CombinePath(tree.opensfm, "camera_models.json");
	FILE* pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		auto image = GetProject().GetImageList()[0];

		fprintf(pFile, "{\n");
		fprintf(pFile, "    \"%s\": {\n", image->GetCameraString(true).c_str());
		fprintf(pFile, "        \"projection_type\": \"brown\",\n");		//TODO: in/derived from exif?
		fprintf(pFile, "        \"width\": %d,\n", image->GetWidth());
		fprintf(pFile, "        \"height\": %d,\n", image->GetHeight());
		fprintf(pFile, "        \"focal_x\": %0.16f,\n", image->GetFocalRatio());
		fprintf(pFile, "        \"focal_y\": %0.16f,\n", image->GetFocalRatio());	// any scenario where x & y could differ?
		fprintf(pFile, "        \"c_x\": 0.0,\n");
		fprintf(pFile, "        \"c_y\": 0.0,\n");
		fprintf(pFile, "        \"k1\": 0.0,\n");	//TODO: how are these calculated
		fprintf(pFile, "        \"k2\": 0.0,\n");
		fprintf(pFile, "        \"p1\": 0.0,\n");
		fprintf(pFile, "        \"p2\": 0.0,\n");
		fprintf(pFile, "        \"k3\": 0.0\n");
		fprintf(pFile, "    }\n");
		fprintf(pFile, "}\n");

		fclose(pFile);
	}

	return status;
}

int StageSFM::WriteReferenceLLA()
{
	// Write 'opensfm/reference_lla.json'
	//
	// This is the center of the image field converted
	// back to lat/lon.
	//

	int status = 0;

	AeroLib::Georef georef = AeroLib::ReadGeoref();
	if (georef.is_valid)
	{
		XString file_name = XString::CombinePath(tree.opensfm, "reference_lla.json");
		FILE* pFile = fopen(file_name.c_str(), "wt");
		if (pFile)
		{
			double lat, lon;
			GIS::XYToLatLon_UTM(georef.utm_zone, georef.hemi, georef.x, georef.y, lat, lon, GIS::Ellipsoid::WGS_84);

			fprintf(pFile, "{\n");
			fprintf(pFile, "	\"latitude\": %0.14f,\n", lat);
			fprintf(pFile, "	\"longitude\": %0.14f, \n", lon);
			fprintf(pFile, "	\"altitude\": 0.0\n");
			fprintf(pFile, "}\n");

			fclose(pFile);
		}
	}

	return status;
}

int StageSFM::WriteConfigYaml()
{
	// Write 'opensfm/config.yaml'
	//

	int status = 0;

	ConfigSFM config_sfm;
	config_sfm.WriteDefaultFile();

	return status;
}

int StageSFM::WriteCamerasJson()
{
	// Write 'cameras.json'
	//
	// Same format as opensfm/camera_models.json with
	// potentially differing data.
	//

	int status = 0;

	XString file_name = XString::CombinePath(GetProject().GetDroneOutputPath(), "cameras.json");
	FILE* pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		auto image = GetProject().GetImageList()[0];

		fprintf(pFile, "{\n");
		fprintf(pFile, "    \"%s\": {\n", image->GetCameraString(true).c_str());
		fprintf(pFile, "        \"projection_type\": \"brown\",\n");		//TODO: in/derived from exif?
		fprintf(pFile, "        \"width\": %d,\n", image->GetWidth());
		fprintf(pFile, "        \"height\": %d,\n", image->GetHeight());
		fprintf(pFile, "        \"focal_x\": %0.16f,\n", image->GetFocalRatio());
		fprintf(pFile, "        \"focal_y\": %0.16f,\n", image->GetFocalRatio());	// any scenario where x & y could differ?
		fprintf(pFile, "        \"c_x\": 0.0,\n");
		fprintf(pFile, "        \"c_y\": 0.0,\n");
		fprintf(pFile, "        \"k1\": 0.0,\n");	//TODO: how are these calculated
		fprintf(pFile, "        \"k2\": 0.0,\n");
		fprintf(pFile, "        \"p1\": 0.0,\n");
		fprintf(pFile, "        \"p2\": 0.0,\n");
		fprintf(pFile, "        \"k3\": 0.0\n");
		fprintf(pFile, "    }\n");
		fprintf(pFile, "}\n");

		fclose(pFile);
	}

	return status;
}

StageSFM::StageSFM(DroneProc* pDroneProc)
	: Stage(pDroneProc)
{
	m_stage_id = Id::OpenSFM;
}

StageSFM::~StageSFM()
{

}
