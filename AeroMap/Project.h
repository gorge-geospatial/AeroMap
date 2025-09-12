#ifndef PROJECT_H
#define PROJECT_H

#include <vector>

#include "exif.h"			// easy exif header
#include "Gis.h"
#include "XString.h"
#include "LasFile.h"
#include "Reconstruction.h"
#include "Photo.h"

#include "gdal_priv.h"
#include "cpl_conv.h"		// for CPLMalloc()
#include "ogrsf_frmts.h"

#include <QObject>

struct ArgType
{
	bool dsm;
	bool dtm;
	int max_concurrency;

	double dem_resolution;
	int    dem_decimation;
	bool   dem_euclidean_map;
	int    dem_gapfill_steps;

	double ortho_resolution;
	bool fast_orthophoto;
	bool orthophoto_no_tiled;
	bool orthophoto_png;
	bool orthophoto_kmz;
	XString orthophoto_compression;
	bool orthophoto_cutline;

	int octree_depth;
	int mesh_size;

	double crop;
	XString boundary;
	bool auto_boundary;
	double auto_boundary_distance;

	double pc_filter;
	double pc_sample;
	XString pc_quality;
	bool pc_classify;
	bool pc_csv;
	bool pc_las;
	bool pc_ept;
	bool pc_copc;
	bool pc_skip_geometric;

	bool use_3dmesh;
	bool use_exif;
	bool skip_3dmodel;
	bool skip_report;
	bool skip_orthophoto;

	bool ignore_gsd;

	bool dev_sfm;			// experimental sfm pipeline

	// Params for features
	XString feature_type;
	//# If true, apply square root mapping to features
	//feature_root: bool = True
	//# If fewer frames are detected, sift_peak_threshold/surf_hessian_threshold is reduced.
	int feature_min_frames;
	//# Same as above but for panorama images
	int feature_min_frames_panorama;
	//# Resize the image if its size is larger than specified. Set to -1 for original size
	int feature_process_size;
	//# Same as above but for panorama images
	int feature_process_size_panorama;
	//feature_use_adaptive_suppression: bool = False
	//# Bake segmentation info (class and instance) in the feature data. Thus it is done once for all at extraction time.
	//features_bake_segmentation: bool = False

	//##################################
	//# Params for SIFT
	//##################################
	//# Smaller value -> more features
	double sift_peak_threshold;
	//# See OpenCV doc
	double sift_edge_threshold;

	//##################################
	//# Params for SURF
	//##################################
	//# Smaller value -> more features
	//surf_hessian_threshold: float = 3000
	//# See OpenCV doc
	//surf_n_octaves: int = 4
	//# See OpenCV doc
	//surf_n_octavelayers: int = 2
	//# See OpenCV doc
	//surf_upright: int = 0

	//##################################
	//# Params for AKAZE (See details in lib/src/third_party/akaze/AKAZEConfig.h)
	//##################################
	//# Maximum octave evolution of the image 2^sigma (coarsest scale sigma units)
	//akaze_omax: int = 4
	//# Detector response threshold to accept point
	//akaze_dthreshold: float = 0.001
	//# Feature type
	XString akaze_descriptor;
	//# Size of the descriptor in bits. 0->Full size
	//akaze_descriptor_size: int = 0
	//# Number of feature channels (1,2,3)
	//akaze_descriptor_channels: int = 3
	//akaze_kcontrast_percentile: float = 0.7
	//akaze_use_isotropic_diffusion: bool = False

	//##################################
	//# Params for HAHOG
	//##################################
	double hahog_peak_threshold;
	double hahog_edge_threshold;
	bool hahog_normalize_to_uchar;

	//##################################
	//# Params for general matching
	//##################################
	//# Ratio test for matches
	//lowes_ratio: float = 0.8
	//# FLANN, BRUTEFORCE, or WORDS
	//matcher_type: str = "FLANN"
	//# Match symmetrically or one-way
	//symmetric_matching: bool = True

	//##################################
	//# Params for FLANN matching
	//##################################
	//# Algorithm type (KMEANS, KDTREE)
	//flann_algorithm: str = "KMEANS"
	//# See OpenCV doc
	//flann_branching: int = 8
	//# See OpenCV doc
	//flann_iterations: int = 10
	//# See OpenCV doc
	//flann_tree: int = 8
	//# Smaller -> Faster (but might lose good matches)
	//flann_checks: int = 20

