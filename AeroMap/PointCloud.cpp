// PointCloud.cpp
// Port of odm point_cloud.py
//

#include <fstream>
#include <nlohmann/json.h>
using json = nlohmann::json;

#include "AeroLib.h"
#include "PointCloud.h"

void PointCloud::filter(XString input_point_cloud, XString output_point_cloud, XString output_stats,
	double stddev, double sample_radius, int max_concurrency)
{
	// Filters a point cloud
	//
	
	if (AeroLib::FileExists(input_point_cloud) == false)
	{
		Logger::Write(__FUNCTION__, "%s does not exist. Process cannot continue.", input_point_cloud.c_str());
		//TODO:
		//need to be able to set cancel photo proc flag
		assert(false);
	}

	int meank = 16;
	Logger::Write(__FUNCTION__, "Filtering '%s' (statistical, meanK %d, standard deviation %0.1f)", input_point_cloud.c_str(), meank, stddev);

	//if boundary is not None:
	{
	//	Logger::Write(__FUNCTION__, "Boundary {}".format(boundary))
	//	fd, boundary_json_file = tempfile.mkstemp(suffix='.boundary.json')
	//	os.close(fd)
	//	with open(boundary_json_file, 'w') as f:
	//		f.write(as_geojson(boundary))
	//	args.append('--boundary "%s"' % boundary_json_file)
	}

	QStringList args;
	args.push_back("--input");
	args.push_back(input_point_cloud.c_str());
	args.push_back("--output");
	args.push_back(output_point_cloud.c_str());
	args.push_back("--stats");
	args.push_back(output_stats.c_str());
	args.push_back("--concurrency");
	args.push_back(XString::Format("%d", arg.max_concurrency).c_str());
	args.push_back("--meank");
	args.push_back(XString::Format("%d", meank).c_str());
	args.push_back("--std");
	args.push_back(XString::Format("%0.1f", stddev).c_str());
	if (sample_radius > 0.0)
	{
		Logger::Write(__FUNCTION__, "Sampling points around a %0.1f m radius", sample_radius);
		args.push_back("--radius");
		args.push_back(XString::Format("%0.1f", sample_radius).c_str());
	}
	AeroLib::RunProgram(tree.prog_filter, args);
	// cmd: "FPCFilter"
	//			--input "d:\test\opensfm\undistorted\openmvs\scene_dense_dense_filtered.ply"
	//			--output "d:\test\odm_filterpoints\point_cloud.ply" 
	//			--stats "d:\test\odm_filterpoints\point_cloud_stats.json"
	//			--concurrency 16 --meank 16 --std 2.5 

	if (AeroLib::FileExists(output_point_cloud) == false)
	{
		Logger::Write(__FUNCTION__, "'%s' not found, filtering has failed.", output_point_cloud.c_str());
	}
}

RectD PointCloud::get_extent(XString input_point_cloud)
{
	// Get point cloud extent
	//

	RectD extent;

	XString json_file = XString::CombinePath(GetProject().GetDroneOutputPath(), "pdal_info.json");

	bool fallback = false;

	// PLY files do not have --summary support
	if (input_point_cloud.EndsWithNoCase(".ply"))
	{
		fallback = true;

		QStringList args;
		args.push_back("info");
		args.push_back(input_point_cloud.c_str());
		AeroLib::RunProgram(tree.prog_pdal, args, json_file);
		//run('pdal info "{0}" > "{1}"'.format(input_point_cloud, json_file))
	}
	//try:
	if (fallback == false)
	{
		QStringList args;
		args.push_back("info");
		args.push_back("--summary");
		args.push_back(input_point_cloud.c_str());
		AeroLib::RunProgram(tree.prog_pdal, args, json_file);
		//run('pdal info --summary "{0}" > "{1}"'.format(input_point_cloud, json_file))
	}
	//except:
	//    fallback = True
	//    run('pdal info "{0}" > "{1}"'.format(input_point_cloud, json_file))

	std::ifstream f(json_file.c_str());
	json data = json::parse(f);

	json bounds = nullptr;
	if (fallback == false)
	{
		json summary = data["summary"];
		if (summary == nullptr)
		{
			Logger::Write(__FUNCTION__, "Cannot compute summary for '%s' (summary key missing)", input_point_cloud.c_str());
		}
		else
		{
			bounds = summary["bounds"];
		}
	}
	else
	{
		json stats = data["stats"];
		if (stats == nullptr)
		{
			Logger::Write(__FUNCTION__, "Cannot compute bounds for '%s' (stats key missing)", input_point_cloud.c_str());
		}
		else
		{
			json bbox = stats["bbox"];
			if (bbox == nullptr)
			{
				Logger::Write(__FUNCTION__, "Cannot compute bounds for '%s' (bbox key missing)", input_point_cloud.c_str());
			}
			else
			{
				json native = bbox["native"];
				if (native == nullptr)
				{
					Logger::Write(__FUNCTION__, "Cannot compute bounds for '%s' (native key missing)", input_point_cloud.c_str());
				}
				else
				{
					bounds = native["bbox"];
				}
			}
		}
	}

	if (bounds == nullptr)
	{
		Logger::Write(__FUNCTION__, "Cannot compute bounds for '%s' (bounds key missing)", input_point_cloud.c_str());
		assert(false);
	}
	else
	{
		if ((bounds.contains("minx") == false) ||
			(bounds.contains("maxx") == false) ||
			(bounds.contains("miny") == false) ||
			(bounds.contains("maxy") == false))
		{
			Logger::Write(__FUNCTION__, "Cannot compute bounds for '%s' (min/max values missing)", input_point_cloud.c_str());
			assert(false);
		}
		else
		{
			extent.x0 = bounds["minx"];
			extent.x1 = bounds["maxx"];
			extent.y0 = bounds["miny"];
			extent.y1 = bounds["maxy"];
		}
	}

	//os.remove(json_file)

	return extent;
}

