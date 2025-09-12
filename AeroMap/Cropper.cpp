// Cropper.cpp
// Port of odm cropper.py
//

#include <fstream>
#include <nlohmann/json.h>
using json = nlohmann::json;

#include "AeroLib.h"
#include "PointCloud.h"
#include "Cropper.h"

Cropper::Cropper(XString storage_dir, XString files_prefix)
{
	m_storage_dir = storage_dir;
	m_files_prefix = files_prefix;
}

XString Cropper::crop(XString gpkg_path, XString geotiff_path, bool keep_original)
{
	if (AeroLib::FileExists(gpkg_path) == false)
	{
		Logger::Write(__FUNCTION__, "'%s' does not exist, will skip cropping.", gpkg_path.c_str());
		return geotiff_path;
	}
	if (AeroLib::FileExists(geotiff_path) == false)
	{
		Logger::Write(__FUNCTION__, "'%s' does not exist, will skip cropping.", geotiff_path.c_str());
		return geotiff_path;
	}

	Logger::Write(__FUNCTION__, "Cropping %s", geotiff_path.c_str());

	// Rename original file
	// path/to/odm_orthophoto.tif --> path/to/odm_orthophoto.original.tif

	XString path = geotiff_path.GetPathName();
	XString filename = geotiff_path.GetFileName();
	// path = path/to
	// filename = odm_orthophoto.tif

	XString basename;
	XString ext;
	int pos = filename.ReverseFind('.');
	if (pos > -1)
	{
		basename = filename.Left(pos);
		ext = filename.Mid(pos);
		// basename = odm_orthophoto
		// ext = .tif
	}

	XString original_geotiff = XString::CombinePath(path, XString::Format("%s.original%s", basename.c_str(), ext.c_str()));
	AeroLib::Replace(geotiff_path, original_geotiff);

	try
	{
		QStringList args;
		args.push_back("-cutline");
		args.push_back(gpkg_path.c_str());
		args.push_back("-crop_to_cutline");
		args.push_back("-co");
		args.push_back("TILED=YES");
		args.push_back("-co");
		args.push_back("COMPRESS=DEFLATE");
		args.push_back("-co");
		args.push_back("BLOCKXSIZE=512");
		args.push_back("-co");
		args.push_back("BLOCKYSIZE=512");
		args.push_back("-co");
		args.push_back("BIGTIFF=IF_SAFER");
		args.push_back("-co");
		args.push_back(XString::Format("NUM_THREADS=%d", arg.max_concurrency).c_str());
		args.push_back(original_geotiff.c_str());		// input file
		args.push_back(geotiff_path.c_str());			// output file
		args.push_back("--config");
		args.push_back("GDAL_CACHEMAX");
		args.push_back("35.4%");						//TODO: get_max_memory()
		AeroLib::RunProgram(tree.prog_gdal_warp, args);
		// cmd: gdalwarp 
		//			-cutline "d:\test\georeference\georeferenced_model.bounds.gpkg" 
		//			-crop_to_cutline
		//			-co TILED=YES -co COMPRESS=DEFLATE -co BLOCKXSIZE=512 -co BLOCKYSIZE=512 -co BIGTIFF=IF_SAFER -co NUM_THREADS=16
		//			"d:\test\odm_dem\dsm.original.tif" 
		//			"d:\test\odm_dem\dsm.tif" 
		//			--config GDAL_CACHEMAX 35.4%

		if (keep_original == false)
			AeroLib::RemoveFile(original_geotiff);
	}
	catch (std::exception ex)
	{
        Logger::Write(__FUNCTION__, "Something went wrong while cropping: %s", ex.what());

        // Revert rename
		AeroLib::Replace(original_geotiff, geotiff_path);
	}

	return geotiff_path;
}

XString Cropper::path(XString suffix)
{
	// Return a path relative to storage_dir and prefixed with files_prefix
	//

	return XString::CombinePath(m_storage_dir, XString::Format("%s.%s", m_files_prefix.c_str(), suffix.c_str()));
}

