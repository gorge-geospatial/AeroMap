// Reconstruction.cpp
//

#include <map>

#include "AeroLib.h"
#include "Reconstruction.h"

Reconstruction::Reconstruction(const std::vector<Photo*> photos)
    : mp_GcpFile(nullptr)
{
    m_photos = photos;
    //    self.georef = None
    detect_multi_camera();
    //    self.filter_photos()
}

Reconstruction::~Reconstruction()
{
}

void Reconstruction::detect_multi_camera()
{
    // Looks at the reconstruction photos and determines if this
    // is a single or multi-camera setup.
    //

    m_mc.clear();
    std::map <std::string, std::vector<Photo*>> band_photos;

    std::string band_name;
    for (auto p : m_photos)
    {
        band_name = p->GetBandName().c_str();
        if (band_photos.find(band_name) == band_photos.end())
            band_photos[band_name] = std::vector<Photo*>();

        band_photos[band_name].push_back(p);
    }

    int band_count = (int)band_photos.size();
    if ((band_count >= 2) && (band_count <= 8))
    {
        // Validate that all bands have the same number of images,
        // otherwise this is not a multi-camera setup
        int img_per_band = (int)band_photos[band_name].size();
        for (auto band : band_photos)
        {
            if (band.second.size() != img_per_band)
            {
                Logger::Write(__FUNCTION__, "Multi-camera setup detected, but band \"%s\" has only %d images (instead of %d).",
                    band.first.c_str(), band.second, img_per_band);
                assert(false);
            }
        }

        for (auto band : band_photos)
        {
            BandType entry;
            entry.band_name = band.first.c_str();
            entry.band_index = band.second[0]->GetBandIndex();
            entry.photos = band.second;
            m_mc.push_back(entry);
        }

        // Sort by band index
        std::sort(m_mc.begin(), m_mc.end());
    }
}

void Reconstruction::filter_photos()
{
    if (is_multi() == false)        // Nothing to do, use all images
        return;

    // Sometimes people might try process both RGB + Blue/Red/Green bands
    // because these are the contents of the SD card from a drone (e.g. DJI P4 Multispectral)
    // But we don't want to process both, so we discard the RGB files in favor
    //bands = {}
    //for b in self.multi_camera:
    //    bands[b['name'].lower()] = b['name']

    //bands_to_remove = []

    //if 'rgb' in bands or 'redgreenblue' in bands:
    //    if 'red' in bands and 'green' in bands and 'blue' in bands:
    //        bands_to_remove.append(bands['rgb'] if 'rgb' in bands else bands['redgreenblue'])
    //    else
    //        for b in ['red', 'green', 'blue']:
    //            if b in bands:
    //                bands_to_remove.append(bands[b])

    //if len(bands_to_remove) > 0:
    //    Logger::Write(__FUNCTION__, "Redundant bands detected, probably because RGB images are mixed with single band images. We will trim some bands as needed")

    //    for band_to_remove in bands_to_remove:
    //        self.multi_camera = [b for b in self.multi_camera if b['name'] != band_to_remove]
    //        photos_before = len(self.photos)
    //        self.photos = [p for p in self.photos if p.band_name != band_to_remove]
    //        photos_after = len(self.photos)

    //        Logger::Write(__FUNCTION__, "Skipping %s band (%s images)" % (band_to_remove, photos_before - photos_after))
}

bool Reconstruction::is_georef()
{
    AeroLib::Georef georef = AeroLib::ReadGeoref();
    return georef.is_valid;
}

bool Reconstruction::has_gcp()
{
    if (is_georef())
    {
        return (mp_GcpFile && mp_GcpFile->exists());
    }
    return false;
}

bool Reconstruction::is_multi()
{
    //return (m_mc.size() > 1);
    return false;
}

int Reconstruction::GetImageCount()
{
    return (int)m_photos.size();
}

const std::vector<Photo*>& Reconstruction::GetImageList()
{
    // Return a const ref to the image list.
    //

    return m_photos;
}

//def has_geotagged_photos(self):
//    for photo in self.photos:
//        if photo.latitude is None and photo.longitude is None:
//            return False
//
//    return True

