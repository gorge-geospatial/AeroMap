// Multispectral.cpp
// Port of odm multispectral.py
//

#include <algorithm>

#include "Multispectral.h"

//from skimage import exposure
//from skimage.morphology import disk
//from skimage.filters import rank, gaussian

const int LOW_RES = 320;        // resolution below which image is considered low resolution

XString Multispectral::get_primary_band_name(Reconstruction::MultiType multi_camera, XString user_band_name)
{
    if (multi_camera.size() < 1)
    {
        Logger::Write(__FUNCTION__, "Invalid multi_camera list");
        assert(false);
    }

    // multi_camera is already sorted by band_index
    if (user_band_name == "auto")
        return multi_camera[0].band_name;

    for (auto band : multi_camera)
    {
        if (band.band_name.CompareNoCase(user_band_name))
            return band.band_name;
    }

    XString band_name_fallback = multi_camera[0].band_name;
    Logger::Write(__FUNCTION__, "Cannot find band name '%s', will use '%s' instead", user_band_name.c_str(), band_name_fallback.c_str());

    return band_name_fallback;
}

std::vector<Photo*> Multispectral::get_photos_by_band(Reconstruction::MultiType multi_camera, XString user_band_name)
{
    XString band_name = get_primary_band_name(multi_camera, user_band_name);

    for (auto band : multi_camera)
    {
        if (band.band_name == band_name)
            return band.photos;
    }

    return std::vector<Photo*>();
}

void Multispectral::compute_band_maps(Reconstruction::MultiType multi_camera, XString primary_band, s2p_type& s2p, p2s_type& p2s)
{
    // Computes maps of:
    //     - photo filename --> associated primary band photo (s2p)
    //     - primary band filename --> list of associated secondary band photos (p2s)
    // by looking at capture UUID, capture time or filenames as a fallback
    //
    
    XString band_name = get_primary_band_name(multi_camera, primary_band);

    std::vector<Photo*> primary_band_photos;
    for (auto band : multi_camera)
    {
        if (band.band_name == band_name)
        {
            primary_band_photos = band.photos;
            break;
        }
    }

    // Try using capture time as the grouping factor
    
    bool use_file_names = false;
    std::map<std::string, Photo*> unique_id_map;
    for (auto p : primary_band_photos)
    {
        std::string uuid = p->GetCaptureUUID().c_str();
        if (uuid.empty())
        {
            Logger::Write(__FUNCTION__, "Cannot use capture time (no information in %s)", p->GetFileName().c_str());
            use_file_names = true;
            break;
        }

        // Should be unique across primary band
        if (unique_id_map.find(uuid) != unique_id_map.end())
        {
            Logger::Write(__FUNCTION__, "Duplicate UUID: %s", uuid.c_str());
            use_file_names = true;
            break;
        }

        unique_id_map[uuid] = p;
    }

    if (use_file_names == false)
    {
        for (auto band : multi_camera)
        {
            for (auto p : band.photos)
            {
                std::string uuid = p->GetCaptureUUID().c_str();
                if (uuid.empty())
                {
                    Logger::Write(__FUNCTION__, "Cannot use capture time (no information in %s)", p->GetFileName().c_str());
                    use_file_names = true;
                    break;
                }

                // Should match the primary band
                if (unique_id_map.find(uuid) == unique_id_map.end())
                {
                    Logger::Write(__FUNCTION__, "Unreliable UUID/capture time detected (no primary band match)");
                    use_file_names = true;
                    break;
                }

                s2p[p->GetFileName().c_str()] = unique_id_map[uuid];

                if (band.band_name != band_name)
                {
                    std::string file_name = unique_id_map[uuid]->GetFileName().c_str();
                    if (p2s.find(file_name) == p2s.end())
                        p2s[file_name] = std::vector<Photo*>();
                    p2s[file_name].push_back(p);
                }
            }
        }
    }

    //TODO:
    assert(use_file_names == false);
    // if use_file_names
    //catch (std::exception ex)
    //    # Fallback on filename conventions
    //    Logger::Write(__FUNCTION__, "%s, will use filenames instead" % str(e))

    //    filename_map = {}
    //    s2p = {}
    //    p2s = {}
    //    file_regex = re.compile(r"^(.+)[-_]\w+(\.[A-Za-z]{3,4})$")

    //    for p in primary_band_photos:
    //        filename_without_band = re.sub(file_regex, "\\1\\2", p.filename)

    //        # Quick check
    //        if filename_without_band == p.filename:
    //            raise Exception("Cannot match bands by filename on %s, make sure to name your files [filename]_band[.ext] uniformly." % p.filename)

    //        filename_map[filename_without_band] = p

    //    for band in multi_camera:
    //        photos = band['photos']

    //        for p in photos:
    //            filename_without_band = re.sub(file_regex, "\\1\\2", p.filename)

    //            # Quick check
    //            if filename_without_band == p.filename:
    //                raise Exception("Cannot match bands by filename on %s, make sure to name your files [filename]_band[.ext] uniformly." % p.filename)

    //            s2p[p.filename] = filename_map[filename_without_band]

    //            if band['name'] != band_name:
    //                p2s.setdefault(filename_map[filename_without_band].filename, []).append(p)
}

