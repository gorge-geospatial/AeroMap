// Mesh.cpp
// Port of odm mesh.py.
//

#include "AeroLib.h"
#include "DEM.h"
#include "Mesh.h"

XString Mesh::screened_poisson_reconstruction(XString inPointCloud, XString outMesh, int depth, double samplesPerNode, int maxVertexCount, double pointWeight, int threads)
{
    // inPointCloud -> PoissonRecon -> outMeshDirty -> ReconMesh -> outMesh
    //

    XString mesh_path = outMesh.GetPathName();
    XString mesh_filename = outMesh.GetFileName();
    // mesh_path = path/to
    // mesh_filename = odm_mesh.ply

    XString basename;
    XString ext;
    int pos = mesh_filename.ReverseFind('.');
    if (pos > -1)
    {
        basename = mesh_filename.Left(pos);
        ext = mesh_filename.Mid(pos);
        // basename = odm_mesh
        // ext = .ply
    }

    XString outMeshDirty = XString::CombinePath(mesh_path, XString::Format("%s.dirty%s", basename.c_str(), ext.c_str()));
    if (QFile::exists(outMeshDirty.c_str()))
        QFile::remove(outMeshDirty.c_str());

    // Since PoissonRecon has some kind of a race condition on ppc64el, and this helps...
    //if platform.machine() == 'ppc64le':
    //    Logger::Write(__FUNCTION__, "ppc64le platform detected, forcing single-threaded operation for PoissonRecon")
    //    threads = 1

    while (true)
    {
        // Run PoissonRecon
        QStringList args;
        args.push_back("--in");
        args.push_back(inPointCloud.c_str());
        args.push_back("--out");
        args.push_back(outMeshDirty.c_str());
        args.push_back("--depth");              // maximum reconstruction depth
        args.push_back(XString::Format("%d", depth).c_str());
        args.push_back("--pointWeight");        // interpolation weight
        args.push_back(XString::Format("%0.1f", pointWeight).c_str());
        args.push_back("--samplesPerNode");     // minimum number of samples per node (default=1.5)
        args.push_back(XString::Format("%0.1f", samplesPerNode).c_str());
        args.push_back("--threads");
        args.push_back(XString::Format("%d", threads).c_str());
        args.push_back("--bType");              // boundary type (1=free, 2=Dirichlet, 3=Neumann)
        args.push_back("2");
        args.push_back("--linearFit");
        AeroLib::RunProgram(tree.prog_poisson, args);
        // cmd: PoissonRecon 
        //          --in "d:/test/odm_filterpoints/point_cloud.ply" 
        //          --out "d:/test/odm_meshing/odm_mesh.dirty.ply" 
        //          --depth 11 
        //          --pointWeight 4.0 
        //          --samplesPerNode 1.0 
        //          --threads 15 
        //          --bType 2 
        //          --linearFit 

        if (AeroLib::FileExists(outMeshDirty))
        {
            break;  // Done!
        }
        else
        {
            // PoissonRecon will sometimes fail due to race conditions
            // on certain machines, especially on Windows
            threads /= 2;

            if (threads < 1)
                break;
            else
                Logger::Write(__FUNCTION__, "PoissonRecon failed with %d threads, retrying with %d...", threads, threads / 2);
		}
	}

	// Cleanup and reduce vertex count if necessary
    int max_faces = maxVertexCount * 2;

    QStringList args;
    args.push_back("-i");
    args.push_back(outMeshDirty.c_str());
    args.push_back("-o");
    args.push_back(outMesh.c_str());
    args.push_back("--archive-type");
    args.push_back("3");
    args.push_back("--remove-spikes");          // flag controlling the removal of spike faces
    args.push_back("0");
    args.push_back("--remove-spurious");        // spurious factor for removing faces with too long edges or isolated components
    args.push_back("20");
    args.push_back("--smooth");                 // number of iterations to smooth the reconstructed surface
    args.push_back("0");
    args.push_back("--target-face-num");
    args.push_back(XString::Format("%d", max_faces).c_str());
    args.push_back("-v");
    args.push_back("0");
    AeroLib::RunProgram(tree.prog_recon_mesh, args);
    // cmd: ReconstructMesh 
    //          -i "d:/test/odm_meshing/odm_mesh.dirty.ply" 
    //          -o "d:/test/odm_meshing/odm_mesh.ply" 
    //          --archive-type 3 --remove-spikes 0 --remove-spurious 20 --smooth 0 --target-face-num 400000 -v 0

    // Delete intermediate results
    // os.remove(outMeshDirty)

    return outMesh;
}

