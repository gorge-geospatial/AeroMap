// Photo.cpp
// Port of odm photo.py
//

#include <iostream> // std::cout
#include <fstream>  // std::ifstream
//#include <vector>   // std::vector

#include "MarkLib.h"
#include "exif.h"       // easy exif header
#include "TinyEXIF.h"   // tiny exif header
#include "Photo.h"

//projections = ['perspective', 'fisheye', 'fisheye_opencv', 'brown', 'dual', 'equirectangular', 'spherical']

Photo::Photo()
    : Photo("")
{
}

Photo::Photo(XString file_name)
{
    m_file_name = file_name;
    //m_mask = None

    // Standard tags (virtually all photos have these)
    m_width = 0;
    m_height = 0;
    m_camera_make.Clear();
    m_camera_model.Clear();
    m_orientation = 1;

    // Geo tags
    m_latitude = 0.0;
    m_longitude = 0.0;
    m_altitude = 0.0;

    // Multi-band fields
    m_band_name = "RGB";
    m_band_index = 0;
    m_capture_uuid = "";

    // Multi-spectral fields
    m_fnumber = 0.0;
    //m_radiometric_calibration = None
    //m_black_level = None
    //m_gain = None
    //m_gain_adjustment = None

    // Capture info
    //m_exposure_time = None
    m_iso_speed = 0;
    //m_bits_per_sample = None
    //m_vignetting_center = None
    //m_vignetting_polynomial = None
    //m_spectral_irradiance = None
    //m_horizontal_irradiance = None
    //m_irradiance_scale_to_si = None
    //m_utc_time = None

    // OPK angles
    m_yaw = 0.0;
    m_pitch = 0.0;
    m_roll = 0.0;
    m_omega = 0.0;
    m_phi = 0.0;
    m_kappa = 0.0;

    // DLS
    //m_sun_sensor = None
    //m_dls_yaw = None
    //m_dls_pitch = None
    //m_dls_roll = None

    // Aircraft speed
    m_speed_x = 0.0;
    m_speed_y = 0.0;
    m_speed_z = 0.0;

    // Original image width/height at capture time (before possible resizes)
    m_exif_width = 0;
    m_exif_height = 0;

    // m_center_wavelength = None
    // m_bandwidth = None

    // RTK
    m_gps_xy_stddev = 0.0;  // Dilution of Precision X/Y
    m_gps_z_stddev = 0.0;   // Dilution of Precision Z

    // Misc SFM
    m_camera_projection = "brown";
    m_focal_ratio = 0.85;

    if (MarkLib::FileExists(file_name.c_str()))
    {
        // read image data using opencv
        m_image = cv::imread(file_name.c_str(), cv::IMREAD_UNCHANGED);
        m_width = m_image.cols;
        m_height = m_image.rows;
        m_exif_width = m_width;
        m_exif_height = m_height;

        // parse values from metadata
        if (file_name.EndsWithNoCase(".jpg") || file_name.EndsWithNoCase(".jpeg"))
            parse_exif_values(file_name);
        else if (file_name.EndsWithNoCase(".tif") || file_name.EndsWithNoCase(".tiff"))
            parse_tiff_values(file_name);

        m_focal_ratio = CalcFocalRatio();
        m_epoch = CalcUnixEpoch(m_capture_date.c_str());
    }
}

Photo::~Photo()
{

}