double PointCloud::get_spacing(XString stats_file, double resolution_fallback)
{
	double spacing = 0.0;
	
	double fallback = (resolution_fallback / 100.0) / 2.0;
	
	if (AeroLib::FileExists(stats_file) == false)
	{
		Logger::Write(__FUNCTION__, "'%s' not found, using fallback value: %0.2f", fallback);
		return fallback;
	}

	std::ifstream f(stats_file.c_str());
	json data = json::parse(f);

	if (data.contains("spacing") == false)
	{
		Logger::Write(__FUNCTION__, "'%s' missing 'spacing' entry, using fallback value: %0.2f", fallback);
		return fallback;
	}

	double d = data["spacing"];
	if (d <= 0.0)
	{
		Logger::Write(__FUNCTION__, "'%s' has invalid 'spacing' entry (%0.2f), using fallback value: %0.2f", d, fallback);
		return fallback;
	}

	spacing = d;
	//return round(d, 3)

	return spacing;
}

//def ply_info(input_ply):
//    if not os.path.exists(input_ply):
//        raise IOError("%s does not exist" % input_ply)
//
//    # Read PLY header, check if point cloud has normals
//    has_normals = False
//    has_views = False
//    vertex_count = 0
//
//    with open(input_ply, 'r', errors='ignore') as f:
//        line = f.readline().strip().lower()
//        i = 0
//        while line != "end_header":
//            line = f.readline().strip().lower()
//            props = line.split(" ")
//            if len(props) == 3:
//                if props[0] == "property" and props[2] in ["nx", "normalx", "normal_x"]:
//                    has_normals = True
//                if props[0] == "property" and props[2] in ["views"]:
//                    has_views = True
//                elif props[0] == "element" and props[1] == "vertex":
//                    vertex_count = int(props[2])
//            i += 1
//            if i > 100:
//                raise IOError("Cannot find end_header field. Invalid PLY?")
//
//
//    return {
//        'has_normals': has_normals,
//        'vertex_count': vertex_count,
//        'has_views': has_views,
//        'header_lines': i + 1
//    }

//def split(input_point_cloud, outdir, filename_template, capacity, dims=None):
//    Logger::Write(__FUNCTION__, "Splitting point cloud filtering in chunks of {} vertices".format(capacity))
//
//    if not os.path.exists(input_point_cloud):
//        log.ODM_ERROR("{} does not exist, cannot split point cloud. The program will now exit.".format(input_point_cloud))
//        sys.exit(1)
//
//    if not os.path.exists(outdir):
//        system.mkdir_p(outdir)
//
//    if len(os.listdir(outdir)) != 0:
//        log.ODM_ERROR("%s already contains some files. The program will now exit.".format(outdir))
//        sys.exit(1)
//
//    cmd = 'pdal split -i "%s" -o "%s" --capacity %s ' % (input_point_cloud, XString::CombinePath(outdir, filename_template), capacity)
//
//    if filename_template.endswith(".ply"):
//        cmd += ("--writers.ply.sized_types=false "
//                "--writers.ply.storage_mode=\"little endian\" ")
//    if dims is not None:
//        cmd += '--writers.ply.dims="%s"' % dims
//    system.run(cmd)
//
//    return [XString::CombinePath(outdir, f) for f in os.listdir(outdir)]

void PointCloud::export_info_json(XString point_cloud_path, XString info_file_path)
{
	QStringList args;
	args.push_back("info");
	args.push_back("--dimensions");
	args.push_back("\"X,Y,Z\"");
	args.push_back(point_cloud_path.c_str());
	AeroLib::RunProgram(tree.prog_pdal, args, info_file_path);
	// system.run('pdal info --dimensions "X,Y,Z" "{0}" > "{1}"'.format(pointcloud_path, info_file_path))
}

void PointCloud::export_summary_json(XString point_cloud_path, XString summary_file_path)
{
	QStringList args;
	args.push_back("info");
	args.push_back("--summary");
	args.push_back(point_cloud_path.c_str());
	AeroLib::RunProgram(tree.prog_pdal, args, summary_file_path);
	//system.run('pdal info --summary "{0}" > "{1}"'.format(pointcloud_path, summary_file_path))
}

