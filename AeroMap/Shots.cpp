// Shots.cpp
// Port of odm shots.py
//

#include "Shots.h"

cv::Mat Shots::get_rotation_matrix(cv::Vec3d vec_rot)
{
	// Get rotation as a 3x3 matrix.
	//

	cv::Mat mat;

	// Converts a rotation matrix to a rotation vector or vice versa
	cv::Rodrigues(vec_rot, mat);

	return mat;
}

cv::Vec3d Shots::get_origin(cv::Vec3d vec_rot, cv::Vec3d vec_trx)
{
	// The origin of the pose in world coordinates.
	//

	cv::Mat mat_rot = get_rotation_matrix(vec_rot);

	// from numpy doc: If a is an N-D array and b is a 1-D array, it is a sum product over the last axis of a and b.
	cv::Vec3d vec_dot;
	vec_dot[0] = (vec_trx[0] * mat_rot.at<double>(0, 0) + vec_trx[1] * mat_rot.at<double>(1, 0) + vec_trx[2] * mat_rot.at<double>(2, 0));
	vec_dot[1] = (vec_trx[0] * mat_rot.at<double>(0, 1) + vec_trx[1] * mat_rot.at<double>(1, 1) + vec_trx[2] * mat_rot.at<double>(2, 1));
	vec_dot[2] = (vec_trx[0] * mat_rot.at<double>(0, 2) + vec_trx[1] * mat_rot.at<double>(1, 2) + vec_trx[2] * mat_rot.at<double>(2, 2));

	return -vec_dot;
}

//def matrix_to_rotation(rotation_matrix):
//    R = np.array(rotation_matrix, dtype=float)
//    # if not np.isclose(np.linalg.det(R), 1):
//    #     raise ValueError("Determinant != 1")
//    # if not np.allclose(np.linalg.inv(R), R.T):
//    #     raise ValueError("Not orthogonal")
//    return cv2.Rodrigues(R)[0].ravel()

//def get_geojson_shots_from_opensfm(reconstruction_file, utm_srs=None, utm_offset=None, pseudo_geotiff=None, a_matrix=None):
//    """
//    Extract shots from OpenSfM's reconstruction.json
//    """
//    pseudo_geocoords = None
//
//    if pseudo_geotiff is not None and os.path.exists(pseudo_geotiff):
//        # pseudogeo transform
//        utm_srs = get_pseudogeo_utm()
//
//        # the pseudo-georeferencing CRS UL corner is at 0,0
//        # but our shot coordinates aren't, so we need to offset them
//        raster = gdal.Open(pseudo_geotiff)
//        ulx, xres, _, uly, _, yres  = raster.GetGeoTransform()
//        lrx = ulx + (raster.RasterXSize * xres)
//        lry = uly + (raster.RasterYSize * yres)
//
//        pseudo_geocoords = np.array([[1.0 / get_pseudogeo_scale() ** 2, 0, 0, ulx + lrx / 2.0],
//                              [0, 1.0 / get_pseudogeo_scale() ** 2, 0, uly + lry / 2.0],
//                              [0, 0, 1, 0],
//                              [0, 0, 0, 1]])
//        raster = None
//        pseudo = True
//
//    # Couldn't get a SRS?
//    if utm_srs is None:
//        return None
//
//    crstrans = transformer(CRS.from_proj4(utm_srs), CRS.from_epsg("4326"))
//
//    if os.path.exists(reconstruction_file):
//        with open(reconstruction_file, 'r') as fin:
//            reconstructions = json.loads(fin.read())
//
//            feats = []
//            added_shots = {}
//            for recon in reconstructions:
//                cameras = recon.get('cameras', {})
//
//                for filename in recon.get('shots', {}):
//                    shot = recon['shots'][filename]
//                    cam = shot.get('camera')
//                    if (not cam in cameras) or (filename in added_shots):
//                        continue
//
//                    cam = cameras[cam]
//                    if pseudo_geocoords is not None:
//                        Rs, T = pseudo_geocoords[:3, :3], pseudo_geocoords[:3, 3]
//                        Rs1 = np.linalg.inv(Rs)
//                        origin = get_origin(shot)
//
//                        # Translation
//                        utm_coords = np.dot(Rs, origin) + T
//                        trans_coords = crstrans.TransformPoint(utm_coords[0], utm_coords[1], utm_coords[2])
//
//                        # Rotation
//                        rotation_matrix = get_rotation_matrix(np.array(shot['rotation']))
//                        rotation = matrix_to_rotation(np.dot(rotation_matrix, Rs1))
//
//                        translation = origin
//                    else:
//                        rotation = shot['rotation']
//
//                        # Just add UTM offset
//                        origin = get_origin(shot)
//
//                        utm_coords = [origin[0] + utm_offset[0],
//                                       origin[1] + utm_offset[1],
//                                       origin[2]]
//
//                        if a_matrix is not None:
//                            rotation = list(np.array(rotation).dot(a_matrix[:3,:3]))
//                            utm_coords = list(a_matrix.dot(np.hstack((np.array(utm_coords), 1)))[:-1])
//
//                        translation = utm_coords
//                        trans_coords = crstrans.TransformPoint(utm_coords[0], utm_coords[1], utm_coords[2])
//
//                    feats.append({
//                        'type': 'Feature',
//                        'properties': {
//                            'filename': filename,
//                            'focal': cam.get('focal', cam.get('focal_x')), # Focal ratio = focal length (mm) / max(sensor_width, sensor_height) (mm)
//                            'width': cam.get('width', 0),
//                            'height': cam.get('height', 0),
//                            'capture_time': shot.get('capture_time', 0),
//                            'translation': list(translation),
//                            'rotation': list(rotation)
//                        },
//                        'geometry':{
//                            'type': 'Point',
//                            'coordinates': list(trans_coords)
//                        }
//                    })
//
//                    added_shots[filename] = True
//
//        return {
//            'type': 'FeatureCollection',
//            'features': feats
//        }
//    else:
//        raise RuntimeError("%s does not exist." % reconstruction_file)

//def merge_geojson_shots(geojson_shots_files, output_geojson_file):
//    result = {}
//    added_files = {}
//    for shot_file in geojson_shots_files:
//        with open(shot_file, "r") as f:
//            shots = json.loads(f.read())
//
//        if len(result) == 0:
//            for feat in shots.get('features', []):
//                added_files[feat['properties']['filename']] = True
//
//            # Use first file as base
//            result = shots
//        else:
//            # Append features if filename not already added
//            for feat in shots.get('features', []):
//                if not feat['properties']['filename'] in added_files:
//                    result['features'].append(feat)
//
//    with open(output_geojson_file, "w") as f:
//        f.write(json.dumps(result))
