// Features.cpp
// Feature detection/management stage of SFM pipeline.
// 

#include "Project.h"
#include "Features.h"

constexpr char* SECT_KEYPOINT = "_KEYPOINTS";
constexpr char* SECT_DESC = "_DESCRIPTORS";
constexpr char* SECT_COLOR = "_COLORS";

Features::Features()
{

}

Features::~Features()
{

}

void Features::detect(XString file_name)
{
	// Public entry point for feature detection.
	// 

	//def detect(
//    image: str,
//    image_array: np.ndarray,
//    segmentation_array: Optional[np.ndarray],
//    instances_array: Optional[np.ndarray],
//    data: DataSetBase,
//    force: bool = False,
//) -> None:

//    need_words = (
//        data.config["matcher_type"] == "WORDS"
//        or data.config["matching_bow_neighbors"] > 0
//    )
//    has_words = not need_words or data.words_exist(image)
//    has_features = data.features_exist(image)
//
//    if not force and has_features and has_words:
//        logger.info(
//            "Skip recomputing {} features for image {}".format(
//                data.feature_type().upper(), image
//            )
//        )
//        return
//
//    Logger::Write(__FUNCTION__, "Extracting {} features for image {}".format(data.feature_type().upper(), image))
//
//    start = timer()

	m_image = cv::imread(file_name.c_str(), cv::IMREAD_UNCHANGED);

	extract_features(m_image, false);
	//    p_unmasked, f_unmasked, c_unmasked = features.extract_features(
	//        image_array, data.config, is_high_res_panorama(data, image, image_array)
	//    )
	//
	//    # Load segmentation and bake it in the data
	//    if data.config["features_bake_segmentation"]:
	//        exif = data.load_exif(image)
	//        s_unsorted, i_unsorted = bake_segmentation(
	//            image_array, p_unmasked, segmentation_array, instances_array, exif
	//        )
	//        p_unsorted = p_unmasked
	//        f_unsorted = f_unmasked
	//        c_unsorted = c_unmasked
	//    # Load segmentation, make a mask from it mask and apply it
	//    else
	//        s_unsorted, i_unsorted = None, None
	//        fmask = masking.load_features_mask(data, image, p_unmasked)
	//        p_unsorted = p_unmasked[fmask]
	//        f_unsorted = f_unmasked[fmask]
	//        c_unsorted = c_unmasked[fmask]
	//
	//    if len(p_unsorted) == 0:
	//        logger.warning("No features found in image {}".format(image))
	//
	//    size = p_unsorted[:, 2]
	//    order = np.argsort(size)
	//    p_sorted = p_unsorted[order, :]
	//    f_sorted = f_unsorted[order, :]
	//    c_sorted = c_unsorted[order, :]
	//    if s_unsorted is not None:
	//        semantic_data = features.SemanticData(
	//            s_unsorted[order],
	//            i_unsorted[order] if i_unsorted is not None else None,
	//            data.segmentation_labels(),
	//        )
	//    else
	//        semantic_data = None
	//    features_data = features.FeatureData(p_sorted, f_sorted, c_sorted, semantic_data)
	//    data.save_features(image, features_data)
	//
	//    if need_words:
	//        bows = bow.load_bows(data.config)
	//        n_closest = data.config["bow_words_to_match"]
	//        closest_words = bows.map_to_words(
	//            f_sorted, n_closest, data.config["bow_matcher_type"]
	//        )
	//        data.save_words(image, closest_words)
	//
	//    end = timer()
	//    report = {
	//        "image": image,
	//        "num_features": len(p_sorted),
	//        "wall_time": end - start,
	//    }
	//    data.save_report(io.json_dumps(report), "features/{}.json".format(image))
}