XString Mesh::create_25dmesh(XString inPointCloud, XString outMesh,
    std::vector<double> radius_steps, double dsm_resolution, int depth, int samples,
    int maxVertexCount, int available_cores, XString method, bool smooth_dsm, int max_tiles)
{
    // Create DSM from point cloud
    //

    // Create temporary directory
    XString mesh_directory = outMesh.GetPathName();
    XString tmp_directory = XString::CombinePath(mesh_directory, "tmp");
    //if os.path.exists(tmp_directory):
    //   shutil.rmtree(tmp_directory)
    AeroLib::CreateFolder(tmp_directory);

    Logger::Write(__FUNCTION__, "Creating DSM for 2.5D mesh");

    DEM::create_dem(inPointCloud, "mesh_dsm", radius_steps, "max", true, tmp_directory, dsm_resolution, available_cores, 4096, 0, false, smooth_dsm, max_tiles);

    XString mesh;
    if (method == "gridded")
    {
        mesh = dem_to_mesh_gridded(XString::CombinePath(tmp_directory, "mesh_dsm.tif"), outMesh, maxVertexCount, std::max(1, available_cores));
    }
    else if (method == "poisson")
    {
        XString dsm_points = dem_to_points(XString::CombinePath(tmp_directory, "mesh_dsm.tif"), XString::CombinePath(tmp_directory, "dsm_points.ply"));
        mesh = screened_poisson_reconstruction(dsm_points, outMesh, depth = depth,
            samples = samples,
            maxVertexCount = maxVertexCount,
            std::max(1, available_cores - 1));  //poissonrecon can get stuck on some machines if --threads == all cores
    }
    else
    {
        //        raise 'Not a valid method: ' + method
        Logger::Write(__FUNCTION__, "Invalid method: '%s'", method.c_str());
        assert(false);
    }

    // Cleanup tmp
    // if os.path.exists(tmp_directory):
    //     shutil.rmtree(tmp_directory)

    return mesh;
}

XString Mesh::dem_to_points(XString inGeotiff, XString outPointCloud)
{
    // inGeotiff -> dem2points.exe -> outPointCloud

    QStringList args;
    args.push_back("-inputFile");
    args.push_back(inGeotiff.c_str());
    args.push_back("-outputFile");
    args.push_back(outPointCloud.c_str());
    args.push_back("-skirtHeightThreshold");
    args.push_back("1.5");
    args.push_back("-skirtIncrements");
    args.push_back("0.2");
    args.push_back("-skirtHeightCap");
    args.push_back("100");
    args.push_back("-verbose");
    AeroLib::RunProgram(tree.prog_dem2points, args);
    // system.run('"{bin}" 
    //          -inputFile "{infile}" '         input DSM raster
    //         '-outputFile "{outfile}"         output PLY points
    //         '-skirtHeightThreshold 1.5 '     Height threshold between cells that triggers the creation of a skirt
    //         '-skirtIncrements 0.2 '          Skirt height increments when adding a new skirt
    //         '-skirtHeightCap 100 '           Height cap that blocks the creation of a skirt
    //         '-verbose '.format(**kwargs))

    return outPointCloud;
}