void Cropper::merge_bounds(std::vector<XString> input_bound_files, XString output_bounds, int buffer_distance)
{
	// Merge multiple bound files into a single bound computed from the convex hull
	// of all bounds (minus a buffer distance in meters)
	//

	OGRGeometryCollection geomcol;

	GDALDriver* driver = static_cast<GDALDriver*>(GDALGetDriverByName("GPKG"));
	OGRSpatialReference* srs = nullptr;

    for (auto input_bound_file : input_bound_files)
	{
		GDALDataset* ds = (GDALDataset*)GDALOpen(input_bound_file.c_str(), GA_ReadOnly);

		OGRLayer* layer = ds->GetLayer(0);
		srs = layer->GetSpatialRef();

		// Collect all Geometry
		OGRGeometryCollection geomcol;
		for (int i = 0; i < layer->GetFeatureCount(); ++i)
			geomcol.addGeometry(layer->GetFeature(i)->GetGeometryRef());

		GDALClose(ds);
	}

    // Calculate convex hull
	OGRGeometry* convexhull = geomcol.ConvexHull();

    // If buffer distance is specified
    // Create two buffers, one shrunk by
    // N + 3 and then that buffer expanded by 3
    // so that we get smooth corners. \m/
	const int BUFFER_SMOOTH_DISTANCE = 3;

    if (buffer_distance > 0.0)
	{
		convexhull = convexhull->Buffer(-(buffer_distance + BUFFER_SMOOTH_DISTANCE));
		convexhull = convexhull->Buffer(BUFFER_SMOOTH_DISTANCE);
	}

    // Save to a new file
	AeroLib::RemoveFile(output_bounds);

	GDALDataset* out_ds = driver->Create(output_bounds.c_str(), 0, 0, 1, GDALDataType::GDT_Float32, nullptr);
	OGRLayer* layer = out_ds->CreateLayer("convexhull", srs, wkbPolygon);

	OGRFeatureDefn* feature_def = layer->GetLayerDefn();
	OGRFeature feature(feature_def);
	feature.SetGeometry(convexhull);
	layer->CreateFeature(&feature);

    // Save and close output data source
	GDALClose(out_ds);
}