void Features::extract_features(cv::Mat image, bool is_panorama)
{
	// Detect features in a color or gray-scale image.
	//
	// The type of feature detected is determined by the feature_type
	// config option.
	//
	// The coordinates of the detected points are returned in normalized
	// image coordinates.

	// Parameters:
	//    - image: a color image with shape (h, w, 3) or
	//             gray-scale image with (h, w) or (h, w, 1)
	//    - config: the configuration structure
	//    - is_panorama : if True, alternate settings are used for feature count and extraction size.

	// Returns:
	//    tuple:
	//          - points: x, y, size and angle for each feature
	//          - descriptors: the descriptor of each feature
	//          - colors: the color of the center of each feature
	//

	int extraction_size = arg.feature_process_size;
	if (is_panorama)
		extraction_size = arg.feature_process_size_panorama;
	int feature_count = arg.feature_min_frames;
	if (is_panorama)
		feature_count = arg.feature_min_frames_panorama;

	// convert color to gray-scale if necessary
	cv::Mat image_gray;
	if (image.channels() == 3)
		cv::cvtColor(image, image_gray, cv::COLOR_RGB2GRAY);
	else
		image_gray = image;

	m_feat.m_kp.clear();

	if (arg.feature_type.CompareNoCase("SIFT"))
	{
		extract_features_sift(image_gray, feature_count);
	}
	else if (arg.feature_type.CompareNoCase("SIFT_GPU"))
	{
		extract_features_popsift(image_gray, feature_count);
	}
	else if (arg.feature_type.CompareNoCase("SURF"))
	{
		extract_features_surf(image_gray, feature_count);
	}
	else if (arg.feature_type.CompareNoCase("AKAZE"))
	{
		extract_features_akaze(image_gray, feature_count);
	}
	else if (arg.feature_type.CompareNoCase("HAHOG"))
	{
		extract_features_hahog(image_gray, feature_count);
	}
	else if (arg.feature_type.CompareNoCase("ORB"))
	{
		extract_features_orb(image_gray, feature_count);
	}
	else
	{
		Logger::Write(__FUNCTION__, "Unknown feature type: '%s':", arg.feature_type.c_str());
	}

	//xs = points[:, 0].round().astype(int)
	//ys = points[:, 1].round().astype(int)
	//colors = image[ys, xs]
	//if image.shape[2] == 1:
	//    colors = np.repeat(colors, 3).reshape((-1, 3))
	
	//TODO:
	// saved keypoints are expected to be normalized
	//if keypoints is not None:
	//    return normalize_features(points, desc, colors,
	//                              image.shape[1], image.shape[0]), keypoints
	
	//return normalize_features(points, desc, colors, image.shape[1], image.shape[0])
}