void Photo::parse_exif_values(XString file_name)
{
    // For historical reasons, this routine does both:
    //      1. parse using easy exif
    //      2. parse using tiny exif
    // Since tiny exif says it's based on easy exif, could
    // probably skip easyexif & get all values from tinyexif.
    //

    // Read the JPEG file into a buffer
    FILE* fp = fopen(file_name, "rb");
    if (!fp)
    {
        Logger::Write(__FUNCTION__, "Unable to open: '%s'", file_name.c_str());
        return;
    }
    fseek(fp, 0, SEEK_END);
    unsigned long fsize = ftell(fp);
    rewind(fp);
    unsigned char* buf = new unsigned char[fsize];
    if (fread(buf, 1, fsize, fp) != fsize)
    {
        Logger::Write(__FUNCTION__, "Unable to read: '%s'", file_name.c_str());
        delete[] buf;
        return;
    }
    fclose(fp);

    // Parse EXIF
    easyexif::EXIFInfo easy_exif;
    int code = easy_exif.parseFrom(buf, fsize);
    delete[] buf;
    if (code)
    {
        Logger::Write(__FUNCTION__, "Error parsing EXIF: '%s' (code %d)", file_name.c_str(), code);
        return;
    }

    m_camera_make = easy_exif.Make.c_str();
    m_camera_model = easy_exif.Model.c_str();
    m_capture_date = easy_exif.DateTime.c_str();
    m_orientation = easy_exif.Orientation;
    m_fnumber = easy_exif.FNumber;
    m_focal_length = easy_exif.FocalLength;
    m_exposure_time = easy_exif.ExposureTime;
    m_iso_speed = easy_exif.ISOSpeedRatings;
    m_latitude = easy_exif.GeoLocation.Latitude;
    m_longitude = easy_exif.GeoLocation.Longitude;
    m_altitude = easy_exif.GeoLocation.Altitude;
    m_FocalPlaneResolutionUnit = easy_exif.LensInfo.FocalPlaneResolutionUnit;
    m_FocalPlaneXResolution = easy_exif.LensInfo.FocalPlaneXResolution;
    m_FocalPlaneYResolution = easy_exif.LensInfo.FocalPlaneYResolution;

    // parse with tiny exif

    std::ifstream istream(file_name.c_str(), std::ifstream::binary);

    // parse image EXIF and XMP metadata
    TinyEXIF::EXIFInfo tiny_exif(istream);
    if (tiny_exif.Fields)
    {
        if (tiny_exif.BandName.empty() == false)
        {
            m_band_name = tiny_exif.BandName.c_str();
            m_band_index = tiny_exif.BandIndex;
        }
        if (tiny_exif.CaptureUUID.empty() == false)
            m_capture_uuid = tiny_exif.CaptureUUID.c_str();
    }
}

void Photo::parse_tiff_values(XString file_name)
{
    XString xmp_meta = GetXmpMeta(file_name);
    if (xmp_meta.IsEmpty() == false)
    {
        TinyEXIF::EXIFInfo tiny_exif;
        int code = tiny_exif.parseFromXMPSegmentXML(xmp_meta.c_str(), xmp_meta.GetLength());
        if (code != TinyEXIF::ErrorCode::PARSE_SUCCESS)
        {
            Logger::Write(__FUNCTION__, "Unable to parse XMP metadata for: '%s'", file_name.c_str());
        }
        else
        {
            if (tiny_exif.BandName.empty() == false)
            {
                m_band_name = tiny_exif.BandName.c_str();
                m_band_index = tiny_exif.BandIndex;
            }
            if (tiny_exif.CaptureUUID.empty() == false)
                m_capture_uuid = tiny_exif.CaptureUUID.c_str();
        }
    }

    TIFF* pTiff = TIFFOpen(file_name.c_str(), "r");

    char* p = nullptr;
    if (TIFFGetField(pTiff, TIFFTAG_MAKE, &p) == 1)
        m_camera_make = p;
    if (TIFFGetField(pTiff, TIFFTAG_MODEL, &p) == 1)
        m_camera_model = p;
    if (TIFFGetField(pTiff, TIFFTAG_DATETIME, &p) == 1)
        m_capture_date = p;

    // move to gps directory
    toff_t offset = 0;
    if (TIFFGetField(pTiff, TIFFTAG_GPSIFD, &offset))
    {
        if (TIFFReadGPSDirectory(pTiff, offset))
        {
            void* raw_data = NULL;
            const TIFFField* fip = TIFFFieldWithTag(pTiff, GPSTAG_LATITUDE);
            if (fip)
            {
                int tv_size = TIFFFieldSetGetSize(fip);
                if (TIFFGetField(pTiff, GPSTAG_LATITUDE, &raw_data) == 1)
                {
                    if (tv_size == 8)
                        m_latitude = ((double*)raw_data)[0] + ((double*)raw_data)[1] / 60.0 + ((double*)raw_data)[2] / 3600.0;
                    else
                        m_latitude = ((float*)raw_data)[0] + ((float*)raw_data)[1] / 60.0 + ((float*)raw_data)[2] / 3600.0;
                }
            }
            fip = TIFFFieldWithTag(pTiff, GPSTAG_LONGITUDE);
            if (fip)
            {
                int tv_size = TIFFFieldSetGetSize(fip);
                if (TIFFGetField(pTiff, GPSTAG_LONGITUDE, &raw_data) == 1)
                {
                    if (tv_size == 8)
                        m_longitude = ((double*)raw_data)[0] + ((double*)raw_data)[1] / 60.0 + ((double*)raw_data)[2] / 3600.0;
                    else
                        m_longitude = ((float*)raw_data)[0] + ((float*)raw_data)[1] / 60.0 + ((float*)raw_data)[2] / 3600.0;
                }
            }
            double d = 0.0;
            if (TIFFGetField(pTiff, GPSTAG_ALTITUDE, &d) == 1)
                m_altitude = d;
        }
    }

    TIFFClose(pTiff);
}

