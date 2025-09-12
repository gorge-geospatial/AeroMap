#ifndef PHOTO_H
#define PHOTO_H

#include <vector>

#include <opencv2/opencv.hpp>	// OpenCV
#include "tiffio.h"
#include "tiffiop.h"

#include "MarkTypes.h"
#include "Logger.h"
#include "XString.h"

class Photo
{
public:

    cv::Mat m_image;        // image data as opencv matrix

public:
	
	Photo();
	Photo(XString file_name);
	~Photo();

    int GetWidth();
    int GetHeight();
    int GetExifWidth();
    int GetExifHeight();

    XString GetBandName();
    int GetBandIndex();
    XString GetCaptureUUID();

    bool is_rgb();
    bool is_thermal();

    XString GetMake();
    XString GetModel();

    int GetOrientation();
    double GetFNumber();
    double GetExposureTime();
    int GetISOSpeed();

    double GetLatitude();
    double GetLongitude();
    double GetAltitude();

    double GetFocalRatio();
    __int64 GetEpoch();
    XString GetCaptureDate();

    XString GetCameraString(bool opensfm);
    XString GetFileName();

    static SizeType find_largest_photo_dims(const std::vector<Photo*>& photos);
	static int      find_largest_photo_dim(const std::vector<Photo*>& photos);
	static Photo*   find_largest_photo(const std::vector<Photo*>& photos);
    static bool     IsSupportedImageFormat(XString file_name);

	static double get_mm_per_unit(int resolution_unit);

private:

	// Standard tags (virtually all photos have these)
	int m_width;
	int m_height;
	XString m_camera_make;
	XString m_camera_model;
	int m_orientation;

	// Geo tags
	double m_latitude;
	double m_longitude;
	double m_altitude;

	// Multi-band fields
	XString m_band_name;
	int m_band_index;
	XString m_capture_uuid;

	// Multi-spectral fields
	double m_fnumber;
	//m_radiometric_calibration = None
	//m_black_level = None
	//m_gain = None
	//m_gain_adjustment = None

    // Capture info
    double m_exposure_time;
    int m_iso_speed;
    //m_bits_per_sample = None
    //m_vignetting_center = None
    //m_vignetting_polynomial = None
    //m_spectral_irradiance = None
    //m_horizontal_irradiance = None
    //m_irradiance_scale_to_si = None
    //m_utc_time = None
    __int64 m_epoch;
    XString m_capture_date;

    // OPK angles
    double m_yaw;
    double m_pitch;
    double m_roll;
    double m_omega;
    double m_phi;
    double m_kappa;

    // DLS
    //m_sun_sensor = None
    //m_dls_yaw = None
    //m_dls_pitch = None
    //m_dls_roll = None

    // Aircraft speed
    double m_speed_x;
    double m_speed_y;
    double m_speed_z;

    // Original image width/height at capture time (before possible resizes)
    int m_exif_width;
    int m_exif_height;

    // m_center_wavelength = None
    // m_bandwidth = None

    // RTK
    double m_gps_xy_stddev;     // Dilution of Precision X/Y
    double m_gps_z_stddev;      // Dilution of Precision Z

    // Misc SFM
    XString m_camera_projection;
    double m_focal_ratio;
    double m_focal_length;
    int m_FocalPlaneResolutionUnit;
    double m_FocalPlaneXResolution;
    double m_FocalPlaneYResolution;

    XString m_file_name;

private:

    void parse_exif_values(XString file_name);
    void parse_tiff_values(XString file_name);

    double Photo::CalcFocalRatio();
    int yisleap(int year);
    int get_yday(int mon, int day, int year);
    __int64 CalcUnixEpoch(const char* dateTime);

    XString GetXmpMeta(XString file_name);
};

#endif // #ifndef PHOTO_H