void Features::extract_features_sift(cv::Mat image, int feature_count)
{
	// Extract features using SIFT detector.
	//
	//TODO:
	// ok for testing, but sift gpu needed for anything near
	// acceptable performance
	//

	double sift_edge_threshold = arg.sift_edge_threshold;
	double sift_peak_threshold = arg.sift_peak_threshold;
	int nfeatures = int(feature_count * 3 / 2);

	cv::Ptr<cv::SIFT> sift = cv::SIFT::create(
		nfeatures,				// Number of best features to retain (0 for unlimited)
		3,						// Number of layers in each octave (3 is the default)
		sift_peak_threshold,	// Threshold to filter out weak features (peak threshold)
		sift_edge_threshold);	// Threshold to filter out edge-like features
	
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat desc;

	int iter_ctr = 8;		// infinite loop protection
	while (true)
	{
		Logger::Write(__FUNCTION__, "Computing SIFT with peak threshold %0.6f", sift_peak_threshold);
		//t = time.time()
		sift = cv::SIFT::create(
			nfeatures,				// Number of best features to retain (0 for unlimited)
			3,						// Number of layers in each octave (3 is the default)
			sift_peak_threshold,	// Threshold to filter out weak features (peak threshold)
			sift_edge_threshold);	// Threshold to filter out edge-like features

		sift->detect(image, keypoints);
		
		Logger::Write(__FUNCTION__, "Found %d points in %0.2fs", keypoints.size(), 0.0);
		if ((keypoints.size() < feature_count) && (sift_peak_threshold > 0.0001))
		{
			sift_peak_threshold = (sift_peak_threshold * 2.0) / 3.0;
			Logger::Write(__FUNCTION__, "reducing peak threshold to: %0.6f", sift_peak_threshold);
		}
		else
		{
			Logger::Write(__FUNCTION__, "done");
			break;
		}

		if (--iter_ctr < 1)
			break;
	}
	sift->compute(image, keypoints, desc);

	if (desc.empty() == false)
	{
		//if config["feature_root"]:
		//    desc = root_feature(desc)
		//points = np.array([(i.pt[0], i.pt[1], i.size, i.angle) for i in points])
	}
	else
	{
		//points = np.array(np.zeros((0, 3)))
		//desc = np.array(np.zeros((0, 3)))
	}

	m_feat.m_kp = keypoints;
	m_feat.m_desc = desc;

	// extract colors from original image
	m_feat.m_colors.clear();
	for (const auto& kp : keypoints)
	{
		int row = (int)round(kp.pt.y);
		int col = (int)round(kp.pt.x);

		int type = m_image.type();
		if (type == CV_8U)
		{
			//uchar ch = image.at<uchar>(row, col);
		}
		else if (type == CV_8UC3)
		{
			cv::Vec3b v = m_image.at <cv::Vec3b>(row, col);
			PixelType c;
			c.R = v[0];
			c.G = v[1];
			c.B = v[2];
			m_feat.m_colors.push_back(c);
		}
		else
		{
			assert(false);
		}
	}
}

void Features::extract_features_popsift(cv::Mat image, int feature_count)
{
	//def extract_features_popsift(
	//    image: np.ndarray, config: Dict[str, Any], feature_count: int
	//) -> Tuple[np.ndarray, np.ndarray]:
	//    from opensfm import pypopsift
	//

	double sift_edge_threshold = arg.sift_edge_threshold;
	double sift_peak_threshold = arg.sift_peak_threshold;

	//points, desc = pypopsift.popsift(image, peak_threshold=sift_peak_threshold,
	//                            edge_threshold=sift_edge_threshold,
	//                            target_num_features=feature_count,
	//                            use_root=bool(config["feature_root"]))
	
	//return points, desc
}

void Features::extract_features_surf(cv::Mat image, int feature_count)
{
	//def extract_features_surf(
	//    image: np.ndarray, config: Dict[str, Any], feature_count: int
	//) -> Tuple[np.ndarray, np.ndarray]:

	//surf_hessian_threshold = config["surf_hessian_threshold"]

	//detector = cv::FeatureDetector_create("SURF")
	//descriptor = cv::DescriptorExtractor_create("SURF")
	//detector.setDouble("hessianThreshold", surf_hessian_threshold)
	//detector.setDouble("nOctaves", config["surf_n_octaves"])
	//detector.setDouble("nOctaveLayers", config["surf_n_octavelayers"])
	//detector.setInt("upright", config["surf_upright"])

	int iter_ctr = 8;		// infinite loop protection
	while (true)
	{
        //Logger::Write(__FUNCTION__, ""Computing surf with threshold {0}".format(surf_hessian_threshold))
        //t = time.time()
        //if context.OPENCV3:
        //    detector.setHessianThreshold(surf_hessian_threshold)
        //else
        //    detector.setDouble(
        //        "hessianThreshold", surf_hessian_threshold
        //    )  # default: 0.04
        //points = detector.detect(image)
        //Logger::Write(__FUNCTION__, ""Found {0} points in {1}s".format(len(points), time.time() - t))
        //if len(points) < feature_count and surf_hessian_threshold > 0.0001:
        //    surf_hessian_threshold = (surf_hessian_threshold * 2) / 3
        //    Logger::Write(__FUNCTION__, ""reducing threshold")
        //else
        //    Logger::Write(__FUNCTION__, ""done")
        //    break

		if (--iter_ctr < 1)
			break;
	}

	//    points, desc = descriptor.compute(image, points)
	//
	//    if desc is not None:
	//        if config["feature_root"]:
	//            desc = root_feature(desc)
	//        points = np.array([(i.pt[0], i.pt[1], i.size, i.angle) for i in points])
	//    else
	//        points = np.array(np.zeros((0, 3)))
	//        desc = np.array(np.zeros((0, 3)))
	//    return points, desc
}

