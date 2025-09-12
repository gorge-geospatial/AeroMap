#ifndef MULTISPECTRAL_H
#define MULTISPECTRAL_H

#include <opencv2/opencv.hpp>	// OpenCV

#include <map>

#include "Calc.h"
#include "XString.h"
#include "Reconstruction.h"

class Multispectral
{
public:

	struct align_type
	{
		cv::Mat warp_matrix;
		std::vector<double> eigvals;
		SizeType dimension;
		XString algo;
		double score;

		align_type()
			: dimension(0,0)
			, score(0.0)
		{
		}

		// Overload operator< for default sorting by score
		bool operator<(const align_type& other) const {
			return score < other.score;
		}
	};

	typedef std::map<std::string, align_type> align_info_type;

	typedef std::map<std::string, Photo*> s2p_type;					// secondary -> primary map
	typedef std::map<std::string, std::vector<Photo*>> p2s_type;	// primary -> secondary map

public:

	static void compute_band_maps(Reconstruction::MultiType multi_camera, XString primary_band, s2p_type& s2p, p2s_type& p2s);
	static XString get_primary_band_name(Reconstruction::MultiType multi_camera, XString user_band_name);
	static std::vector<Photo*> get_photos_by_band(Reconstruction::MultiType multi_camera, XString user_band_name);

	static align_info_type compute_alignment_matrices(Reconstruction::MultiType multi_camera, XString primary_band_name, XString images_path,
		Multispectral::s2p_type s2p, Multispectral::p2s_type p2s, int max_concurrency = 1, int max_samples = 30);

	static align_type compute_homography(XString image_filename, XString align_image_filename);

	static cv::Mat find_features_homography(cv::Mat image_gray, cv::Mat align_image_gray, double feature_retention = 0.7, double min_match_count = 10);
	static cv::Mat find_ecc_homography(cv::Mat image_gray, cv::Mat align_image_gray, int number_of_iterations = 1000, double termination_eps = 1e-8, double  start_eps = 1e-4);
	static cv::Mat compute_using_features(cv::Mat image_gray, cv::Mat align_image_gray);
	static cv::Mat compute_using_ecc(cv::Mat image_gray, cv::Mat align_image_gray);

	static cv::Mat to_8bit(cv::Mat image, bool force_normalize = false);
	static cv::Mat resize_match(cv::Mat image, SizeType dimension);

	static cv::Mat gradient(cv::Mat im, int ksize = 5);
	static cv::Mat local_normalize(cv::Mat im);

	static cv::Mat dn_to_radiance(Photo* photo, cv::Mat image);
	static cv::Mat dn_to_reflectance(Photo* photo, cv::Mat image, bool use_sun_sensor = true);
	static double compute_irradiance(Photo* photo, bool use_sun_sensor = true);
};

#endif // #ifndef MULTISPECTRAL_H
