#ifndef THERMAL_H
#define THERMAL_H

#include <opencv2/opencv.hpp>	// OpenCV

#include "Photo.h"

class Thermal
{
public:

	static cv::Mat resize_to_match(cv::Mat image, Photo* match_photo = nullptr);
	static cv::Mat dn_to_temperature(Photo* photo, cv::Mat image, XString images_path);
};

#endif // #ifndef THERMAL_H
