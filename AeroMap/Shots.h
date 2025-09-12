#ifndef SHOTS_H
#define SHOTS_H

#include <opencv2/opencv.hpp>	// OpenCV

class Shots
{
public:

	static cv::Mat get_rotation_matrix(cv::Vec3d vec_rot);
	static cv::Vec3d get_origin(cv::Vec3d vec_rot, cv::Vec3d vec_trx);
};

#endif // #ifndef SHOTS_H
