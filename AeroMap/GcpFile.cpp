// GcpFile.cpp
// Port of odm gcp.py
//

#include "Gis.h"
#include "MarkLib.h"
#include "Logger.h"
#include "TextFile.h"
#include "GcpFile.h"

GcpFile::GcpFile(XString file_name)
{
	m_file_name = file_name;

	read();
}

GcpFile::~GcpFile()
{

}

void GcpFile::read()
{
	if (exists() == false)
		return;

    TextFile textFile(m_file_name.c_str());
    if (textFile.GetLineCount() > 0)
    {
        m_raw_srs = textFile.GetLine(0).c_str();
        m_srs = parse_srs_header(m_raw_srs);

        for (int i = 1; i < textFile.GetLineCount(); ++i)
        {
            XString line = textFile.GetLine(i).c_str();
            line.TruncateAt('#');
            line.Trim(" \t\r\n");

            int token_count = line.Tokenize(" \t");
            if (token_count >= 6)
            {
                EntryType entry;
                entry.x = line.GetToken(0).GetDouble();
                entry.y = line.GetToken(1).GetDouble();
                entry.z = line.GetToken(2).GetDouble();
                entry.px = line.GetToken(3).GetInt();
                entry.py = line.GetToken(4).GetInt();
                entry.file_name = line.GetToken(5);
                m_entry_list.push_back(entry);
            }
            else
            {
                Logger::Write(__FUNCTION__, "Malformed GCP line: %s", line.c_str());
            }
        }
    }
}

void GcpFile::check_entries()
{
    std::map<std::string, int> coords;                      // map coord key -> count
    std::map<std::string, std::vector<EntryType>> gcps;     // map coord key -> list of entries with that coord
    int errors = 0;

    for (auto entry : m_entry_list)
    {
        std::string k = entry.coords_key().c_str();

        if (coords.find(k) == coords.end())
            coords[k] = 0;
        ++coords[k];

        if (gcps.find(k) == gcps.end())
            gcps[k] = std::vector<EntryType>();
        gcps[k].push_back(entry);
    }

    for (auto coord : coords)
    {
        if (coord.second < 3)
        {
            XString description = coord.second < 2 ? "insufficient" : "not ideal";
            Logger::Write(__FUNCTION__, "The number of images where the GCP %s has been tagged is %s", coord.first.c_str(), description.c_str());
            Logger::Write(__FUNCTION__, "At least 3 images should be tagged.");
            ++errors;
        }
    }
    
    if (coords.size() < 3)
    {
        Logger::Write(__FUNCTION__, "Low number of GCPs detected (%d). For best results use at least 5.", coords.size());
        ++errors;
    }

    if (errors > 0)
        Logger::Write(__FUNCTION__, "Some issues detected with GCPs (but we're going to process this anyway)");
}

int GcpFile::entry_count()
{
	return (int)m_entry_list.size();
}

bool GcpFile::exists()
{
    if (m_file_name.IsEmpty())
        return false;

    return MarkLib::FileExists(m_file_name.c_str());
}

XString GcpFile::wgs84_utm_zone()
{
    // Finds the UTM zone where the first point of the GCP falls into
    // 
    // Outputs:
    //      return = utm zone string valid for a coordinates header
    //
    
    XString wgs84_str;

    if (entry_count() > 0)
    {
        OGRSpatialReference longlat;
        longlat.importFromEPSG(4326);
        OGRCoordinateTransformation* ct = OGRCreateCoordinateTransformation(&m_srs, &longlat);
        if (ct)
        {
            double x = m_entry_list[0].x;
            double y = m_entry_list[0].y;

            if (ct->Transform(1, &x, &y))
            {
                double lat = x;
                double lon = y;

                int utm_zone = GIS::GetUTMZone(lon);
                char hemi = lat >= 0.0 ? 'N' : 'S';
                wgs84_str = XString::Format("WGS84 UTM %d%c", utm_zone, hemi);
            }
        }
    }

    return wgs84_str;
}

XString GcpFile::make_utm_copy(XString gcp_file_output, std::vector<XString>file_names, std::vector<XString>rejected_entries)
{
    // Creates a new GCP file from an existing GCP file
    // by optionally including only filenames and reprojecting each point to
    // a UTM CRS. Rejected entries can recorded by passing a list object to
    // rejected_entries.
    //

    //if os.path.exists(gcp_file_output):
    //    os.remove(gcp_file_output)

    XString output = wgs84_utm_zone() + "\n";

    OGRSpatialReference target_srs = parse_srs_header(output);
    OGRCoordinateTransformation* ct = OGRCreateCoordinateTransformation(&m_srs, &target_srs);
    if (ct)
    {
        for (auto entry : m_entry_list)
        {
            if ((file_names.size() == 0) || file_in_list(entry.file_name, file_names))
            {
                double x = entry.x;
                double y = entry.y;
                double z = entry.x;
                if (ct->Transform(1, &x, &y, &x))
                {
                    entry.x = x;
                    entry.y = y;
                    entry.z = z;
                    output += entry.to_string() + "\n";
                }
            }
            //else if isinstance(rejected_entries, list):
            //    rejected_entries.append(entry)
        }

        write_file(gcp_file_output, output);
    }

    return gcp_file_output;
}

