#ifndef FEATURES_H
#define FEATURES_H

#include <vector>

#include <opencv2/opencv.hpp>	// OpenCV

#include "Calc.h"
#include "XString.h"

class FeatureData
{
public:

	std::vector<cv::KeyPoint> m_kp;
	cv::Mat m_desc;
	std::vector<PixelType> m_colors;

	//    FEATURES_VERSION: int = 3
	//    FEATURES_HEADER: str = "OPENSFM_FEATURES_VERSION"

public:

	FeatureData();
	~FeatureData();

	void save(XString file_name);
	void load(XString file_name);
};

class Features
{
public:

	cv::Mat m_image;		// input image
	FeatureData m_feat;

public:

	Features();
	~Features();

	void detect(XString file_name);

	void save(XString file_name);
	void load(XString file_name);

private:

	void extract_features(cv::Mat image, bool is_panorama);
	void extract_features_sift(cv::Mat image, int feature_count);
	void extract_features_popsift(cv::Mat image, int feature_count);
	void extract_features_surf(cv::Mat image, int feature_count);
	void extract_features_akaze(cv::Mat image, int feature_count);
	void extract_features_hahog(cv::Mat image, int feature_count);
	void extract_features_orb(cv::Mat image, int feature_count);

	cv::Mat resized_image(cv::Mat image, int max_size);
};

#endif // #ifndef FEATURES_H