//def akaze_descriptor_type(name: str) -> pyfeatures.AkazeDescriptorType:
//    d = pyfeatures.AkazeDescriptorType.__dict__
//    if name in d:
//        return d[name]
//    else
//        Logger::Write(__FUNCTION__, ""Wrong akaze descriptor type")
//        return d["MSURF"]
//
void Features::extract_features_akaze(cv::Mat image, int feature_count)
{
	//def extract_features_akaze(
	//    image: np.ndarray, config: Dict[str, Any], feature_count: int
	//) -> Tuple[np.ndarray, np.ndarray]:

	//options = pyfeatures.AKAZEOptions()
	//options.omax = config["akaze_omax"]
	//akaze_descriptor_name = config["akaze_descriptor"]
	//options.descriptor = akaze_descriptor_type(akaze_descriptor_name)
	//options.descriptor_size = config["akaze_descriptor_size"]
	//options.descriptor_channels = config["akaze_descriptor_channels"]
	//options.dthreshold = config["akaze_dthreshold"]
	//options.kcontrast_percentile = config["akaze_kcontrast_percentile"]
	//options.use_isotropic_diffusion = config["akaze_use_isotropic_diffusion"]
	//options.target_num_features = feature_count
	//options.use_adaptive_suppression = config["feature_use_adaptive_suppression"]

	//Logger::Write(__FUNCTION__, ""Computing AKAZE with threshold {0}".format(options.dthreshold))
	//t = time.time()
	//points, desc = pyfeatures.akaze(image, options)
	//Logger::Write(__FUNCTION__, ""Found {0} points in {1}s".format(len(points), time.time() - t))
	
	//if config["feature_root"]:
	//    if akaze_descriptor_name in ["SURF_UPRIGHT", "MSURF_UPRIGHT"]:
	//        desc = root_feature_surf(desc, partial=True)
	//    else if akaze_descriptor_name in ["SURF", "MSURF"]:
	//        desc = root_feature_surf(desc, partial=False)
	//points = points.astype(float)
	//return points, desc
}

void Features::extract_features_hahog(cv::Mat image, int feature_count)
{
	//def extract_features_hahog(
	//    image: np.ndarray, config: Dict[str, Any], feature_count: int
	//) -> Tuple[np.ndarray, np.ndarray]:

	//t = time.time()
	//points, desc = pyfeatures.hahog(
	//    image.astype(np.float32) / 255,  # VlFeat expects pixel values between 0, 1
	//    peak_threshold=config["hahog_peak_threshold"],
	//    edge_threshold=config["hahog_edge_threshold"],
	//    target_num_features=feature_count,
	//)
	
	//if config["feature_root"]:
	//    desc = np.sqrt(desc)
	//    uchar_scaling = 362  # x * 512 < 256  =>  sqrt(x) * 362 < 256
	//else
	//    uchar_scaling = 512
	
	//if config["hahog_normalize_to_uchar"]:
	//    desc = (uchar_scaling * desc).clip(0, 255).round()
	
	//Logger::Write(__FUNCTION__, ""Found {0} points in {1}s".format(len(points), time.time() - t))
	//return points, desc
}