Multispectral::align_info_type Multispectral::compute_alignment_matrices(Reconstruction::MultiType multi_camera, XString primary_band_name, XString images_path,
    Multispectral::s2p_type s2p, Multispectral::p2s_type p2s, int max_concurrency, int max_samples)
{
    Logger::Write(__FUNCTION__, "Computing band alignment");

    align_info_type alignment_info;

    // For each secondary band
    for (auto band : multi_camera)
    {
        if (band.band_name == primary_band_name)
            continue;
       
        std::vector<align_type> matrices;

        //TODO:
        //parallel loop
        for (auto p : band.photos)
        {
            try
            {
                if (matrices.size() >= max_samples)
                {
                    Logger::Write(__FUNCTION__, "Got enough samples for %s (%d)", band.band_name.c_str(), max_samples);
                    break;
                }

                // Find good matrix candidates for alignment

                Photo* primary_band_photo = s2p[p->GetFileName().c_str()];
                if (primary_band_photo == nullptr)
                {
                    Logger::Write(__FUNCTION__, "Cannot find primary band photo for '%s'", p->GetFileName().c_str());
                    break;
                }

                align_type align = compute_homography(p->GetFileName(), primary_band_photo->GetFileName());

                if (align.warp_matrix.empty() == false)
                {
                    Logger::Write(__FUNCTION__, "%s --> %s good match", p->GetFileName().c_str(), primary_band_photo->GetFileName().c_str());

                    matrices.push_back(align);
                }
                else
                {
                    Logger::Write(__FUNCTION__, "%s --> %s cannot be matched", p->GetFileName().c_str(), primary_band_photo->GetFileName().c_str());
                }
            }
            catch (std::exception ex)
            {
                Logger::Write(__FUNCTION__, "Failed to compute homography for '%s': %s", p->GetFileName().c_str(), ex.what());
            }
        }
        
        // Find the matrix that has the most common eigvals
        // among all matrices. That should be the "best" alignment.
        for (auto m1 : matrices)
        {
            //acc = np.array([0.0,0.0,0.0])
            //e = m1['eigvals']

            //for m2 in matrices:
            //    acc += abs(e - m2['eigvals'])

            //m1['score'] = acc.sum()
        }

        // Sort
        std::sort(matrices.begin(), matrices.end());

        if (matrices.size() > 0)
        {
            alignment_info[band.band_name.c_str()] = matrices[0];
            Logger::Write(__FUNCTION__, "%s band will be aligned using warp matrix with score: %0.6f)", band.band_name.c_str(), matrices[0].score);
        }
        else
        {
            Logger::Write(__FUNCTION__, "Cannot find alignment matrix for band %s, The band might end up misaligned.", band.band_name.c_str());
        }
    }

    return alignment_info;
}