void Reconstruction::georeference_with_gcp(XString gcp_file, XString output_coords_file, XString output_gcp_file, bool rerun)
{
    if ((AeroLib::FileExists(output_coords_file) == false) || (AeroLib::FileExists(output_gcp_file) == false) || rerun)
    {
        mp_GcpFile = new GcpFile(gcp_file);
        if (mp_GcpFile->exists())
        {
            //if gcp.entries_count() == 0:
            //    raise RuntimeError("This GCP file does not have any entries. Are the entries entered in the proper format?")

            //gcp.check_entries()

            //# Convert GCP file to a UTM projection since the rest of the pipeline
            //# does not handle other SRS well.
            //rejected_entries = []
            //utm_gcp = GCPFile(gcp.create_utm_copy(output_gcp_file, filenames=[p.filename for p in self.photos], rejected_entries=rejected_entries, include_extras=True))

            //if not utm_gcp.exists():
            //    raise RuntimeError("Could not project GCP file to UTM. Please double check your GCP file for mistakes.")

            //for re in rejected_entries:
            //    Logger::Write(__FUNCTION__, "GCP line ignored (image not found): %s" % str(re))

            //if utm_gcp.entries_count() > 0:
            //    Logger::Write(__FUNCTION__, "%s GCP points will be used for georeferencing" % utm_gcp.entries_count())
            //else
            //    raise RuntimeError("A GCP file was provided, but no valid GCP entries could be used. Note that the GCP file is case sensitive (\".JPG\" is not the same as \".jpg\").")

            //self.gcp = utm_gcp

            //# Compute RTC offsets from GCP points
            //x_pos = [p.x for p in utm_gcp.iter_entries()]
            //y_pos = [p.y for p in utm_gcp.iter_entries()]
            //x_off, y_off = int(np.round(np.mean(x_pos))), int(np.round(np.mean(y_pos)))

            //# Create coords file, we'll be using this later
            //# during georeferencing
            //with open(output_coords_file, 'w') as f:
            //    coords_header = gcp.wgs84_utm_zone()
            //    f.write(coords_header + "\n")
            //    f.write("{} {}\n".format(x_off, y_off))
            //    Logger::Write(__FUNCTION__, "Generated coords file from GCP: %s" % coords_header)
        }
        else
        {
            //    Logger::Write(__FUNCTION__, "GCP file does not exist: %s" % gcp_file)
        //    return
        }
    }
    else
    {
        //    Logger::Write(__FUNCTION__, "Coordinates file already exist: %s" % output_coords_file)
        //    Logger::Write(__FUNCTION__, "GCP file already exist: %s" % output_gcp_file)
        //    self.gcp = GCPFile(output_gcp_file)
    }

    //self.georef = ODM_GeoRef.FromCoordsFile(output_coords_file)
    //return self.georef
}

void Reconstruction::georeference_with_gps(XString images_path, XString output_coords_file, bool rerun)
{
    //try:
    //    if not io.file_exists(output_coords_file) or rerun:
    //        location.extract_utm_coords(self.photos, images_path, output_coords_file)
    //    else
    //        Logger::Write(__FUNCTION__, "Coordinates file already exist: %s" % output_coords_file)

    //    self.georef = ODM_GeoRef.FromCoordsFile(output_coords_file)
    //except:
    //    log.ODM_WARNING('Could not generate coordinates file. The orthophoto will not be georeferenced.')

    //self.gcp = GCPFile(None)
    //return self.georef
}

//def save_proj_srs(self, file):
//    # Save proj to file for future use (unless this
//    # dataset is not georeferenced)
//    if self.is_georeferenced():
//        with open(file, 'w') as f:
//            f.write(self.get_proj_srs())

//def get_proj_srs(self):
//    if self.is_georeferenced():
//        return self.georef.proj4()

//def get_proj_offset(self):
//    if self.is_georeferenced():
//        return (self.georef.utm_east_offset, self.georef.utm_north_offset)
//    else
//        return (None, None)

//def get_photo(self, filename):
//    for (auto p : photos)
//        if p.filename == filename:
//            return p
