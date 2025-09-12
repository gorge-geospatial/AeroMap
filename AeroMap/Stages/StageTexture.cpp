// StageTexture.cpp
// Apply textures
//

#include "StageTexture.h"

int StageTexture::Run()
{
	// Inputs:
	// Outputs:
	//		+ texture
	//			textured_model_geo.conf
	//			textured_model_geo.mtl
	//			textured_model_geo.obj
	//		+ texture_25d
	//			textured_model_geo.conf
	//			textured_model_geo.mtl
	//			textured_model_geo.obj
	//

	int status = 0;

	GetApp()->LogWrite("Texture...");
	BenchmarkStart();
	LogStage();

	int max_dim = Photo::find_largest_photo_dim(GetReconstruction()->GetImageList());
	m_max_texture_size = 8 * 1024;	// default

	if (max_dim > 8000)
	{
		Logger::Write(__FUNCTION__, "Large input images (%d pixels), increasing maximum texture size.", max_dim);
		m_max_texture_size *= 3;
	}

	TextureModel3D();
	TextureModel25D();

	GetProject().Update();		// trigger ProjectWindow update
	BenchmarkStop();

	return status;
}

int StageTexture::TextureModel3D()
{
	int status = 0;
	
	AeroLib::CreateFolder(tree.texture_path);

	XString textured_model_obj = XString::CombinePath(tree.texture_path, tree.textured_model_obj);
	if ((AeroLib::FileExists(textured_model_obj) == false) || Rerun())
	{
		GetApp()->LogWrite("Writing MVS textured file: '%s'", textured_model_obj.c_str());

		XString out_prefix = XString::CombinePath(tree.texture_path, "textured_model_geo");

		QStringList args;
		args.push_back(tree.opensfm_reconstruction_nvm.c_str());		// input scene
		args.push_back(tree.mesh_file.c_str());							// input mesh
		args.push_back(out_prefix.c_str());								// output prefix
		args.push_back("-d");					// Data term: {area, gmi}
		args.push_back("gmi");
		args.push_back("-o");					// Photometric outlier (pedestrians etc.) removal method: {none, gauss_damping, gauss_clamping}
		args.push_back("gauss_clamping");		
		args.push_back("-t");
		args.push_back("none");					// Tone mapping method: {none, gamma}
		args.push_back("--no_intermediate_results");
		args.push_back(XString::Format("--max_texture_size=%d", m_max_texture_size).c_str());
		AeroLib::RunProgram(tree.prog_recon_tex, args);
		// cmd: texrecon 
		//			"d:/test/opensfm/undistorted/reconstruction.nvm" 
		//			"d:/test/odm_meshing/odm_mesh.ply" 
		//			"d:/test/texture/textured_model_geo" 
		//			-d gmi -o gauss_clamping -t none --no_intermediate_results --max_texture_size=8192
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing textured model: '%s'", textured_model_obj.c_str());
	}
	
	return status;
}

int StageTexture::TextureModel25D()
{
	int status = 0;

	AeroLib::CreateFolder(tree.texture_path_25d);

	XString textured_model_obj = XString::CombinePath(tree.texture_path_25d, tree.textured_model_obj);
	if ((AeroLib::FileExists(textured_model_obj) == false) || Rerun())
	{
		GetApp()->LogWrite("Writing MVS textured file: '%s'", textured_model_obj.c_str());

		XString out_prefix = XString::CombinePath(tree.texture_path_25d, "textured_model_geo");

		QStringList args;
		args.push_back(tree.opensfm_reconstruction_nvm.c_str());		// input scene
		args.push_back(tree.mesh_file_25d.c_str());						// input mesh
		args.push_back(out_prefix.c_str());								// output prefix
		args.push_back("-d");
		args.push_back("gmi");
		args.push_back("-o");
		args.push_back("gauss_clamping");
		args.push_back("-t");
		args.push_back("none");
		args.push_back("--nadir_mode");
		args.push_back("--no_intermediate_results");
		args.push_back(XString::Format("--max_texture_size=%d", m_max_texture_size).c_str());
		AeroLib::RunProgram(tree.prog_recon_tex, args);
		// cmd: texrecon 
		//			"d:/test/opensfm/undistorted/reconstruction.nvm" 
		//			"d:/test/odm_meshing/odm_25dmesh.ply" 
		//			"d:/test/texture_25d/textured_model_geo" 
		//			-d gmi -o gauss_clamping -t none --no_intermediate_results
		//		    --nadir_mode
		//			--max_texture_size=8192
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing textured model: '%s'", textured_model_obj.c_str());
	}

	return status;
}

StageTexture::StageTexture(DroneProc* pDroneProc)
	: Stage(pDroneProc)
{
	m_stage_id = Id::Texture;
}

StageTexture::~StageTexture()
{

}