Multispectral::align_type Multispectral::compute_homography(XString image_filename, XString align_image_filename)
{
    align_type align;       // return value

    try
    {
        // Convert images to grayscale if needed
        cv::Mat image;
        cv::imread(image_filename.c_str(), image, cv::IMREAD_UNCHANGED);
        cv::Mat image_gray = to_8bit(image);

        int max_dim = std::max(image_gray.cols, image_gray.rows);
        if (max_dim <= LOW_RES)
            Logger::Write(__FUNCTION__, "Small image for band alignment (%dx%d), this might be tough to compute.", image_gray.cols, image_gray.rows);

        cv::Mat align_image;
        cv::imread(align_image_filename.c_str(), align_image, cv::IMREAD_UNCHANGED);
        cv::Mat align_image_gray = to_8bit(align_image);

        cv::Mat result;
        if (max_dim > LOW_RES)
        {
            align.algo = "feat";
            result = compute_using_features(image_gray, align_image_gray);

            if (result.empty())
            {
                align.algo = "ecc";
                Logger::Write(__FUNCTION__, "Can't use feature matching, will use ECC (this might take a bit)");
                result = compute_using_ecc(image_gray, align_image_gray);
                if (result.empty())
                    align.algo = "";
            }
        }
        else        // ECC only for low resolution images
        {
            align.algo = "ecc";
            Logger::Write(__FUNCTION__, "Using ECC (this might take a bit)");
            result = compute_using_ecc(image_gray, align_image_gray);
            if (result.empty())
                align.algo = "";
        }

        align.warp_matrix = result;
    }
    catch (std::exception ex)
    {
        Logger::Write(__FUNCTION__, "Compute homography: %s", ex.what());
        align.algo = "";
    }

    return align;
}

cv::Mat Multispectral::compute_using_features(cv::Mat image_gray, cv::Mat align_image_gray)
{
    cv::Mat h;
    try
    {
        h = find_features_homography(image_gray, align_image_gray);
    }
    catch (std::exception ex)
    {
        Logger::Write(__FUNCTION__, "Cannot compute homography: %s", ex.what());
        return cv::Mat();
    }

    if (h.empty())
        return cv::Mat();

    //det = np.linalg.det(h)

    //# Check #1 homography's determinant will not be close to zero
    //if abs(det) < 0.25:
    //    return None, (None, None)

    //# Check #2 the ratio of the first-to-last singular value is sane (not too high)
    //svd = np.linalg.svd(h, compute_uv=False)
    //if svd[-1] == 0:
    //    return None, (None, None)

    //ratio = svd[0] / svd[-1]
    //if ratio > 100000:
    //    return None, (None, None)

    return h;
}

cv::Mat Multispectral::compute_using_ecc(cv::Mat image_gray, cv::Mat align_image_gray)
{
    cv::Mat h;
    try
    {
        h = find_ecc_homography(image_gray, align_image_gray);
    }
    catch (std::exception ex)
    {
        Logger::Write(__FUNCTION__, "Cannot compute homography: %s", ex.what());
        return cv::Mat();
    }

    if (h.empty())
        return cv::Mat();

    //det = np.linalg.det(h)

    //# Check #1 homography's determinant will not be close to zero
    //if abs(det) < 0.25:
    //    return None, (None, None)

    //# Check #2 the ratio of the first-to-last singular value is sane (not too high)
    //svd = np.linalg.svd(h, compute_uv=False)
    //if svd[-1] == 0:
    //    return None, (None, None)

    //ratio = svd[0] / svd[-1]
    //if ratio > 100000:
    //    return None, (None, None)

    return h;
}

