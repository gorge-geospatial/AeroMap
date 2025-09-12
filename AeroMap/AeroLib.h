#ifndef AEROLIB_H
#define AEROLIB_H

#include "AeroMap.h"
#include "exif.h"		// easy exif header

namespace AeroLib
{
	int  RunProgram(const char* prog, QStringList args, const char* output_file = nullptr);
	void InitRunLog();
	void LogWrite(const char* text, ...);

	struct Georef
	{
		int utm_zone;
		char hemi;
		double x;
		double y;
		bool is_valid;

		Georef()
			: utm_zone(0)
			, hemi(' ')
			, x(0.0)
			, y(0.0)
			, is_valid(false)
		{

		}
	};

	Georef ReadGeoref();

	double  CalcFocalRatio(easyexif::EXIFInfo exif);
	__int64 CalcUnixEpoch(const char* dateTime);
	XString GetCameraString(easyexif::EXIFInfo exif, bool opensfm);
	int     GetDepthMapResolution(std::vector<Photo*> photos);

	XString GetProj(int utm_zone, char hemi = 'N');
	XString GetProj(Georef georef);

	double Median(std::vector<double>& v);
	double Mean(std::vector<double>& v);

	double get_max_memory(int minimum = 5, double use_at_most = 0.5);
	int get_max_memory_mb(int minimum = 100, double use_at_most = 0.5);
	unsigned __int64 get_total_memory();

	void CreateFolder(XString path);
	void DeleteFolder(XString path);
	void Replace(XString src_file, XString dest_file);
	bool FileExists(XString file_name);
	void RemoveFile(XString file_name);
	void Touch(XString file_name);
	XString FileExt(XString file_name);
	XString FileRoot(XString file_name);
	XString related_file_path(XString input_file_path, XString prefix = "", XString postfix = "");
}

#endif // #ifndef AEROLIB_H
