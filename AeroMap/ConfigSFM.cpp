// ConfigSFM.cpp
// Manager for OpenSFM configuration.
//

#include "TextFile.h"
#include "ConfigSFM.h"

ConfigSFM::ConfigSFM()
{
	LoadFile();
}

ConfigSFM::~ConfigSFM()
{

}

XString ConfigSFM::GetFileName()
{
	return XString::CombinePath(tree.opensfm, "config.yaml");
}

void ConfigSFM::WriteDefaultFile()
{
	FILE* pFile = fopen(GetFileName().c_str(), "wt");
	if (pFile)
	{
		fprintf(pFile, "align_method: auto\n");
		fprintf(pFile, "align_orientation_prior: vertical\n");
		fprintf(pFile, "bundle_outlier_filtering_type: AUTO\n");
		fprintf(pFile, "feature_min_frames: 10000\n");
		fprintf(pFile, "feature_process_size: 2448\n");
		fprintf(pFile, "feature_type: SIFT\n");
		fprintf(pFile, "flann_algorithm: KDTREE\n");
		fprintf(pFile, "local_bundle_radius: 0\n");
		fprintf(pFile, "matcher_type: FLANN\n");
		fprintf(pFile, "matching_gps_distance: 0\n");
		fprintf(pFile, "matching_gps_neighbors: 0\n");
		fprintf(pFile, "matching_graph_rounds: 50\n");
		fprintf(pFile, "optimize_camera_parameters: true\n");
		fprintf(pFile, "processes: 16\n");
		fprintf(pFile, "reconstruction_algorithm: incremental\n");
		fprintf(pFile, "retriangulation_ratio: 2\n");
		fprintf(pFile, "sift_peak_threshold: 0.066\n");
		fprintf(pFile, "triangulation_type: ROBUST\n");
		fprintf(pFile, "undistorted_image_format: tif\n");
		fprintf(pFile, "undistorted_image_max_size: 4896\n");
		fprintf(pFile, "use_altitude_tag: true\n");
		fprintf(pFile, "use_exif_size: false\n");

		fclose(pFile);
	}
}

void ConfigSFM::SetValue(XString key, int val)
{
	bool found = false;

	for (int i = 0; i < m_entry_list.size(); ++i)
	{
		if (m_entry_list[i].key.CompareNoCase(key))
		{
			m_entry_list[i].val = XString::Format("%d", val);
			found = true;
			break;
		}
	}

	if (found == false)
	{
		EntryType entry;
		entry.key = key;
		entry.val = XString::Format("%d", val);
		m_entry_list.push_back(entry);
	}

	SaveFile();
}

int ConfigSFM::GetIntValue(XString key)
{
	int val = 0;

	for (auto entry : m_entry_list)
	{
		if (entry.key.CompareNoCase(key))
		{
			val = entry.val.GetInt();
			break;
		}
	}

	return val;
}

void ConfigSFM::LoadFile()
{
	XString file_name = GetFileName();
	TextFile textFile(file_name.c_str());

	for (int i = 0; i < textFile.GetLineCount(); ++i)
	{
		XString line = textFile.GetLine(i).c_str();
		int token_count = line.Tokenize(":");
		if (token_count == 2)
		{
			EntryType entry;
			entry.key = line.GetToken(0);
			entry.val = line.GetToken(1);
			m_entry_list.push_back(entry);
		}
	}
}

void ConfigSFM::SaveFile()
{
	FILE* pFile = fopen(GetFileName().c_str(), "wt");
	if (pFile)
	{
		for (auto entry : m_entry_list)
		{
			fprintf(pFile, "%s: %s\n", entry.key.c_str(), entry.val.c_str());
		}

		fclose(pFile);
	}
}