cv::Mat Multispectral::find_features_homography(cv::Mat image_gray, cv::Mat align_image_gray, double feature_retention, double min_match_count)
{
    // Detect SIFT features and compute descriptors.
    double edgeThreshold = 10.0;
    double contrastThreshold = 0.1;
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create(0, 3, contrastThreshold, edgeThreshold);
    
    std::vector<cv::KeyPoint> kp_image, kp_align_image;
    cv::Mat desc_image, desc_align_image;
    sift->detectAndCompute(image_gray, cv::noArray(), kp_image, desc_image);
    sift->detectAndCompute(align_image_gray, cv::noArray(), kp_align_image, desc_align_image);

    // Match
   
    cv::Ptr<cv::flann::IndexParams> indexParams = cv::makePtr<cv::flann::KDTreeIndexParams>(5); // 5 trees
    cv::Ptr<cv::flann::SearchParams> searchParams = cv::makePtr<cv::flann::SearchParams>(50); // checks
    cv::FlannBasedMatcher matcher(indexParams, searchParams);
    
    std::vector<std::vector<cv::DMatch>> matches;
    try
    {
        matcher.knnMatch(desc_image, desc_align_image, matches, 2);
    }
    catch (std::exception ex)
    {
        Logger::Write(__FUNCTION__, "knnMatch() failed: %s", ex.what());
        return cv::Mat();
    }

    // Filter good matches following Lowe's ratio test
    std::vector<cv::DMatch> good_matches;
    for (size_t i = 0; i < matches.size(); i++)
    {
        if (matches[i][0].distance < feature_retention * matches[i][1].distance)
        {
            good_matches.push_back(matches[i][0]);
        }
    }
    
    if (good_matches.size() < min_match_count)
        return cv::Mat();

    // Debug
    // imMatches = cv2.drawMatches(im1, kp_image, im2, kp_align_image, matches, None)
    // cv2.imwrite("matches.jpg", imMatches)

    // Extract location of good matches
    std::vector<cv::Point2f> points_image;
    std::vector<cv::Point2f> points_align_image;

    for (size_t i = 0; i < good_matches.size(); i++)
    {
        points_image.push_back(kp_image[good_matches[i].queryIdx].pt);
        points_align_image.push_back(kp_align_image[good_matches[i].trainIdx].pt);
    }

    // Find homography
    cv::Mat h = cv::findHomography(points_image, points_align_image, cv::RANSAC);
    return h;
}