XString Cropper::create_bounds_geojson(XString pointcloud_path, int buffer_distance, int decimation_step)
{
	// Compute a buffered polygon around the data extents (not just a bounding box)
	// of the given point cloud.
	//
	// return filename to GeoJSON containing the polygon
	//
	
	if (AeroLib::FileExists(pointcloud_path) == false)
	{
		Logger::Write(__FUNCTION__, "Point cloud does not exist, cannot generate bounds: '%s'", pointcloud_path.c_str());
		return "";
	}

	// Do decimation prior to extracting boundary information
	XString decimated_pointcloud_path = path("decimated.las");
	
	QStringList args;
	args.push_back("translate");
	args.push_back("-i");
	args.push_back(pointcloud_path.c_str());
	args.push_back("-o");
	args.push_back(decimated_pointcloud_path.c_str());
	args.push_back("decimation");
	args.push_back(XString::Format("--filters.decimation.step=%d", decimation_step).c_str());
	AeroLib::RunProgram(tree.prog_pdal, args);
	// cmd: pdal translate
	//			-i "d:\test\georeference\georeferenced_model.laz" 
	//			-o "d:\test\georeference\georeferenced_model.decimated.las" 
	//			decimation 
	//			--filters.decimation.step=40 
	
	if (AeroLib::FileExists(decimated_pointcloud_path) == false)
	{
		Logger::Write(__FUNCTION__, "Could not decimate point cloud, thus cannot generate GPKG bounds: '%s'", decimated_pointcloud_path.c_str());
		return "";
	}

	// Use PDAL to dump boundary information
	// then read the information back
	
	XString boundary_file_path = path("boundary.json");

	args.clear();
	args.push_back("info");
	args.push_back("--boundary");
	args.push_back("--filters.hexbin.edge_size=1");
	args.push_back("--filters.hexbin.threshold=0");
	args.push_back(decimated_pointcloud_path.c_str());
	AeroLib::RunProgram(tree.prog_pdal, args, boundary_file_path);
	// cmd: pdal info
	//			--boundary
	//			--filters.hexbin.edge_size=1 
	//			--filters.hexbin.threshold=0
	//			"d:\test\georeference\georeferenced_model.decimated.las" > "d:\test\georeference\georeferenced_model.boundary.json"
	
	json pc_geojson_boundary_feature = nullptr;
	std::ifstream f(boundary_file_path.c_str());
	json data = json::parse(f);
	if (data.contains("boundary"))
	{
		pc_geojson_boundary_feature = data["boundary"]["boundary_json"];
	}
	if (pc_geojson_boundary_feature == nullptr)
	{
		Logger::Write(__FUNCTION__, "Could not determine point cloud boundaries");
		assert(false);
	}
	
	// Write bounds to GeoJSON
	XString tmp_bounds_geojson_path = path("tmp-bounds.geojson");
	FILE* pFile = fopen(tmp_bounds_geojson_path.c_str(), "wt");
	if (pFile)
	{
		FILE* pFile = fopen(tmp_bounds_geojson_path, "wt");
		if (pFile)
		{
			fprintf(pFile, "{\n");
			fprintf(pFile, "    \"type\": \"FeatureCollection\",\n");
			fprintf(pFile, "    \"features\":\n");
			fprintf(pFile, "    [\n");
			fprintf(pFile, "        {\n");
			fprintf(pFile, "            \"type\": \"Feature\",\n");
			fprintf(pFile, "            \"geometry\": %s", pc_geojson_boundary_feature.dump().c_str());
			fprintf(pFile, "        }\n");
			fprintf(pFile, "    ]\n");
			fprintf(pFile, "}\n");
			fclose(pFile);
		}

		// write a file like:
		//{
		//	"type": "FeatureCollection",
		//	"features":
		//	[
		//		{
		//			"type": "Feature",
		//			"geometry":
		//			{
		//				"type": "MultiPolygon",
		//				"coordinates": [[[[.. list of polylines ..]]]]
		//			}
		//		}
		//	]
		//}
	}
	
	// Create a convex hull around the boundary
	// as to encompass the entire area (no holes)
	GDALDriver* driver = static_cast<GDALDriver*>(GDALGetDriverByName("GeoJSON"));
	GDALDataset* ds = (GDALDataset*)GDALOpenEx(tmp_bounds_geojson_path.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
	OGRLayer* layer = ds->GetLayer(0);
	
	// Collect all Geometry
	OGRGeometryCollection geomcol;
	for (int i = 0; i < layer->GetFeatureCount(); ++i)
		geomcol.addGeometry(layer->GetFeature(i)->GetGeometryRef());
	
	// Calculate convex hull
	OGRGeometry* convexhull = geomcol.ConvexHull();

	// If buffer distance is specified
	// Create two buffers, one shrunk by
	// N + 3 and then that buffer expanded by 3
	// so that we get smooth corners. \m/
	const int BUFFER_SMOOTH_DISTANCE = 3;

	if (buffer_distance > 0)
	{
		// For small areas, check that buffering doesn't obliterate
		// our hull
		OGRGeometry* tmp = convexhull->Buffer(-(buffer_distance + BUFFER_SMOOTH_DISTANCE));
		tmp = tmp->Buffer(BUFFER_SMOOTH_DISTANCE);
		OGRMultiPolygon* pMulti = (OGRMultiPolygon*)tmp;
		if (pMulti->get_Area() > 0.0)
			convexhull = tmp;
		else
			Logger::Write(__FUNCTION__, "Very small crop area detected, it will not be smoothed.");
	}

	// Save to a new file
	XString bounds_geojson_path = path("bounds.geojson");
	AeroLib::RemoveFile(bounds_geojson_path);

	GDALDataset* out_ds = driver->Create(bounds_geojson_path.c_str(), 0, 0, 1, GDALDataType::GDT_Float32, nullptr);
	layer = out_ds->CreateLayer("convexhull", nullptr, wkbPolygon);


	OGRFeatureDefn* feature_def = layer->GetLayerDefn();
	OGRFeature feature(feature_def);
	feature.SetGeometry(convexhull);
	layer->CreateFeature(&feature);

	// Save and close data sources
	GDALClose(out_ds);
	GDALClose(ds);

	// Remove decimated point cloud
	// if AeroLib::FileExists(decimated_pointcloud_path):
	//     os.remove(decimated_pointcloud_path)

	// Remove tmp bounds
	// if AeroLib::FileExists(tmp_bounds_geojson_path):
	//     os.remove(tmp_bounds_geojson_path)

	return bounds_geojson_path;
}

XString Cropper::create_bounds_gpkg(XString pointcloud_path, int buffer_distance, int decimation_step)
{
	// Compute a buffered polygon around the data extents (not just a bounding box)
	// of the given point cloud.
	//
	// return filename to Geopackage containing the polygon
	//

	if (AeroLib::FileExists(pointcloud_path) == false)
	{
		Logger::Write(__FUNCTION__, "Point cloud does not exist, cannot generate GPKG bounds: '%'", pointcloud_path.c_str());
		return "";
	}

	XString bounds_geojson_path = create_bounds_geojson(pointcloud_path, buffer_distance, decimation_step);

	XString summary_file_path = XString::CombinePath(m_storage_dir, XString::Format("%s.summary.json", m_files_prefix.c_str()));
	PointCloud::export_summary_json(pointcloud_path, summary_file_path);

	json pc_proj4 = nullptr;
	std::ifstream f(summary_file_path.c_str());
	json data = json::parse(f);
	if (data.contains("summary"))
	{
		pc_proj4 = data["summary"]["srs"]["proj4"];
	}
	if (pc_proj4 == nullptr)
	{
		Logger::Write(__FUNCTION__, "Could not determine point cloud proj4 declaration");
		assert(false);
	}

	XString bounds_gpkg_path = XString::CombinePath(m_storage_dir, XString::Format("%s.bounds.gpkg", m_files_prefix.c_str()));
	AeroLib::RemoveFile(bounds_gpkg_path);

	// Convert bounds to GPKG
	std::string proj_str = pc_proj4.template get<std::string>();

	QStringList args;
	args.push_back("-overwrite");
	args.push_back("-f");
	args.push_back("GPKG");
	args.push_back("-a_srs");
	args.push_back(proj_str.c_str());
	args.push_back(bounds_gpkg_path.c_str());			// output file
	args.push_back(bounds_geojson_path.c_str());		// input file
	AeroLib::RunProgram(tree.prog_ogr2ogr, args);
	// cmd: ogr2ogr 
	//			-overwrite
	//			-f GPKG
	//			-a_srs "+proj=utm +zone=32 +datum=WGS84 +units=m +no_defs" 
	//			"d:\test\georeference\georeferenced_model.bounds.gpkg" 
	//			"d:\test\georeference\georeferenced_model.bounds.geojson"

	return bounds_gpkg_path;
}
