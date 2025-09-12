#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "GcpFile.h"
#include "Photo.h"

class Reconstruction
{
public:

	struct BandType
	{
		XString band_name;
		int band_index;
		std::vector<Photo*> photos;

		// operator for sorting
		bool operator<(const BandType& val) const
		{
			return band_index < val.band_index;
		}
	};
	typedef std::vector<BandType> MultiType;

	MultiType m_mc;

public:

	Reconstruction(const std::vector<Photo*> photos);
	~Reconstruction();

	bool is_georef();
	bool is_multi();
	bool has_gcp();

	void detect_multi_camera();
	void filter_photos();

	void georeference_with_gcp(XString gcp_file, XString output_coords_file, XString output_gcp_file, bool rerun);
	void georeference_with_gps(XString images_path, XString output_coords_file, bool rerun);

	int GetImageCount();
	const std::vector<Photo*>& GetImageList();

private:

	GcpFile* mp_GcpFile;
	std::vector<Photo*> m_photos;
};

#endif // #ifndef RECONSTRUCTION_H