cv::Mat Multispectral::find_ecc_homography(cv::Mat image_gray, cv::Mat align_image_gray, int number_of_iterations, double termination_eps, double start_eps)
{
    cv::Mat warp_matrix;        // return value

    int pyramid_levels = 0;
    int w = image_gray.cols;
    int h = image_gray.rows;
    int max_dim = std::max(w, h);

    const int max_size = 1280;

    if (max_dim > max_size)
    {
        double sf = 1.0;
        if (max_dim == w)
            sf = (double)max_size / (double)w;
        else
            sf = (double)max_size / (double)h;
        cv::resize(image_gray, image_gray, cv::Size(), sf, sf, cv::INTER_AREA);
        w = image_gray.cols;
        h = image_gray.rows;
    }

    int min_dim = std::min(w, h);

    while (min_dim > 300)
    {
        min_dim /= 2.0;
        ++pyramid_levels;
    }

    Logger::Write(__FUNCTION__, "Pyramid levels: %d", pyramid_levels);

    // Quick check on size
    if ((align_image_gray.cols != image_gray.cols) || (align_image_gray.rows != image_gray.rows))
    {
		align_image_gray = to_8bit(align_image_gray);
		image_gray = to_8bit(image_gray);

        double fx = (double)image_gray.cols / (double)align_image_gray.cols;
        double fy = (double)image_gray.rows / (double)align_image_gray.rows;

        int interp = cv::INTER_LANCZOS4;
        if ((fx < 1.0) && (fy < 1.0))
            interp = cv::INTER_AREA;

        cv::resize(align_image_gray, align_image_gray, cv::Size(), fx, fy, interp);
    }

    // Build pyramids
    std::vector<cv::Mat> image_gray_pyr;
    std::vector<cv::Mat> align_image_pyr;
    image_gray_pyr.push_back(image_gray);
    align_image_pyr.push_back(align_image_gray);

    for (int level = 0; level < pyramid_levels; ++level)
    {
        cv::Mat tmp;

        image_gray_pyr[0] = to_8bit(image_gray_pyr[0], true);
        cv::resize(image_gray_pyr[0], tmp, cv::Size(), 0.5, 0.5, cv::INTER_AREA);
        image_gray_pyr.insert(image_gray_pyr.begin(), tmp);

        align_image_pyr[0] = to_8bit(align_image_pyr[0], true);
        cv::resize(align_image_pyr[0], tmp, cv::Size(), 0.5, 0.5, cv::INTER_AREA);
        align_image_pyr.insert(align_image_pyr.begin(), tmp);
    }

    // Define the motion model, scale the initial warp matrix to smallest level
    // create matrix:
    //      [1,   1,   2]
    //      [1,   1,   2]
    //      [0.5, 0.5, 1]
    cv::Mat base = cv::Mat(3, 3, CV_32F);
    base.at <float>(0, 0) = 1.0; base.at <float>(0, 1) = 1.0; base.at <float>(0, 2) = 2.0;
    base.at <float>(1, 0) = 1.0; base.at <float>(1, 1) = 1.0; base.at <float>(1, 2) = 2.0;
    base.at <float>(2, 0) = 0.5; base.at <float>(2, 1) = 0.5; base.at <float>(2, 2) = 1.0;
    double exponent = (double)(1 - (pyramid_levels + 1));
    cv::Mat base_exp;
    cv::pow(base, exponent, base_exp);
    //warp_matrix = warp_matrix * np.array([[1,1,2],[1,1,2],[0.5,0.5,1]], dtype=np.float32)**(1-(pyramid_levels+1))
    warp_matrix = cv::Mat::eye(3, 3, CV_32F);
    warp_matrix *= base_exp;

    for (int level = 0; level < pyramid_levels + 1; ++level)
    {
        cv::Mat ig = image_gray_pyr[level];
        cv::Mat aig = align_image_pyr[level];
        //TODO:
        //ig = gradient(gaussian(image_gray_pyr[level]))
        //aig = gradient(gaussian(align_image_pyr[level]))

        double eps;
        if (level == pyramid_levels && pyramid_levels == 0)
            eps = termination_eps;
        else
            eps = start_eps - ((start_eps - termination_eps) / (pyramid_levels)) * level;

        // Define termination criteria
        cv::TermCriteria criteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, number_of_iterations, eps);

        try
        {
            Logger::Write(__FUNCTION__, "Computing ECC pyramid level %d", level);
            int gauss_filter_size = 9;
            double ecc = cv::findTransformECC(ig, aig, warp_matrix, cv::MOTION_HOMOGRAPHY, criteria, cv::noArray(), gauss_filter_size);
        }
        catch (std::exception ex)
        {
            if (level != pyramid_levels)
            {
                Logger::Write(__FUNCTION__, "Could not compute ECC warp_matrix at pyramid level %d, resetting matrix", level);
                //warp_matrix = warp_matrix * np.array([[1,1,2],[1,1,2],[0.5,0.5,1]], dtype=np.float32)**(1-(pyramid_levels+1))
                warp_matrix = cv::Mat::eye(3, 3, CV_32F);
                warp_matrix *= base_exp;
            }
            else
            {
                Logger::Write(__FUNCTION__, "cv::findTransformECC() failed: %s", ex.what());
                assert(false);
            }
        }

        if (level != pyramid_levels)
            warp_matrix *= base;
    }


    return warp_matrix;
}

cv::Mat Multispectral::gradient(cv::Mat im, int ksize)
{
    cv::Mat grad;
    im = local_normalize(im);
    //grad_x = cv2.Sobel(im,cv2.CV_32F,1,0,ksize=ksize)
    //grad_y = cv2.Sobel(im,cv2.CV_32F,0,1,ksize=ksize)
    //grad = cv2.addWeighted(np.absolute(grad_x), 0.5, np.absolute(grad_y), 0.5, 0)
    return grad;
}