XString Photo::GetXmpMeta(XString file_name)
{
    // Extract xmp metadata from tif file.
    // 

    XString xmp_meta;

    FILE* fp = fopen(file_name, "rb");
    if (!fp)
    {
        Logger::Write(__FUNCTION__, "Unable to open: '%s'", file_name.c_str());
        return "";
    }
    fseek(fp, 0, SEEK_END);
    unsigned long fsize = ftell(fp);
    rewind(fp);
    unsigned char* buf = new unsigned char[fsize];
    if (fread(buf, 1, fsize, fp) != fsize)
    {
        Logger::Write(__FUNCTION__, "Unable to read: '%s'", file_name.c_str());
        delete[] buf;
        return "";
    }
    fclose(fp);

    char* start_key = "<x:xmpmeta ";
    char* stop_key = "</x:xmpmeta>";
    int start_len = (int)strlen(start_key);
    int stop_len = (int)strlen(stop_key);

    for (int i = fsize - start_len; i > start_len; --i)
    {
        if (memcmp(&buf[i], start_key, start_len) == 0)
        {
            // copy into null terminated buffer for conversion
            // to string

            // +2 = 1 for trailing zero & 1 for fencepost
            int meta_len = fsize - i + 2;
            unsigned char* meta_buf = new unsigned char[meta_len];
            memset(meta_buf, 0, meta_len);
            memcpy(meta_buf, &buf[i], meta_len - 2);
            xmp_meta = (char*)meta_buf;
            delete[] meta_buf;

            int pos = xmp_meta.Find(stop_key);
            if (pos > -1)
                xmp_meta = xmp_meta.Left(pos + stop_len);
            //printf("xmp_meta=%s", xmp_meta.c_str());
        }
    }

    delete[] buf;

    return xmp_meta;
}

//    def set_mask(self, mask):
//        self.mask = mask

//    def update_with_geo_entry(self, geo_entry):
//        self.latitude = geo_entry.y
//        self.longitude = geo_entry.x
//        self.altitude = geo_entry.z
//        if geo_entry.yaw is not None and geo_entry.pitch is not None and geo_entry.roll is not None:
//            self.yaw = geo_entry.yaw
//            self.pitch = geo_entry.pitch
//            self.roll = geo_entry.roll
//            self.dls_yaw = geo_entry.yaw
//            self.dls_pitch = geo_entry.pitch
//            self.dls_roll = geo_entry.roll
//        self.gps_xy_stddev = geo_entry.horizontal_accuracy
//        self.gps_z_stddev = geo_entry.vertical_accuracy

//    def get_radiometric_calibration(self):
//        if isinstance(self.radiometric_calibration, str):
//            parts = self.radiometric_calibration.split(" ")
//            if len(parts) == 3:
//                return list(map(float, parts))
//
//        return [None, None, None]

//    def get_dark_level(self):
//        if self.black_level:
//            levels = np.array([float(v) for v in self.black_level.split(" ")])
//            return levels.mean()

//    def get_gain(self):
//        if self.gain is not None:
//            return self.gain
//        elif self.iso_speed:
//            #(gain = ISO/100)
//            return self.iso_speed / 100.0

//    def get_vignetting_center(self):
//        if self.vignetting_center:
//            parts = self.vignetting_center.split(" ")
//            if len(parts) == 2:
//                return list(map(float, parts))
//        return [None, None]

