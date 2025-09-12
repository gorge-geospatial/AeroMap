// StageGeoref.cpp
// Georeference point cloud.
//

#include "Cropper.h"
#include "PointCloud.h"
#include "StageGeoref.h"

int StageGeoref::Run()
{
	// Inputs:
	// Outputs:
	//		+ georeference
	//			georeferenced_model.laz
	//

	int status = 0;

	GetApp()->LogWrite("Georeference...");
	BenchmarkStart();
	LogStage();

	// Export GCP information if available
	XString gcp_export_file = XString::CombinePath(tree.georef_path, "ground_control_points.gpkg");
	//gcp_gml_export_file = tree.path("odm_georeferencing", "ground_control_points.gml")
	//gcp_geojson_export_file = tree.path("odm_georeferencing", "ground_control_points.geojson")
	//unaligned_model = io.related_file_path(tree.georef_path_model_laz, postfix="_unaligned")
	//if os.path.isfile(unaligned_model) and self.rerun():
	//    os.unlink(unaligned_model)

	if (GetReconstruction()->has_gcp() && (AeroLib::FileExists(gcp_export_file) == false) || Rerun())
	{
		//octx = OSFMContext(tree.opensfm)
		//gcps = octx.ground_control_points(reconstruction.georef.proj4())

		//if len(gcps):
		//    gcp_schema = {
		//        'geometry': 'Point',
		//        'properties': OrderedDict([
		//            ('id', 'str'),
		//            ('observations_count', 'int'),
		//            ('observations_list', 'str'),
		//            ('error_x', 'float'),
		//            ('error_y', 'float'),
		//            ('error_z', 'float'),
		//        ])
		//    }

		//    # Write GeoPackage
		//    with fiona.open(gcp_export_file, 'w', driver="GPKG",
		//                    crs=fiona.crs.from_string(reconstruction.georef.proj4()),
		//                    schema=gcp_schema) as f:
		//        for gcp in gcps:
		//            f.write({
		//                'geometry': {
		//                    'type': 'Point',
		//                    'coordinates': gcp['coordinates'],
		//                },
		//                'properties': OrderedDict([
		//                    ('id', gcp['id']),
		//                    ('observations_count', len(gcp['observations'])),
		//                    ('observations_list', ",".join([obs['shot_id'] for obs in gcp['observations']])),
		//                    ('error_x', gcp['error'][0]),
		//                    ('error_y', gcp['error'][1]),
		//                    ('error_z', gcp['error'][2]),
		//                ])
		//            })

		//    # Write GML
		//    try:
		//        system.run('ogr2ogr -of GML "{}" "{}"'.format(gcp_gml_export_file, gcp_export_file))
		//    except Exception as e:
		//        Logger::Write(__FUNCTION__,"Cannot generate ground control points GML file: %s" % str(e))

		//    # Write GeoJSON
		//    geojson = {
		//        'type': 'FeatureCollection',
		//        'features': []
		//    }

		//    from_srs = CRS.from_proj4(reconstruction.georef.proj4())
		//    to_srs = CRS.from_epsg(4326)
		//    transformer = location.transformer(from_srs, to_srs)

		//    for gcp in gcps:
		//        properties = gcp.copy()
		//        del properties['coordinates']

		//        geojson['features'].append({
		//            'type': 'Feature',
		//            'geometry': {
		//                'type': 'Point',
		//                'coordinates': transformer.TransformPoint(*gcp['coordinates']),
		//            },
		//            'properties': properties
		//        })

		//    with open(gcp_geojson_export_file, 'w') as f:
		//        f.write(json.dumps(geojson, indent=4))

		//else
		//    Logger::Write(__FUNCTION__,"GCPs could not be loaded for writing to %s" % gcp_export_file)
	}

	if ((AeroLib::FileExists(tree.georef_model_laz) == false) || Rerun())
	{
		AeroLib::Georef georef = AeroLib::ReadGeoref();
		if (georef.is_valid)
		{
			// build pdal pipeline

			XString pipe_file = XString::CombinePath(GetProject().GetDroneOutputPath(), "geo_pipe.json");
			FILE* pFile = fopen(pipe_file.c_str(), "wt");
			if (pFile)
			{
				fprintf(pFile, "[\n");

				// reader
				fprintf(pFile, "    {\n");
				fprintf(pFile, "        \"type\": \"readers.ply\",\n");
				fprintf(pFile, "        \"filename\": \"%s\"\n", tree.filtered_point_cloud.c_str());
				fprintf(pFile, "    },\n");

				// filters
				fprintf(pFile, "    {\n");
				fprintf(pFile, "        \"type\": \"filters.ferry\",\n");
				fprintf(pFile, "        \"dimensions\": \"views => UserData\"\n");
				fprintf(pFile, "    },\n");
				fprintf(pFile, "    {\n");
				fprintf(pFile, "        \"type\": \"filters.transformation\",\n");
				fprintf(pFile, "        \"matrix\": \"1 0 0 %0.1f 0 1 0 %0.f 0 0 1 0 0 0 0 1\"\n", georef.x, georef.y);
				fprintf(pFile, "    },\n");

				// writer
				fprintf(pFile, "    {\n");
				fprintf(pFile, "        \"type\": \"writers.las\",\n");
				fprintf(pFile, "        \"filename\": \"%s\",\n", tree.georef_model_laz.c_str());
				fprintf(pFile, "        \"offset_x\": %0.1f,\n", georef.x);
				fprintf(pFile, "        \"offset_y\": %0.1f,\n", georef.y);
				fprintf(pFile, "        \"offset_z\": 0.0,\n");
				fprintf(pFile, "        \"scale_x\": 0.001,\n");
				fprintf(pFile, "        \"scale_y\": 0.001,\n");
				fprintf(pFile, "        \"scale_z\": 0.001,\n");
				fprintf(pFile, "        \"a_srs\": \"+proj=utm +zone=%d +datum=WGS84 +units=m +no_defs +type=crs\"\n", georef.utm_zone);
				fprintf(pFile, "    }\n");

				fprintf(pFile, "]\n");
				fclose(pFile);
			}

			// run pipeline

			QStringList args;
			args.push_back("pipeline");
			args.push_back("-i");
			args.push_back(pipe_file.c_str());
			AeroLib::RunProgram(tree.prog_pdal, args);

			// original command:
			// pdal translate 
			//          -i "d:\test\odm_filterpoints\point_cloud.ply" 
			//          -o "d:\test\odm_georeferencing\georeferenced_model.laz"
			//          ferry transformation 
			//          --filters.ferry.dimensions="views => UserData"  
			//          --filters.transformation.matrix="1 0 0 322263.0 0 1 0 5157982.0 0 0 1 0 0 0 0 1" 
			//          --writers.las.offset_x=322263.0 --writers.las.offset_y=5157982.0 --writers.las.scale_x=0.001 --writers.las.scale_y=0.001 --writers.las.scale_z=0.001 
			//          --writers.las.offset_z=0 
			//          --writers.las.a_srs="+proj=utm +zone=32 +datum=WGS84 +units=m +no_defs +type=crs"

			// needed for gcps
			//        if reconstruction.has_gcp() and io.file_exists(gcp_geojson_export_file):
			//            Logger::Write(__FUNCTION__, "Embedding GCP info in point cloud")
			//            params += [
			//                '--writers.las.vlrs="{\\\"filename\\\": \\\"%s\\\", \\\"user_id\\\": \\\"ODM\\\", \\\"record_id\\\": 1, \\\"description\\\": \\\"Ground Control Points (GeoJSON)\\\"}"' % gcp_geojson_export_file.replace(os.sep, "/")
			//            ]

			if (arg.crop > 0)
			{
				Logger::Write(__FUNCTION__, "Calculating cropping area and generating bounds shapefile from point cloud");
				Cropper cropper(tree.georef_path, "georeferenced_model");

				int decimation_step = 40;
				if (arg.fast_orthophoto)
					decimation_step = 4;

				// More aggressive decimation for large datasets
				if (arg.fast_orthophoto == false)
				{
					decimation_step *= GetReconstruction()->GetImageCount() / 1000 + 1;
					decimation_step = std::min(decimation_step, 95);
				}

				try
				{
					cropper.create_bounds_gpkg(tree.georef_model_laz, arg.crop, decimation_step);
				}
				catch (std::exception ex)
				{
					Logger::Write(__FUNCTION__, "Cannot calculate crop bounds! We will skip cropping");
					arg.crop = 0.0;
				}
			}
			//if 'boundary' in outputs and args.crop == 0:
			{
				//Logger::Write(__FUNCTION__, "Using boundary JSON as cropping area")

				//bounds_base, _ = os.path.splitext(tree.georef_path_model_laz)
				//bounds_json = bounds_base + ".bounds.geojson"
				//bounds_gpkg = bounds_base + ".bounds.gpkg"
				//export_to_bounds_files(outputs['boundary'], reconstruction.get_proj_srs(), bounds_json, bounds_gpkg)
			}
		}
		else
		{
			//TODO:
			// would need pipeline here as well
			//        Logger::Write(__FUNCTION__, "Converting point cloud (non-georeferenced)")
			//        system.run(cmd + ' ' + ' '.join(stages) + ' ' + ' '.join(params))
		}

		//  stats_dir = tree.path("opensfm", "stats", "codem")
		//  if os.path.exists(stats_dir) and self.rerun():
		//      shutil.rmtree(stats_dir)

		//  if tree.odm_align_file is not None:
			//not implemented

		PointCloud::post_point_cloud_steps(Rerun());
	}
	else
	{
		Logger::Write(__FUNCTION__, "Found existing georeferenced model: '%s'", tree.georef_model_laz.c_str());
	}

	GetProject().Update();		// trigger ProjectWindow update
	BenchmarkStop();

	return status;
}

StageGeoref::StageGeoref(DroneProc* pDroneProc)
	: Stage(pDroneProc)
{
	m_stage_id = Id::Georeference;
}

StageGeoref::~StageGeoref()
{

}