cv::Mat Multispectral::local_normalize(cv::Mat im)
{
    //width, _ = im.shape
    //disksize = int(width/5)
    //if disksize % 2 == 0:
    //    disksize = disksize + 1
    //selem = disk(disksize)
    //im = rank.equalize(im, selem=selem)
    return im;
}

//def align_image(image, warp_matrix, dimension):
//    image = resize_match(image, dimension)
//
//    if warp_matrix.shape == (3, 3):
//        return cv2.warpPerspective(image, warp_matrix, dimension)
//    else
//        return cv2.warpAffine(image, warp_matrix, dimension)

cv::Mat Multispectral::to_8bit(cv::Mat image, bool force_normalize)
{
    if (force_normalize == false && image.type() == CV_8U)
        return image;

    // convert to grayscale if needed
    if (image.channels() == 3)
        cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);

    // Convert to 8bit
    
    double min_val = 0.0;
    double max_val = 0.0;
    cv::minMaxLoc(image, &min_val, &max_val);
    double value_range = max_val - min_val;

    image.convertTo(image, CV_32F);
    image -= min_val;
    image *= 255.0 / value_range;
    //image[image > 255] = 255
    //image[image < 0] = 0
    image.convertTo(image, CV_8U);

    return image;
}

cv::Mat Multispectral::resize_match(cv::Mat image, SizeType dimension)
{
    int w = image.cols;
    int h = image.rows;
    int mw = dimension.cx;
    int mh = dimension.cy;

    if (w != mw || h != mh)
    {
        double fx = (double)mw / (double)w;
        double fy = (double)mh / (double)h;

        int interp = cv::INTER_LANCZOS4;
        if ((fx < 1.0) && (fy < 1.0))
            interp = cv::INTER_AREA;
        
        cv::resize(image, image, cv::Size(), fx, fy, interp);
    }

    return image;
}

// Loosely based on https://github.com/micasense/imageprocessing/blob/master/micasense/utils.py

cv::Mat Multispectral::dn_to_radiance(Photo* photo, cv::Mat image)
{
    // Convert Digital Number values to Radiance values
    // 
    // Inputs:
    //      photo ODM_Photo
    //      image numpy array containing image data
    // Outputs:
    //      return numpy array with radiance image values
    //
    
    //image = image.astype("float32")
    //if len(image.shape) != 3:
    //    raise ValueError("Image should have shape length of 3 (got: %s)" % len(image.shape))
    
    //# Thermal (this should never happen, but just in case..)
    //if photo.is_thermal():
    //    return image
    
    //# All others
    //a1, a2, a3 = photo.get_radiometric_calibration()
    //dark_level = photo.get_dark_level()
    
    //exposure_time = photo.exposure_time
    //gain = photo.get_gain()
    //gain_adjustment = photo.gain_adjustment
    //photometric_exp = photo.get_photometric_exposure()
    
    //if a1 is None and photometric_exp is None:
    //    Logger::Write(__FUNCTION__, "Cannot perform radiometric calibration, no FNumber/Exposure Time or Radiometric Calibration EXIF tags found in %s. Using Digital Number." % photo.filename)
    //    return image
    
    //if a1 is None and photometric_exp is not None:
    //    a1 = photometric_exp
    
    //V, x, y = vignette_map(photo)
    //if x is None:
    //    x, y = np.meshgrid(np.arange(photo.width), np.arange(photo.height))
    
    //if dark_level is not None:
    //    image -= dark_level
    
    //# Normalize DN to 0 - 1.0
    //bit_depth_max = photo.get_bit_depth_max()
    //if bit_depth_max:
    //    image /= bit_depth_max
    //else
    //    Logger::Write(__FUNCTION__, "Cannot normalize DN for %s, bit depth is missing" % photo.filename)
    
    //if V is not None:
    //    # vignette correction
    //    V = np.repeat(V[:, :, np.newaxis], image.shape[2], axis=2)
    //    image *= V
    
    //if exposure_time and a2 is not None and a3 is not None:
    //    # row gradient correction
    //    R = 1.0 / (1.0 + a2 * y / exposure_time - a3 * y)
    //    R = np.repeat(R[:, :, np.newaxis], image.shape[2], axis=2)
    //    image *= R
    
    //# Floor any negative radiances to zero (can happen due to noise around blackLevel)
    //if dark_level is not None:
    //    image[image < 0] = 0
    
    //# apply the radiometric calibration - i.e. scale by the gain-exposure product and
    //# multiply with the radiometric calibration coefficient
    
    //if gain is not None and exposure_time is not None:
    //    image /= (gain * exposure_time)
    
    //image *= a1
    
    //if gain_adjustment is not None:
    //    image *= gain_adjustment

    return image;
}

