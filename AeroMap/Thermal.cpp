// Thermal.cpp
// Port of odm thermal.py
//

#include "Thermal.h"

//from opendm.thermal_tools import dji_unpack
//from opendm.exiftool import extract_raw_thermal_image_data
//from opendm.thermal_tools.thermal_utils import sensor_vals_to_temp

cv::Mat Thermal::resize_to_match(cv::Mat image, Photo* match_photo)
{
	// Resize images to match the dimension of another photo
	// 
	// Inputs:
	//		image array containing image data to resize
	//		match_photo whose dimensions should be used for resize
	// Outputs:
	//		return array with resized image data
	//
	
	if (match_photo != nullptr)
	{
		int w = image.cols;
		int h = image.rows;
        
        if ((w != match_photo->GetWidth()) || (h != match_photo->GetHeight()))
		{
			double fx = (double)match_photo->GetWidth() / (double)w;
			double fy = (double)match_photo->GetHeight() / (double)h;

			cv::resize(image, image, cv::Size(), fx, fy, cv::INTER_LANCZOS4);
		}
	}

	return image;
}

cv::Mat Thermal::dn_to_temperature(Photo* photo, cv::Mat image, XString images_path)
{
	// Convert Digital Number values to temperature (C) values
	// 
	// Inputs:
	//		photo photo object
	//		image array containing image data
	//		images_path path to original source image to read data using PIL for DJI thermal photos
	// Outputs:
	//		return array with temperature (C) image values
	//

	// Handle thermal bands
	if (photo->is_thermal())
	{
		// Every camera stores thermal information differently
		// The following will work for MicaSense Altum cameras
		// but not necessarily for others
		
		if (photo->GetMake().CompareNoCase("MicaSense") && photo->GetModel().CompareNoCase("Altum"))
		{
			image.convertTo(image, CV_32F);
			image -= (273.15 * 100.0);		// Convert Kelvin to Celsius
			image *= 0.01;
			return image;
		}
		else if (photo->GetMake().CompareNoCase("DJI") && photo->GetModel().CompareNoCase("ZH20T"))
		{
			//filename, file_extension = os.path.splitext(photo.filename)
			//# DJI H20T high gain mode supports measurement of -40~150 celsius degrees
			//if file_extension.lower() in [".tif", ".tiff"] and image.min() >= 23315: # Calibrated grayscale tif
			//    image.convertTo(image, CV_32F);
			//    image -= (273.15 * 100.0) # Convert Kelvin to Celsius
			//    image *= 0.01
			//    return image
			//else:
			//    return image
		}
		else if (photo->GetMake().CompareNoCase("DJI") && photo->GetModel().CompareNoCase("MAVIC2-ENTERPRISE-ADVANCED"))
		{
			//image = dji_unpack.extract_temperatures_dji(photo, image, images_path)
			//image.convertTo(image, CV_32F);
			//return image
		}
		else
		{
			//try:
			//    params, image = extract_raw_thermal_image_data(os.path.join(images_path, photo.filename))
			//    image = sensor_vals_to_temp(image, **params)
			//except Exception as e:
			//    Logger::Write(__FUNCTION__, "Cannot radiometrically calibrate %s: %s" % (photo.filename, str(e)))
			//image.convertTo(image, CV_32F);
			//return image
		}
	}
	else
	{
		image.convertTo(image, CV_32F);
		Logger::Write(__FUNCTION__, "Tried to radiometrically calibrate a non-thermal image with temperature values (%s)", photo->GetFileName().c_str());
		return image;
	}

	return image;
}
