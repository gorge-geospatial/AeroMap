#pragma once

#include "AeroMap.h"

class Mesh
{
public:

	static XString screened_poisson_reconstruction(XString inPointCloud, XString outMesh, 
		int depth = 8, double samplesPerNode = 1.0,
		int maxVertexCount = 100000, double pointWeight = 4, int threads = arg.max_concurrency);

	static XString create_25dmesh(XString inPointCloud, XString outMesh,
		std::vector<double> radius_steps, double dsm_resolution = 0.05, int depth = 8, int samples = 1,
		int maxVertexCount = 100000, int available_cores = 0, XString method = "gridded", bool smooth_dsm = true, int max_tiles = 0);

	static XString dem_to_points(XString inGeotiff, XString outPointCloud);
	static XString dem_to_mesh_gridded(XString inGeotiff, XString outMesh, int maxVertexCount, int maxConcurrency = 1);
};