//def merge(input_point_cloud_files, output_file, rerun=False):
//    num_files = len(input_point_cloud_files)
//    if num_files == 0:
//        Logger::Write(__FUNCTION__, "No input point cloud files to process")
//        return
//
//    if io.file_exists(output_file):
//        Logger::Write(__FUNCTION__, "Removing previous point cloud: %s" % output_file)
//        os.remove(output_file)
//
//    kwargs = {
//        'all_inputs': " ".join(map(double_quote, input_point_cloud_files)),
//        'output': output_file
//    }
//
//    system.run('lasmerge -i {all_inputs} -o "{output}"'.format(**kwargs))

//def fast_merge_ply(input_point_cloud_files, output_file):
//    # Assumes that all input files share the same header/content format
//    # As the merge is a naive byte stream copy
//
//    num_files = len(input_point_cloud_files)
//    if num_files == 0:
//        Logger::Write(__FUNCTION__, "No input point cloud files to process")
//        return
//
//    if io.file_exists(output_file):
//        Logger::Write(__FUNCTION__, "Removing previous point cloud: %s" % output_file)
//        os.remove(output_file)
//
//    vertex_count = sum([ply_info(pcf)['vertex_count'] for pcf in input_point_cloud_files])
//    master_file = input_point_cloud_files[0]
//    with open(output_file, "wb") as out:
//        with open(master_file, "r", errors="ignore") as fhead:
//            # Copy header
//            line = fhead.readline()
//            out.write(line.encode('utf8'))
//
//            i = 0
//            while line.strip().lower() != "end_header":
//                line = fhead.readline()
//
//                # Intercept element vertex field
//                if line.lower().startswith("element vertex "):
//                    out.write(("element vertex %s\n" % vertex_count).encode('utf8'))
//                else:
//                    out.write(line.encode('utf8'))
//
//                i += 1
//                if i > 100:
//                    raise IOError("Cannot find end_header field. Invalid PLY?")
//
//        for ipc in input_point_cloud_files:
//            i = 0
//            with open(ipc, "rb") as fin:
//                # Skip header
//                line = fin.readline()
//                while line.strip().lower() != b"end_header":
//                    line = fin.readline()
//
//                    i += 1
//                    if i > 100:
//                        raise IOError("Cannot find end_header field. Invalid PLY?")
//
//                # Write fields
//                out.write(fin.read())
//
//    return output_file

//def merge_ply(input_point_cloud_files, output_file, dims=None):
//    num_files = len(input_point_cloud_files)
//    if num_files == 0:
//        Logger::Write(__FUNCTION__, "No input point cloud files to process")
//        return
//
//    cmd = [
//        'pdal',
//        'merge',
//        '--writers.ply.sized_types=false',
//        '--writers.ply.storage_mode="little endian"',
//        ('--writers.ply.dims="%s"' % dims) if dims is not None else '',
//        ' '.join(map(double_quote, input_point_cloud_files + [output_file])),
//    ]
//
//    system.run(' '.join(cmd))

void PointCloud::post_point_cloud_steps(bool rerun)
{
    //# XYZ point cloud output
    //if args.pc_csv:
    //    Logger::Write(__FUNCTION__, "Creating CSV file (XYZ format)")

    //    if not io.file_exists(tree.georef_path_xyz_file) or rerun:
    //        system.run("pdal translate -i \"{}\" "
    //            "-o \"{}\" "
    //            "--writers.text.format=csv "
    //            "--writers.text.order=\"X,Y,Z\" "
    //            "--writers.text.keep_unspecified=false ".format(
    //                tree.georef_path_model_laz,
    //                tree.georef_path_xyz_file))
    //    else:
    //        Logger::Write(__FUNCTION__, "Found existing CSV file %s" % tree.georef_path_xyz_file)

    //# LAS point cloud output
    //if args.pc_las:
    //    Logger::Write(__FUNCTION__, "Creating LAS file")

    //    if not io.file_exists(tree.georef_path_model_las) or rerun:
    //        system.run("pdal translate -i \"{}\" "
    //            "-o \"{}\" ".format(
    //                tree.georef_path_model_laz,
    //                tree.georef_path_model_las))
    //    else:
    //        Logger::Write(__FUNCTION__, "Found existing LAS file %s" % tree.georef_path_xyz_file)

    //# EPT point cloud output
    //if args.pc_ept:
    //    Logger::Write(__FUNCTION__, "Creating Entwine Point Tile output")
    //    entwine.build([tree.georef_path_model_laz], tree.entwine_pointcloud, max_concurrency=args.max_concurrency, rerun=rerun)

    //# COPC point clouds
    //if args.pc_copc:
    //    Logger::Write(__FUNCTION__, "Creating Cloud Optimized Point Cloud (COPC)")

    //    copc_output = io.related_file_path(tree.georef_path_model_laz, postfix=".copc")
    //    entwine.build_copc([tree.georef_path_model_laz], copc_output, convert_rgb_8_to_16=True)
}