//    def get_vignetting_polynomial(self):
//        if self.vignetting_polynomial:
//            parts = self.vignetting_polynomial.split(" ")
//            if len(parts) > 0:
//                coeffs = list(map(float, parts))
//
//                # Different camera vendors seem to use different ordering for the coefficients
//                if self.camera_make != "Sentera":
//                    coeffs.reverse()
//
//                return coeffs

//    def get_utc_time(self):
//        if self.utc_time:
//            return datetime.fromtimestamp(self.utc_time / 1000, timezone.utc)

//    def get_photometric_exposure(self):
//        # H ~= (exposure_time) / (f_number^2)
//        if self.fnumber is not None and self.exposure_time is not None and self.exposure_time > 0 and self.fnumber > 0:
//            return self.exposure_time / (self.fnumber * self.fnumber)

//    def get_horizontal_irradiance(self):
//        if self.horizontal_irradiance is not None:
//            scale = 1.0 # Assumed
//            if self.irradiance_scale_to_si is not None:
//                scale = self.irradiance_scale_to_si
//
//            return self.horizontal_irradiance * scale
//        elif self.camera_make == "DJI" and self.spectral_irradiance is not None:
//            # Phantom 4 Multispectral saves this value in @drone-dji:Irradiance
//            return self.spectral_irradiance

//    def get_sun_sensor(self):
//        if self.sun_sensor is not None:
//            # TODO: Presence of XMP:SunSensorExposureTime
//            # and XMP:SunSensorSensitivity might
//            # require additional logic. If these two tags are present,
//            # then sun_sensor is not in physical units?
//            return self.sun_sensor / 65535.0 # normalize uint16 (is this correct?)
//        elif self.spectral_irradiance is not None:
//            scale = 1.0 # Assumed
//            if self.irradiance_scale_to_si is not None:
//                scale = self.irradiance_scale_to_si
//
//            return self.spectral_irradiance * scale

//    def get_dls_pose(self):
//        if self.dls_yaw is not None:
//            return [self.dls_yaw, self.dls_pitch, self.dls_roll]
//        return [0.0, 0.0, 0.0]

//    def get_bit_depth_max(self):
//        if self.bits_per_sample:
//            return float(2 ** self.bits_per_sample)
//        else:
//            # If it's a JPEG, this must be 256
//            _, ext = os.path.splitext(self.filename)
//            if ext.lower() in [".jpeg", ".jpg"]:
//                return 256.0
//
//        return None

//    def get_capture_id(self):
//        # Use capture UUID first, capture time as fallback
//        if self.capture_uuid is not None:
//            return self.capture_uuid
//
//        return self.get_utc_time()

//    def get_gps_dop(self):
//        val = -9999
//        if self.gps_xy_stddev is not None:
//            val = self.gps_xy_stddev
//        if self.gps_z_stddev is not None:
//            val = max(val, self.gps_z_stddev)
//        if val > 0:
//            return val
//
//        return None

//    def override_gps_dop(self, dop):
//        self.gps_xy_stddev = self.gps_z_stddev = dop

//    def override_camera_projection(self, camera_projection):
//        if camera_projection in projections:
//            self.camera_projection = camera_projection

//    def to_opensfm_exif(self, rolling_shutter = False, rolling_shutter_readout = 0):
//        capture_time = 0.0
//        if self.utc_time is not None:
//            capture_time = self.utc_time / 1000.0
//
//        gps = {}
//        has_gps = self.latitude is not None and self.longitude is not None
//        if has_gps:
//            gps['latitude'] = self.latitude
//            gps['longitude'] = self.longitude
//            if self.altitude is not None:
//                gps['altitude'] = self.altitude
//            else:
//                gps['altitude'] = 0.0
//
//            dop = self.get_gps_dop()
//            if dop is None:
//                dop = 10.0 # Default
//
//            gps['dop'] = dop
//
//        d = {
//            "make": self.camera_make,
//            "model": self.camera_model,
//            "width": self.width,
//            "height": self.height,
//            "projection_type": self.camera_projection,
//            "focal_ratio": self.focal_ratio,
//            "orientation": self.orientation,
//            "capture_time": capture_time,
//            "gps": gps,
//            "camera": self.camera_id()
//        }
//
//        if self.has_opk():
//            d['opk'] = {
//                'omega': self.omega,
//                'phi': self.phi,
//                'kappa': self.kappa
//            }
//
//        # Speed is not useful without GPS
//        if self.has_speed() and has_gps:
//            d['speed'] = [self.speed_y, self.speed_x, self.speed_z]
//
//        if rolling_shutter:
//            d['rolling_shutter'] = get_rolling_shutter_readout(self, rolling_shutter_readout)
//
//        return d