void Features::extract_features_orb(cv::Mat image, int feature_count)
{
	//def extract_features_orb(
	//    image: np.ndarray, config: Dict[str, Any], feature_count: int
	//) -> Tuple[np.ndarray, np.ndarray]:

	//detector = cv::FeatureDetector_create("ORB")
	//descriptor = cv::DescriptorExtractor_create("ORB")
	//detector.setDouble("nFeatures", feature_count)

	//Logger::Write(__FUNCTION__, ""Computing ORB")
	//t = time.time()
	//points = detector.detect(image)
	
	//points, desc = descriptor.compute(image, points)
	//if desc is not None:
	//    points = np.array([(i.pt[0], i.pt[1], i.size, i.angle) for i in points])
	//else
	//    points = np.array(np.zeros((0, 3)))
	//    desc = np.array(np.zeros((0, 3)))
	
	//Logger::Write(__FUNCTION__, ""Found {0} points in {1}s".format(len(points), time.time() - t))
	//return points, desc
}

//def build_flann_index(descriptors: np.ndarray, config: Dict[str, Any]) -> Any:
//    # FLANN_INDEX_LINEAR = 0
//    FLANN_INDEX_KDTREE = 1
//    FLANN_INDEX_KMEANS = 2
//    # FLANN_INDEX_COMPOSITE = 3
//    # FLANN_INDEX_KDTREE_SINGLE = 4
//    # FLANN_INDEX_HIERARCHICAL = 5
//    FLANN_INDEX_LSH = 6
//
//    if descriptors.dtype.type is np.float32:
//        algorithm_type = config["flann_algorithm"].upper()
//        if algorithm_type == "KMEANS":
//            FLANN_INDEX_METHOD = FLANN_INDEX_KMEANS
//        else if algorithm_type == "KDTREE":
//            FLANN_INDEX_METHOD = FLANN_INDEX_KDTREE
//        else
//            raise ValueError("Unknown flann algorithm type " "must be KMEANS, KDTREE")
//        flann_params = {
//            "algorithm": FLANN_INDEX_METHOD,
//            "branching": config["flann_branching"],
//            "iterations": config["flann_iterations"],
//            "tree": config["flann_tree"],
//        }
//    else if descriptors.dtype.type is np.uint8:
//        flann_params = {
//            "algorithm": FLANN_INDEX_LSH,
//            "table_number": 10,
//            "key_size": 24,
//            "multi_probe_level": 1,
//        }
//
//    return context.flann_Index(descriptors, flann_params)


cv::Mat Features::resized_image(cv::Mat image, int max_size)
{
	// Resize image to feature_process_size.
	// 

	int size = std::max(image.cols, image.rows);
	if (max_size < size)
	{
		double sf = (double)max_size / (double)size;
		cv::Size dsize;
		dsize.width = image.cols * sf;
		dsize.height = image.rows * sf;
		cv::resize(image, image, dsize, cv::INTER_AREA);
	}

	return image;
}

//def root_feature(desc: np.ndarray, l2_normalization: bool = False) -> np.ndarray:
//    if l2_normalization:
//        s2 = np.linalg.norm(desc, axis=1)
//        desc = (desc.T / s2).T
//    s = np.sum(desc, 1)
//    desc = np.sqrt(desc.T / s).T
//    return desc

//def root_feature_surf(
//    desc: np.ndarray, l2_normalization: bool = False, partial: bool = False
//) -> np.ndarray:
//    """
//    Experimental square root mapping of surf-like feature, only work for 64-dim surf now
//    """
//    if desc.shape[1] == 64:
//        if l2_normalization:
//            s2 = np.linalg.norm(desc, axis=1)
//            desc = (desc.T / s2).T
//        if partial:
//            ii = np.array([i for i in range(64) if (i % 4 == 2 or i % 4 == 3)])
//        else
//            ii = np.arange(64)
//        desc_sub = np.abs(desc[:, ii])
//        desc_sub_sign = np.sign(desc[:, ii])
//        # s_sub = np.sum(desc_sub, 1)  # This partial normalization gives slightly better results for AKAZE surf
//        s_sub = np.sum(np.abs(desc), 1)
//        desc_sub = np.sqrt(desc_sub.T / s_sub).T
//        desc[:, ii] = desc_sub * desc_sub_sign
//    return desc