XString Mesh::dem_to_mesh_gridded(XString inGeotiff, XString outMesh, int maxVertexCount, int maxConcurrency)
{
    // inGeoTiff -> dem2mesh.exe -> outMeshDirty -> reconMesh -> outMesh
    // 

    Logger::Write(__FUNCTION__, "Creating mesh from DSM: '%s'", inGeotiff.c_str());

    XString mesh_path = outMesh.GetPathName();
    XString mesh_filename = outMesh.GetFileName();
    // mesh_path = path/to
    // mesh_filename = odm_mesh.ply

    XString basename;
    XString ext;
    int pos = mesh_filename.ReverseFind('.');
    if (pos > -1)
    {
        basename = mesh_filename.Left(pos);
        ext = mesh_filename.Mid(pos);
        // basename = odm_mesh
        // ext = .ply
    }

    XString outMeshDirty = XString::CombinePath(mesh_path, XString::Format("%s.dirty%s", basename.c_str(), ext.c_str()));

    // This should work without issues most of the time,
    // but just in case we lower maxConcurrency if it fails.
    while (true)
    {
        QStringList args;
        args.push_back("-inputFile");
        args.push_back(inGeotiff.c_str());
        args.push_back("-outputFile");
        args.push_back(outMeshDirty.c_str());
        args.push_back("-maxTileLength");
        args.push_back("2000");
        args.push_back("-maxVertexCount");
        args.push_back(XString::Format("%d", maxVertexCount).c_str());
        args.push_back("-maxConcurrency");
        args.push_back(XString::Format("%d", maxConcurrency).c_str());
        args.push_back("-edgeSwapThreshold");
        args.push_back("0.15");
        args.push_back("-verbose");
        AeroLib::RunProgram(tree.prog_dem2mesh, args);
        // system.run('"{bin}" -inputFile "{infile}" '
        //        '-outputFile "{outfile}" '
        //        '-maxTileLength 2000 '
        //        '-maxVertexCount {maxVertexCount} '
        //        '-maxConcurrency {maxConcurrency} '
        //        '-edgeSwapThreshold 0.15 '
        //        '-verbose '.format(**kwargs))
        if (AeroLib::FileExists(outMeshDirty))
        {
            break;      // done!
        }
        else
        {
            maxConcurrency = floor(maxConcurrency / 2);
            if (maxConcurrency >= 1)
            {
                Logger::Write(__FUNCTION__, "dem2mesh failed, retrying with lower concurrency (%d) in case this is a memory issue", maxConcurrency);
            }
            else
            {
                assert(false);
            }
        }
    }

    // Cleanup and reduce vertex count if necessary
    // (as dem2mesh cannot guarantee that we'll have the target vertex count)

    int max_faces = maxVertexCount * 2;

    QStringList args;
    args.push_back("-i");
    args.push_back(outMeshDirty.c_str());
    args.push_back("-o");
    args.push_back(outMesh.c_str());
    args.push_back("--archive-type");
    args.push_back("3");
    args.push_back("--remove-spikes");          // flag controlling the removal of spike faces
    args.push_back("0");
    args.push_back("--remove-spurious");        // spurious factor for removing faces with too long edges or isolated components
    args.push_back("0");
    args.push_back("--smooth");                 // number of iterations to smooth the reconstructed surface
    args.push_back("0");
    args.push_back("--target-face-num");
    args.push_back(XString::Format("%d", max_faces).c_str());
    args.push_back("-v");
    args.push_back("0");
    AeroLib::RunProgram(tree.prog_recon_mesh, args);
    //system.run('"{reconstructmesh}" -i "{infile}" '
    //     '-o "{outfile}" '
    //     '--archive-type 3 '
    //     '--remove-spikes 0 --remove-spurious 0 --smooth 0 '
    //     '--target-face-num {max_faces} -v 0'.format(**cleanupArgs))

    // Delete intermediate results
    //os.remove(outMeshDirty)

    return outMesh;
}