//    def has_ypr(self):
//        return self.yaw is not None and \
//            self.pitch is not None and \
//            self.roll is not None

//    def has_opk(self):
//        return self.omega is not None and \
//            self.phi is not None and \
//            self.kappa is not None

//    def has_speed(self):
//        return self.speed_x is not None and \
//                self.speed_y is not None and \
//                self.speed_z is not None

//    def has_geo(self):
//        return self.latitude is not None and \
//            self.longitude is not None

//    def compute_opk(self):
//        if self.has_ypr() and self.has_geo():
//            y, p, r = math.radians(self.yaw), math.radians(self.pitch), math.radians(self.roll)
//
//            # Ref: New Calibration and Computing Method for Direct
//            # Georeferencing of Image and Scanner Data Using the
//            # Position and Angular Data of an Hybrid Inertial Navigation System
//            # by Manfred Bäumker
//
//            # YPR rotation matrix
//            cnb = np.array([[ math.cos(y) * math.cos(p), math.cos(y) * math.sin(p) * math.sin(r) - math.sin(y) * math.cos(r), math.cos(y) * math.sin(p) * math.cos(r) + math.sin(y) * math.sin(r)],
//                            [ math.sin(y) * math.cos(p), math.sin(y) * math.sin(p) * math.sin(r) + math.cos(y) * math.cos(r), math.sin(y) * math.sin(p) * math.cos(r) - math.cos(y) * math.sin(r)],
//                            [ -math.sin(p), math.cos(p) * math.sin(r), math.cos(p) * math.cos(r)],
//                           ])
//
//            # Convert between image and body coordinates
//            # Top of image pixels point to flying direction
//            # and camera is looking down.
//            # We might need to change this if we want different
//            # camera mount orientations (e.g. backward or sideways)
//
//            # (Swap X/Y, flip Z)
//            cbb = np.array([[0, 1, 0],
//                            [1, 0, 0],
//                            [0, 0, -1]])
//
//            delta = 1e-7
//
//            alt = self.altitude if self.altitude is not None else 0.0
//            p1 = np.array(ecef_from_lla(self.latitude + delta, self.longitude, alt))
//            p2 = np.array(ecef_from_lla(self.latitude - delta, self.longitude, alt))
//            xnp = p1 - p2
//            m = np.linalg.norm(xnp)
//
//            if m == 0:
//                Logger::Write(__FUNCTION__, "Cannot compute OPK angles, divider = 0")
//                return
//
//            # Unit vector pointing north
//            xnp /= m
//
//            znp = np.array([0, 0, -1]).T
//            ynp = np.cross(znp, xnp)
//
//            cen = np.array([xnp, ynp, znp]).T
//
//            # OPK rotation matrix
//            ceb = cen.dot(cnb).dot(cbb)
//
//            self.omega = math.degrees(math.atan2(-ceb[1][2], ceb[2][2]))
//            self.phi = math.degrees(math.asin(ceb[0][2]))
//            self.kappa = math.degrees(math.atan2(-ceb[0][1], ceb[0][0]))

//    def get_capture_megapixels(self):
//        if self.exif_width is not None and self.exif_height is not None:
//            # Accurate so long as resizing / postprocess software
//            # did not fiddle with the tags
//            return self.exif_width * self.exif_height / 1e6
//        elif self.width is not None and self.height is not None:
//            # Fallback, might not be accurate since the image
//            # could have been resized
//            return self.width * self.height / 1e6
//        else:
//            return 0.0