//def normalized_image_coordinates(
//    pixel_coords: np.ndarray, width: int, height: int
//) -> np.ndarray:
//    size = max(width, height)
//    p = np.empty((len(pixel_coords), 2))
//    p[:, 0] = (pixel_coords[:, 0] + 0.5 - width / 2.0) / size
//    p[:, 1] = (pixel_coords[:, 1] + 0.5 - height / 2.0) / size
//    return p

//def denormalized_image_coordinates(
//    norm_coords: np.ndarray, width: int, height: int
//) -> np.ndarray:
//    size = max(width, height)
//    p = np.empty((len(norm_coords), 2))
//    p[:, 0] = norm_coords[:, 0] * size - 0.5 + width / 2.0
//    p[:, 1] = norm_coords[:, 1] * size - 0.5 + height / 2.0
//    return p

//def normalize_features(
//    points: np.ndarray, desc: np.ndarray, colors: np.ndarray, width: int, height: int
//) -> Tuple[np.ndarray, np.ndarray, np.ndarray,]:
//    """Normalize feature coordinates and size."""
//    points[:, :2] = normalized_image_coordinates(points[:, :2], width, height)
//    points[:, 2:3] /= max(width, height)
//    return points, desc, colors

//def _in_mask(point: np.ndarray, width: int, height: int, mask: np.ndarray) -> bool:
//    """Check if a point is inside a binary mask."""
//    u = mask.shape[1] * (point[0] + 0.5) / width
//    v = mask.shape[0] * (point[1] + 0.5) / height
//    return mask[int(v), int(u)] != 0

void Features::save(XString file_name)
{
	m_feat.save(file_name);
}

void Features::load(XString file_name)
{
	m_feat.load(file_name);
}

FeatureData::FeatureData()
{
    //points: np.ndarray
    //descriptors: Optional[np.ndarray]
    //colors: np.ndarray
    //semantic: Optional[SemanticData]

    //FEATURES_VERSION: int = 3
    //FEATURES_HEADER: str = "OPENSFM_FEATURES_VERSION"

    //def __init__(
    //    self,
    //    points: np.ndarray,
    //    descriptors: Optional[np.ndarray],
    //    colors: np.ndarray,
    //    semantic: Optional[SemanticData],
    //):
    //    self.points = points
    //    self.descriptors = descriptors
    //    self.colors = colors
    //    self.semantic = semantic
}

FeatureData::~FeatureData()
{

}

//    def get_segmentation(self) -> Optional[np.ndarray]:
//        semantic = self.semantic
//        if not semantic:
//            return None
//        if semantic.segmentation is not None:
//            return semantic.segmentation
//        return None
//
//    def has_instances(self) -> bool:
//        semantic = self.semantic
//        if not semantic:
//            return False
//        return semantic.instances is not None
//
//    def mask(self, mask: np.ndarray) -> "FeatureData":
//        if self.semantic:
//            masked_semantic = self.semantic.mask(mask)
//        else
//            masked_semantic = None
//        return FeatureData(
//            self.points[mask],
//            self.descriptors[mask] if self.descriptors is not None else None,
//            self.colors[mask],
//            masked_semantic,
//        )