//def vignette_map(photo):
//    x_vc, y_vc = photo.get_vignetting_center()
//    polynomial = photo.get_vignetting_polynomial()
//
//    if x_vc and polynomial:
//        # append 1., so that we can call with numpy polyval
//        polynomial.append(1.0)
//        vignette_poly = np.array(polynomial)
//
//        # perform vignette correction
//        # get coordinate grid across image
//        x, y = np.meshgrid(np.arange(photo.width), np.arange(photo.height))
//
//        # meshgrid returns transposed arrays
//        # x = x.T
//        # y = y.T
//
//        # compute matrix of distances from image center
//        r = np.hypot((x - x_vc), (y - y_vc))
//
//        # compute the vignette polynomial for each distance - we divide by the polynomial so that the
//        # corrected image is image_corrected = image_original * vignetteCorrection
//        vignette = np.polyval(vignette_poly, r)
//
//        # DJI is special apparently
//        if photo.camera_make != "DJI":
//            vignette = 1.0 / vignette
//
//        return vignette, x, y
//
//    return None, None, None

cv::Mat Multispectral::dn_to_reflectance(Photo* photo, cv::Mat image, bool use_sun_sensor)
{
    cv::Mat radiance = dn_to_radiance(photo, image);
    double irradiance = compute_irradiance(photo, use_sun_sensor);
    return radiance * PI / irradiance;
}

double Multispectral::compute_irradiance(Photo* photo, bool use_sun_sensor)
{
    // Thermal (this should never happen, but just in case..)
    if (photo->is_thermal())
        return 1.0;

    //# Some cameras (Micasense, DJI) store the value (nice! just return)
    //hirradiance = photo.get_horizontal_irradiance()
    //if hirradiance is not None:
    //    return hirradiance

    //# TODO: support for calibration panels

    //if use_sun_sensor and photo.get_sun_sensor():
    //    # Estimate it
    //    dls_orientation_vector = np.array([0,0,-1])
    //    sun_vector_ned, sensor_vector_ned, sun_sensor_angle, \
    //    solar_elevation, solar_azimuth = dls.compute_sun_angle([photo.latitude, photo.longitude],
    //                                    photo.get_dls_pose(),
    //                                    photo.get_utc_time(),
    //                                    dls_orientation_vector)

    //    angular_correction = dls.fresnel(sun_sensor_angle)

    //    # TODO: support for direct and scattered irradiance

    //    direct_to_diffuse_ratio = 6.0 # Assumption, clear skies
    //    spectral_irradiance = photo.get_sun_sensor()

    //    percent_diffuse = 1.0 / direct_to_diffuse_ratio
    //    sensor_irradiance = spectral_irradiance / angular_correction

    //    # Find direct irradiance in the plane normal to the sun
    //    untilted_direct_irr = sensor_irradiance / (percent_diffuse + np.cos(sun_sensor_angle))
    //    direct_irradiance = untilted_direct_irr
    //    scattered_irradiance = untilted_direct_irr * percent_diffuse

    //    # compute irradiance on the ground using the solar altitude angle
    //    horizontal_irradiance = direct_irradiance * np.sin(solar_elevation) + scattered_irradiance
    //    return horizontal_irradiance
    //elif use_sun_sensor:
    //    Logger::Write(__FUNCTION__, "No sun sensor values found for %s" % photo.filename)

    return 1.0;
}