	//##################################
	//# Params for BoW matching
	//##################################
	//bow_file: str = "bow_hahog_root_uchar_10000.npz"
	//# Number of words to explore per feature.
	//bow_words_to_match: int = 50
	//# Number of matching features to check.
	//bow_num_checks: int = 20
	//# Matcher type to assign words to features
	//bow_matcher_type: str = "FLANN"

	//##################################
	//# Params for VLAD matching
	//##################################
	//vlad_file: str = "bow_hahog_root_uchar_64.npz"
};
extern ArgType arg;

struct TreeType
{
	XString sfm;						// root AeroMap sfm output folder

	XString opensfm;					// root OpenSFM output folder
	XString opensfm_reconstruction;
	XString opensfm_reconstruction_nvm;
	XString opensfm_topocentric_reconstruction;
	XString opensfm_geocoords_reconstruction;

	XString openmvs;
	XString openmvs_model;

	XString filter_path;
	XString filtered_point_cloud;
	XString filtered_point_cloud_stats;

	XString mesh_path;					// root output mesh path
	XString mesh_file;
	XString mesh_file_25d;

	XString texture_path;				// root output texture path, 3d
	XString texture_path_25d;			// 2.5d
	XString textured_model_obj;
	XString textured_model_glb;

	XString georef_path;				// root output georeferencing path
	XString georef_coords;
	XString georef_model_laz;
	XString georef_gcp;
	XString georef_gcp_utm;

	XString dem_path;					// root output dem path
	XString dtm_file;					// AeroMap terrain models
	XString dsm_file;

	XString ortho_path;					// root output orthophoto path
	XString orthophoto_tif;
	XString orthophoto_render;
	XString orthophoto_log;
	XString orthophoto_corners;
	XString orthophoto_tiles;

	XString report_path;				// root output report path

	// executables

	XString prog_opensfm;
	XString prog_densify;
	XString prog_filter;
	XString prog_pdal;
	XString prog_poisson;
	XString prog_recon_mesh;
	XString prog_recon_tex;
	XString prog_gdal_addo;
	XString prog_gdal_buildvrt;
	XString prog_gdal_translate;
	XString prog_gdal_warp;
	XString prog_dem2mesh;
	XString prog_dem2points;
	XString prog_odm_orthophoto;
	XString prog_ogr2ogr;
};
extern TreeType tree;

class Project : public QObject
{
	Q_OBJECT

public:

	struct LidarType
	{
		GIS::GEODATA type;		// geospatial data type, las or laz only
		XString src;			// full path/name of lidar source file
		LasFile* pFile;
		bool exists;			// false if file not found

		LidarType()
			: type(GIS::GEODATA::None)
			, pFile(nullptr)
		{
		}
	};

public:

	Project();
	~Project();

	int Load(const char* fileName);
	int Save(const char* fileName = nullptr);
	void Clear();
	void Update();
	bool IsDirty();

	int LoadImageList();
	int GetImageCount();
	const std::vector<Photo*>& GetImageList();

	XString GetFileName();
	XString GetName();

	XString GetDroneInputPath();
	void    SetDroneInputPath(XString path);
	XString GetDroneOutputPath();
	void    SetDroneOutputPath(XString path);

	int		 GetLidarFileCount();
	void	 AddLidarFile(const char* pathName);
	void	 RemoveLidarFile(int index);
	XString  GetLidarFileName(int index);
	LasFile* GetLasFile(int index);
	bool	 GetLidarExists(int index);

	Reconstruction* CreateReconstruction();
	Reconstruction* GetReconstruction();

	void PreProcessArgs();

	static const char* GetDefaultExt();		// default file extension
	
signals:

	void projectChanged_sig();				// any change to project structure

private:

	std::vector<Photo*> m_ImageList;
	std::vector<LidarType> mv_Lidar;		// lidar files
	
	XString ms_FileName;			// project path/file name
	XString ms_ProjectName;			// project name

	XString ms_DroneInputPath;		// location of input images
	XString ms_DroneOutputPath;		// root folder that will receive outputs

	int m_ErrorCtr;					// # of errors in project file
	bool mb_IsDirty;				// unsaved changes

	Reconstruction* mp_Reconstruction;

private:

	int  GetIndentLevel(XString& str);
	void FreeResources();
	void LoadData();

	void InitArg();
	void InitTree();

	// private methods to update internal component lists
	// with correctly formatted data; these do not set dirty
	// flags or emit signals
	void AddLidarFileToList(const char* pathName);
};

#endif // #ifndef PROJECT_H
