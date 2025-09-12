#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include "AeroMap.h"		// application header

class PointCloud
{
public:

	static void filter(XString input_point_cloud, XString output_point_cloud, XString output_stats, double stddev = 2.5, double sample_radius = 0, int max_concurrency = 1 /* boundary=None */);
	static RectD get_extent(XString input_point_cloud);
	static double get_spacing(XString stats_file, double resolution_fallback = 5.0);

	static void export_info_json(XString point_cloud_path, XString info_file_path);
	static void export_summary_json(XString point_cloud_path, XString summary_file_path);
	static void post_point_cloud_steps(bool rerun = false);
};

#endif // #ifndef POINTCLOUD_H