int Photo::yisleap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int Photo::get_yday(int mon, int day, int year)
{
    static const int days[2][13] = {
        {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };
    int leap = yisleap(year);

    return days[leap][mon] + day;
}

__int64 Photo::CalcUnixEpoch(const char* dateTime)
{
    // Return seconds since Jan 1, 1950.
    //
    // Inputs:
    //		dateTime = "YYYY:MM:SS hh:mm:ss"
    // Outputs:
    //		return = seconds since Jan 1, 1970
    //

    int epoch = 0;

    XString str = dateTime;
    if (str.GetLength() == 19)
    {
        int tm_year = str.Mid(0, 4).GetLong();
        int tm_month = str.Mid(5, 2).GetLong();
        int tm_day = str.Mid(8, 2).GetLong();
        int tm_hour = str.Mid(11, 2).GetLong();
        int tm_min = str.Mid(14, 2).GetLong();
        int tm_sec = str.Mid(17, 2).GetLong();

        int tm_yday = get_yday(tm_month, tm_day, tm_year);

        // yday is 1-based
        --tm_yday;

        tm_year -= 1900;

        epoch = tm_sec + tm_min * 60 + tm_hour * 3600 + tm_yday * 86400 +
            (tm_year - 70) * 31536000 + ((tm_year - 69) / 4) * 86400 -
            ((tm_year - 1) / 100) * 86400 + ((tm_year + 299) / 400) * 86400;
    }

    return epoch;
}

double Photo::CalcFocalRatio()
{
    double focal_ratio = 0.0;

    double sensor_width = 0.0;
    if (m_FocalPlaneResolutionUnit > 0 && m_FocalPlaneXResolution > 0.0)
    {
        int resolution_unit = m_FocalPlaneResolutionUnit;
        double mm_per_unit = 0.0;
        if (resolution_unit == 2)		// inch
            mm_per_unit = 25.4;
        else if (resolution_unit == 3)		// cm
            mm_per_unit = 10;
        else if (resolution_unit == 4)		// mm
            mm_per_unit = 1;
        else if (resolution_unit == 5)		// um
            mm_per_unit = 0.001;
        else
        {
            Logger::Write(__FUNCTION__, "Unknown EXIF resolution unit: %d", resolution_unit);
            assert(false);
            return focal_ratio;
        }

        if (mm_per_unit > 0.0)
        {
            double pixels_per_unit = m_FocalPlaneXResolution;
            if (pixels_per_unit <= 0.0 && m_FocalPlaneYResolution > 0.0)
                pixels_per_unit = m_FocalPlaneYResolution;

            if ((pixels_per_unit > 0.0) && (m_width > 0))
            {
                double units_per_pixel = 1.0 / pixels_per_unit;
                sensor_width = (double)m_width * units_per_pixel * mm_per_unit;
            }
        }
    }

    //focal_35 = None
    double focal = 0.0;
    //if "EXIF FocalLengthIn35mmFilm" in tags:
    //	focal_35 = self.float_value(tags["EXIF FocalLengthIn35mmFilm"])
    if (m_focal_length > 0.0)
        focal = m_focal_length;
    //if focal is None and "@aux:Lens" in xtags:
    //	lens = self.get_xmp_tag(xtags, ["@aux:Lens"])
    //	matches = re.search('([\d\.]+)mm', str(lens))
    //	if matches:
    //		focal = float(matches.group(1))

    //if focal_35 is not None and focal_35 > 0:
    //	focal_ratio = focal_35 / 36.0  # 35mm film produces 36x24mm pictures.
    //else:
    //	if not sensor_width:
    //		sensor_width = sensor_data().get(sensor_string, None)
    if ((sensor_width > 0.0) && (focal > 0.0))
        focal_ratio = focal / sensor_width;
    else
        focal_ratio = 0.85;

    return focal_ratio;
}

int Photo::GetWidth()
{
    return m_width;
}

int Photo::GetHeight()
{
    return m_height;
}

int Photo::GetExifWidth()
{
    return m_exif_width;
}

int Photo::GetExifHeight()
{
    return m_exif_height;
}

XString Photo::GetBandName()
{
    return m_band_name;
}

int Photo::GetBandIndex()
{
    return m_band_index;
}

XString Photo::GetCaptureUUID()
{
    return m_capture_uuid;
}

bool Photo::is_rgb()
{
    if (m_band_name.CompareNoCase("RGB") || m_band_name.CompareNoCase("REDGREENBLUE"))
        return true;
    
    return false;
}

bool Photo::is_thermal()
{
    // Added for support M2EA camera sensor
    if ((m_camera_make == "DJI") && (m_camera_model == "MAVIC2-ENTERPRISE-ADVANCED") && (m_width == 640) && (m_height == 512))
        return true;
    // Added for support DJI H20T camera sensor
    if ((m_camera_make == "DJI") && (m_camera_model == "ZH20T") && (m_width == 640) && (m_height == 512))
        return true;
    if (m_band_name.CompareNoCase("LWIR"))      // TODO: more?
        return true;

    return false;
}

XString Photo::GetMake()
{
    return m_camera_make;
}

XString Photo::GetModel()
{
    return m_camera_model;
}

int Photo::GetOrientation()
{
    return m_orientation;
}

double Photo::GetFNumber()
{
    return m_fnumber;
}

double Photo::GetExposureTime()
{
    return m_exposure_time;
}

int Photo::GetISOSpeed()
{
    return m_iso_speed;
}

double Photo::GetLatitude()
{
    return m_latitude;
}

double Photo::GetLongitude()
{
    return m_longitude;
}

double Photo::GetAltitude()
{
    return m_altitude;
}

double Photo::GetFocalRatio()
{
    return m_focal_ratio;
}

__int64 Photo::GetEpoch()
{
    return m_epoch;
}

XString Photo::GetCaptureDate()
{
    return m_capture_date;
}

XString Photo::GetCameraString(bool opensfm)
{
    // Return camera id string.
    //

    double focal_ratio = CalcFocalRatio();

    XString camera_str = XString::Format("%s %s %d %d brown %0.4f",
        m_camera_make.c_str(), m_camera_model.c_str(),
        m_width, m_height,
        focal_ratio);

    // opensfm and odm have slightly different formats
    if (opensfm)
        camera_str.Insert(0, "v2 ");

    camera_str.MakeLower();

    return camera_str;
}

XString Photo::GetFileName()
{
	return m_file_name;
}

/* static */ SizeType Photo::find_largest_photo_dims(const std::vector<Photo*>&photos)
{
	int max_mp = 0;
	SizeType max_dims = { 0, 0 };

	for (auto p : photos)
	{
		if (p->GetWidth() == 0 || p->GetHeight() == 0)
			continue;
		
        int mp = p->GetWidth() * p->GetHeight();
		if (mp > max_mp)
		{
			max_mp = mp;
			max_dims = { p->GetWidth(), p->GetHeight() };
		}
	}

	return max_dims;
}

/* static */ int Photo::find_largest_photo_dim(const std::vector<Photo*>&photos)
{
	int max_dim = 0;

	for (auto p : photos)
	{
		if (p->GetWidth() == 0 || p->GetHeight() == 0)
			continue;

        max_dim = std::max(max_dim, std::max(p->GetWidth(), p->GetHeight()));
	}

	return max_dim;
}

/* static */ Photo* Photo::find_largest_photo(const std::vector<Photo*>&photos)
{
	Photo* max_p = nullptr;

	int max_area = 0;
	for (auto p : photos)
	{
		if (p->GetWidth() == 0 || p->GetHeight() == 0)
			continue;
		
        int area = p->GetWidth() * p->GetHeight();
		if (area > max_area)
		{
			max_area = area;
			max_p = p;
		}
	}

	return max_p;
}

/* static */ double Photo::get_mm_per_unit(int resolution_unit)
{
    // Length of a resolution unit in millimeters.
    //
    // Uses the values from the EXIF specs in
    // https://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/EXIF.html
    //
    // Inputs:
    //		resolution_unit	= resolution unit value given in the EXIF
    //

    double mm = 0.0;

    switch (resolution_unit) {
    case 2:			// inch
        mm = 25.4;
        break;
    case 3:			// cm
        mm = 10.0;
        break;
    case 4:			// mm
        mm = 1.0;
        break;
    case 5:			// um
        mm = 0.001;
        break;
    default:
        Logger::Write(__FUNCTION__, "Unknown EXIF resolution unit value: %d", resolution_unit);
    }

    return mm;
}

bool Photo::IsSupportedImageFormat(XString file_name)
{
    if (file_name.EndsWithNoCase(".jpg") || file_name.EndsWithNoCase(".jpeg"))
        return true;
    else if (file_name.EndsWithNoCase(".tif") || file_name.EndsWithNoCase(".tiff"))
        return true;

    return false;
}