XString GcpFile::make_resized_copy(XString gcp_file_output, double ratio)
{
    // Creates a new resized GCP file from an existing GCP file. If one already exists, it will be removed.
    // 
    // Inputs:
    //      gcp_file_output output path of new GCP file
    //      ratio scale GCP coordinates by this value
    // Outputs:
    //      return path to new GCP file
    //

    XString output = m_raw_srs + "\n";

    for (auto entry : m_entry_list)
    {
        entry.px = (int)(entry.px * ratio);
        entry.py = (int)(entry.py * ratio);

        output += entry.to_string() + "\n";
    }

    write_file(gcp_file_output, output);

    return gcp_file_output;
}

XString GcpFile::make_filtered_copy(XString gcp_file_output, XString images_dir, int min_images)
{
    // Creates a new GCP file from an existing GCP file includes
    // only the points that reference images existing in the images_dir directory.
    // If less than min_images images are referenced, no GCP copy is created.
    //
    // Outputs:
    //      return gcp_file_output if successful, None if no output file was created.
    //
    
    if ((exists() == false) || (MarkLib::PathExists(images_dir.c_str()) == false))
        return "";
    
    //if os.path.exists(gcp_file_output):
    //    os.remove(gcp_file_output)
    
    std::vector<XString> files = MarkLib::GetFileList(images_dir);
    
    XString output = m_raw_srs + "\n";
    int files_found = 0;
    
    for (auto entry : m_entry_list)
    {
        if (file_in_list(entry.file_name, files))
        {
            output += entry.to_string() + "\n";
            ++files_found;
        }
    }

    if (files_found >= min_images)
        write_file(gcp_file_output, output);

    return gcp_file_output;
}

OGRSpatialReference GcpFile::parse_srs_header(XString header)
{
    // Parse a header coming from GCP file
    // 

    OGRSpatialReference srs;

    header.Trim(" \t\r\n");
    int token_count = header.Tokenize(" \t");

    try
    {
        if (token_count > 2 && header.GetToken(0).CompareNoCase("WGS84") && header.GetToken(1).CompareNoCase("UTM"))
        {
            XString datum = header.GetToken(0);
            XString str = header.GetToken(2);
            char utm_pole = str.Right(1)[0];
            int utm_zone = str.GetInt();

            XString proj4 = XString::Format("+proj=utm +zone=%d +datum=%s +units=m +no_defs=True", utm_zone, datum.c_str());
            if (utm_pole == 'S')
                proj4 += " +south=True";

            OGRErr err = srs.importFromProj4(proj4.c_str());
            if (err != OGRERR_NONE)
                Logger::Write(__FUNCTION__, "importFromProj4(%s) failed. Code: %d", proj4.c_str(), (int)err);
        }
        else if (header.Find("+proj") > -1)
        {
            header.Replace("\'", "");
            OGRErr err = srs.importFromProj4(header.c_str());
            if (err != OGRERR_NONE)
                Logger::Write(__FUNCTION__, "importFromProj4(%s) failed. Code: %d", header.c_str(), (int)err);
        }
        else if (header.BeginsWithNoCase("EPSG:"))
        {
            //    srs = CRS.from_epsg(header.lower()[5:])
            int epsg_code = header.Mid(5).GetInt();
            OGRErr err = srs.importFromEPSG(epsg_code);
            if (err != OGRERR_NONE)
                Logger::Write(__FUNCTION__, "importFromEPSG(%d) failed. Code: %d", epsg_code, (int)err);
        }
        else
        {
            Logger::Write(__FUNCTION__, "Could not parse coordinates. Bad SRS supplied: '%s'", header.c_str());
        }
    }
    catch (std::exception ex)
    {
        Logger::Write(__FUNCTION__, "Invalid GCP header: '%s'", header.c_str());
    }
    
    return srs;
}

void GcpFile::write_file(XString file_name, XString text)
{
    FILE* pFile = fopen(file_name.c_str(), "wt");
    if (pFile)
    {
        fprintf(pFile, "%s", text.c_str());
        fclose(pFile);
    }
}

bool GcpFile::file_in_list(XString file_name, std::vector<XString> list)
{
    bool found = false;

    // compare only root file names
    if (file_name.GetFileName().IsEmpty() == false)
        file_name = file_name.GetFileName();

    for (auto f : list)
    {
        XString file = f;
        if (file.GetFileName().IsEmpty() == false)
            file = f.GetFileName();

        if (file_name.CompareNoCase(file))
        {
            found = true;
            break;
        }
    }

    return found;
}