void FeatureData::save(XString file_name)
{
    // Save features to file.
	//
	// Writes:
	//		keypoints
	//		keypoint descriptors
	//		colors at each keyponit
	//
    
	int feature_data_type = CV_32F;
	XString feature_type = arg.feature_type;
	if (((feature_type == "AKAZE") && (arg.akaze_descriptor == "MLDB_UPRIGHT" || arg.akaze_descriptor == "MLDB"))
		|| (arg.feature_type == "HAHOG" && arg.hahog_normalize_to_uchar)
		|| (feature_type == "ORB"))
		feature_data_type = CV_8U;
	else
		feature_data_type = CV_32F;
    
	if (m_desc.empty())
	{
		Logger::Write(__FUNCTION__, "No descriptors found, cannot save features data.");
		assert(false);
	}

	FILE* pFile = fopen(file_name.c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "%s: %d\n", SECT_KEYPOINT, (int)m_kp.size());
		for (auto f : m_kp)
		{
			fprintf(pFile, "%0.3f %0.3f %0.3f %0.3f %0.3f %d %d\n",
				f.pt.x, f.pt.y,		// 2D coordinates(x, y) of the keypoint in the image.
				f.size,				// diameter of the keypoint's neighborhood, indicating the scale at which the keypoint was detected.
				f.angle,			// orientation of the keypoint, typically in degrees.
				f.response,			// measure of the keypoint's strength or distinctiveness.
				f.octave,			// octave(pyramid level) at which the keypoint was detected, relevant for scale - space feature detectors.
				f.class_id);		// optional ID to group keypoints from different objects or classes.
		}

		fprintf(pFile, "%s: %dx%d\n", SECT_DESC, m_desc.rows, m_desc.cols);
		for (int r = 0; r < m_desc.rows; ++r)
		{
			for (int c = 0; c < m_desc.cols; ++c)
			{
				fprintf(pFile, "%0.3f ", m_desc.at <float>(r, c));
			}
			fprintf(pFile, "\n");
		}

		fprintf(pFile, "%s: %d\n", SECT_COLOR, (int)m_colors.size());
		for (auto c : m_colors)
		{
			fprintf(pFile, "%d %d %d\n", c.R, c.B, c.G);
		}

		fclose(pFile);
	}
}

void FeatureData::load(XString file_name)
{
	// Load features written by save() method.
	//

	enum class Sect
	{
		None,
		KeyPoints,
		Desc,
		Color
	};
	Sect cur_sect = Sect::None;

	m_kp.clear();
	m_desc = cv::Mat();
	m_colors.clear();
	int desc_rows = 0;
	int desc_cols = 0;
	int cur_desc_row = 0;

	FILE* pFile = fopen(file_name.c_str(), "rt");
	if (pFile)
	{
		char buf[2048] = { 0 };
		while (fgets(buf, sizeof(buf), pFile))
		{
			XString line = buf;
			line.Trim("\n\r");

			int token_count = line.Tokenize(" :");
			if (line.BeginsWith(SECT_KEYPOINT))
			{
				cur_sect = Sect::KeyPoints;
			}
			else if (line.BeginsWith(SECT_DESC))
			{
				cur_sect = Sect::Desc;
				desc_rows = line.GetToken(1).GetInt();
				desc_cols = line.GetToken(2).GetInt();
				m_desc = cv::Mat(desc_rows, desc_cols, CV_32F);
			}
			else if (line.BeginsWith(SECT_COLOR))
			{
				cur_sect = Sect::Color;
			}
			else
			{
				switch (cur_sect) {
				case Sect::KeyPoints:
					{
						cv::KeyPoint kp;
						kp.pt.x = line.GetToken(0).GetDouble();
						kp.pt.y = line.GetToken(1).GetDouble();
						kp.size = line.GetToken(2).GetDouble();
						kp.angle = line.GetToken(3).GetDouble();
						kp.response = line.GetToken(4).GetDouble();
						kp.octave = line.GetToken(5).GetInt();
						kp.class_id = line.GetToken(6).GetInt();
						m_kp.push_back(kp);
					}
					break;
				case Sect::Desc:
					{
						// each line is a row of the descriptor matrix
						for (int c = 0; c < desc_cols; ++c)
						{
							m_desc.at <float>(cur_desc_row, c) = line.GetToken(c).GetDouble();
						}
						++cur_desc_row;
					}
					break;
				case Sect::Color:
					{
						PixelType c;
						c.R = line.GetToken(0).GetInt();
						c.G = line.GetToken(1).GetInt();
						c.B = line.GetToken(2).GetInt();
						m_colors.push_back(c);
					}
					break;
				}
			}
		}

		fclose(pFile);
	}
}
