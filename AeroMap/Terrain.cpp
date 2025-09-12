// Terrain.cpp
// Terrain model base class (see TerrainGL for rendering support)
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include <stdio.h>

#include <direct.h>  
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include <stack>
#include <algorithm>

#include "gdal_priv.h"

#include "ScaleColor.h"
#include "RasterFile.h"			// gdal raster file
#include "Logger.h"				// message logging facility
#include "Calc.h"
#include "Terrain.h"

#define open _open
#define lseek _lseek
#define read _read
#define write _write
#define close _close
#define chmod _chmod

Terrain::Terrain(const char* file_name)
	: ms_TerrainFile(file_name)
	, ms_TextureFile("")
	, m_File(0)
	, mb_IsDirty(false)
{
	memset(&m_Header, 0, sizeof(m_Header));

	LoadData();
}

Terrain::~Terrain()
{
	CloseDataFile();

	for (auto i : m_trn)
	{
		delete i;
	}
	m_trn.clear();
}

/* static */ bool Terrain::Create(const char* fileName, int rowCount, int colCount, double pitchMeters, double noData)
{
    // Create terrain file
    // 
    // Inputs:
    //		fileName	= fully qualified path/file name
    //      rowCount	= # of data rows
    //      colCount	= # of data columns
    //      pitchMeters	= horizontal and vertical point spacing, meters
    //

	int fileHandle = _open(fileName, O_CREAT | O_BINARY | O_TRUNC | O_WRONLY, S_IWRITE | S_IREAD);
	if (fileHandle < 1)
	{
		Logger::Write(__FUNCTION__, "Unable to create file '%s' (%s)", fileName, strerror(errno));
		return false;
	}

	// create header
	TerrainHeaderDiskType hdr = { 0 };
	memset(&hdr, 0, sizeof(hdr));
	hdr.Marker[0] = 'A';
	hdr.Marker[1] = 'M';
	hdr.VxSize = sizeof(TerrainVertexDiskType);		// size of vertex record, bytes
	hdr.Version = 1;
	hdr.MinHeight = 0.0;
	hdr.MaxHeight = 0.0;
	hdr.NoData = noData;
	hdr.RowCount = rowCount;
	hdr.ColCount = colCount;
	hdr.GridSpace = pitchMeters;

	if (write(fileHandle, &hdr, sizeof(hdr)) != sizeof(hdr))
	{
		Logger::Write(__FUNCTION__, "Error: write() failed on header (%s)", strerror(errno));
		close(fileHandle);
		fileHandle = 0;
		return false;
	}

	// allocate space
	for (int row = 0; row <rowCount; ++row)
	{
		for (int col = 0; col < colCount; ++col)
		{
			TerrainVertexDiskType vx = { 0 };
			write(fileHandle, &vx, sizeof(vx));
		}
	}

	close(fileHandle);

    return true;
}

double Terrain::GetHeight(int row, int col)
{
	// Return terrain elevation at row/col, meters msl.
	//

	double height = 0.0;

	if (IsValid(row, col))
	{
		if (IsNoData(row, col))
			height = GetMinElev();
		else
			height = m_trn[row][col].Height;
	}

	return height;
}

double Terrain::GetHeightNormal(double x, double y, VEC3* pNormal)
{
	// Return the terrain altitude and face normal at a specified x/y position.
	//
	// To get the face, we assume the NW/SE layout.
	//
	// Inputs:
	//		(x,y) = location for which to return terrain height, map units
	//		
	// Outputs:
	//		pNormal = normal vector at (x,y), if non-null
	//      return = altitude at (x,y)
	//

	VEC3 A(0, 0, 0);
	VEC3 B(0, 0, 0);
	VEC3 C(0, 0, 0);

	int dbRow, dbCol;
	double vxX, vxY;

	XYToRowCol(x, y, dbRow, dbCol);			// nearest (ul) row/col
	RowColToXY(dbRow, dbCol, vxX, vxY);	    // x/y of that vertex

	// determine face 1 / 2 in quad

	double dx = (x - vxX) / GetPitch();		// how far from UL corner are we into
	double dy = 1.0 + (vxY - y) / GetPitch();		// current quad?

	// if x offset + y offset < 1
	if (dx + dy < 1.0)		// nw face
	{
		//   B - C
		//   | /
		//   A
		double vxHeightA = GetHeight(dbRow, dbCol);
		double vxHeightB = GetHeight(dbRow + 1, dbCol);
		double vxHeightC = GetHeight(dbRow + 1, dbCol + 1);

		// don't need actual xy coordinates, just a triangle
		// with correct corner spacing & heights
		A = VEC3( vxX, vxY, vxHeightA );
		B = VEC3( vxX, vxY + GetPitch(), vxHeightB );
		C = VEC3( vxX + GetPitch(), vxY + GetPitch(), vxHeightC );
	}
	else	// se face
	{
		//     B
		//   / |
		//  A--C
		double vxHeightA = GetHeight(dbRow,     dbCol);
		double vxHeightB = GetHeight(dbRow + 1, dbCol + 1);
		double vxHeightC = GetHeight(dbRow,     dbCol + 1);

        A = VEC3( vxX, vxY, vxHeightA );
		B = VEC3( vxX + GetPitch(), vxY + GetPitch(), vxHeightB );
		C = VEC3( vxX + GetPitch(), vxY, vxHeightC );
	}

	// calculate the height of a point in a triangle (seems to work ok on arbitrary plane
	// defined by 3 vx)
	//
	// where, x, y	= test point
	//		  A,B,C	= corners of triangle

	VEC3 P_Line, V_Line;		// point & vx defining line that passes thru face

	VEC3 N = CrossProduct(B - A, C - A);		// get the normal
	N = Normalize(N);

	// define the line passing thru the plane as a point above the plane & a vertical
	// line extending straight down thru the plane

	P_Line = VEC3(x, y, m_Header.MaxHeight + 100.0);		// base of ray
	V_Line = VEC3(0, 0, m_Header.MinHeight - 100.0);		// direction/magnitude of ray

	// calc the intersection of the line with the plane as a ratio of the length of the
	// distance from P_Line to the plane over P_Line + V_Line
	double t = DotProduct(N, (C - P_Line)) / DotProduct(N, V_Line);

	VEC3 result = P_Line + (V_Line * t);

	if (pNormal)
		*pNormal = N;

	return result.z;
}

void Terrain::SetHeight(int row, int col, double heightMeters)
{
	// Set the altitude of the terrain vertex at the specified row/col.
	//

	if (IsValid(row, col) == false)
		return;

	m_trn[row][col].Height = heightMeters;

	OnDataChanged();
}

VEC3 Terrain::GetTerrainVertex(int row, int col)
{
	// Return the 3d vertex corresponding to (row,col)
	//

	VEC3 V;
	V.x = row * m_Header.GridSpace;
	V.y = col * m_Header.GridSpace;
	V.z = GetHeight(row, col);
	return V;
}

void Terrain::SetFlags(int row, int col, UInt8 flags)
{
	// Set the terrain vertex flags.
	//
	// Inputs:
	//		(row,col)	= location of vertex
	//		flags		= new flag mask
	//

	if (IsValid(row, col) == false)
		return;

	m_trn[row][col].Flags = flags;

	OnDataChanged();
}

UInt8 Terrain::GetFlags(int row, int col)
{
	// Get the terrain vertex flags.
	//
	// Inputs:
	//		(row,col)	= location of vertex
	// Outputs:
	//		return		= flag mask for vertex at (row,col)
	//

	if (IsValid(row, col) == false)
		return 0x00;

	return GetVertex(row, col).Flags;
}

void Terrain::UpdateHeightRange()
{
	// Ideally you could maintain these dynamically, but changing a height value
	// can only extend the current min/max range. if you've changed the lowest point,
	// you would not know it and the in-line min/max test would fail.

	// That's why this routine exists, to force the min/max to match the actual
	// terrain values. it's too slow to use in realtime, but is useful for cases
	// like just after creation or at the end of an editing operation.

	bool landInit = false;

	for (int row = 0; row < GetRowCount(); ++row)
	{
		for (int col = 0; col < GetColCount(); ++col)
		{
			TerrainVertexDiskType vx = m_trn[row][col];
			if (vx.Height != m_Header.NoData)
			{
				if (landInit == false)
				{
					m_Header.MinHeight = vx.Height;
					m_Header.MaxHeight = vx.Height;
					landInit = true;
				}
				else
				{
					if (m_Header.MinHeight > vx.Height)
						m_Header.MinHeight = vx.Height;
					if (m_Header.MaxHeight < vx.Height)
						m_Header.MaxHeight = vx.Height;
				}
			}
		}
	}
}

bool Terrain::OpenDataFile()
{
	// Open terrain data file.
	//

	CloseDataFile();

	m_File = _open(ms_TerrainFile.c_str(), O_RDWR | O_BINARY | O_RANDOM);
	if (m_File < 1)
	{
		Logger::Write(__FUNCTION__, "Unable to open '%s' (%s)", ms_TerrainFile.c_str(), strerror(errno));
		return false;
	}

	return true;
}

void Terrain::CloseDataFile()
{
	if (m_File > 0)
	{
		int status = close(m_File);
		if (status != 0)
			Logger::Write(__FUNCTION__, "close() failed, errno = %d", errno);
		else
			m_File = 0;
	}
}

void Terrain::LoadData()
{
	// Load elevation data.

	if (OpenDataFile() == false)
		return;

	// read file header

	int expected = sizeof(TerrainHeaderDiskType);
	int bytes = read(m_File, &m_Header, expected);
	if (bytes != expected)
	{
		Logger::Write(__FUNCTION__, "Unable to read header: expected %d bytes, read %d", expected, bytes);
		return;
	}

	// load entire row at a time
	int rowSize = sizeof(TerrainVertexDiskType) * GetColCount();
	m_trn.resize(GetRowCount());
	for (int row = 0; row < GetRowCount(); ++row)
	{
		m_trn[row] = new TerrainVertexDiskType[GetColCount()];
		int bytes = read(m_File, m_trn[row], rowSize);
		if (bytes != rowSize)
			Logger::Write(__FUNCTION__, "Read row failed: expected %d bytes, read %d", rowSize, bytes);
	}

	CloseDataFile();

	UpdateHeightRange();

	mb_IsDirty = false;
}

void Terrain::SaveData()
{
	// Save elevation data.
	//

	if (OpenDataFile() == false)
		return;

	UpdateHeightRange();

	// write header
	lseek(m_File, 0, SEEK_SET);
	write(m_File, &m_Header, sizeof(m_Header));

	int rowSize = sizeof(TerrainVertexDiskType) * GetColCount();
	for (int row = 0; row < GetRowCount(); ++row)
	{
		int bytes = write(m_File, m_trn[row], rowSize);
		if (bytes != rowSize)
			Logger::Write(__FUNCTION__, "write() failed: expected %d bytes, wrote %d", rowSize, bytes);
	}

	CloseDataFile();

	mb_IsDirty = false;
}

void Terrain::Flush()
{
	// Ensure changes written to disk.

	if (m_File > 0)
	{
		// windows specific
		int status = _commit(m_File);
		if (status != 0)
		{
			Logger::Write(__FUNCTION__, "_commit() failed, errno = %d", errno);
		}
	}
}

bool Terrain::IsValid(int row, int col)
{
	// Check (row,col) for within terrain.

	return (row >= 0 && row < GetRowCount() && col >= 0 && col < GetColCount());
}

/* inline */ UInt32 Terrain::CalcByteOffset(int rowIdx, int colIdx)
{
	// Calculate the record #.

	UInt32 recIdx = GetColCount()*rowIdx + colIdx;

	// convert to byte offset

	return recIdx * sizeof(TerrainVertexDiskType) + sizeof(TerrainHeaderDiskType);
}

bool Terrain::XYToRowCol(double x, double y, int& row, int& col)
{
	// Map world x,y to terrain row/col
	//
	// Inputs:
	//		(x,y) = world units (can be off map)
	// Outputs:
	//		(row,col) = terrain row/col corresponding to (x,y)
	//		return = true if (row,col) on map
	//

	row = (int)(y / GetPitch());
	col = (int)(x / GetPitch());

	return (IsValid(row, col));
}

void Terrain::RowColToXY(int row, int col, double& x, double& y)
{
	// Map terrain row/col to world x/y.
	//
	// Note: this is simply offset from terrain origin.
	//       For rowcol to projected xy, use RowColToProj()
	//

	if (col < 0)
		x = 0.0;
	else if (col > GetColCount() - 1)
		x = GetSizeX();
	else
		x = col * GetPitch();

	if (row < 0)
		y = 0.0;
	else if (row > GetRowCount() - 1)
		y = row * GetPitch();
	else
		y = row * GetPitch();
}

bool Terrain::ProjToRowCol(double x, double y, int& row, int& col)
{
	// Map projected coordinates to terrain coordinates.
	//

	// only projected system currently supported
	assert(m_Header.Projection == static_cast<int>(GIS::Projection::UTM));

	double terrain_x = x - m_Header.SWX;
	double terrain_y = y - m_Header.SWY;

	return XYToRowCol(terrain_x, terrain_y, row, col);
}

void Terrain::RowColToProj(int row, int col, double& x, double& y)
{
	// Map row/col to projected coordinate system.
	//

	// only projected system currently supported
	assert(m_Header.Projection == static_cast<int>(GIS::Projection::UTM));

	double terrain_x;
	double terrain_y;
	RowColToXY(row, col, terrain_x, terrain_y);

	x = m_Header.SWX + terrain_x;
	y = m_Header.SWY + terrain_y;
}

bool Terrain::LLToRowCol(double lat, double lon, int& row, int& col)
{
	// Map lat/lon to terrain row/col
	//
	// Inputs:
	//		(lat,lon)	= world units (can be off map)
	// Outputs:
	//		(row,col)	= terrain row/col corresponding to (lat,lon)
	//		return		= true if (row,col) on map
	//

	switch (static_cast<GIS::Projection>(m_Header.Projection)) {
	case GIS::Projection::UTM:
	{
		// project to terrain utm

		double xm, ym;
		GIS::LatLonToXY_UTM(lat, lon, xm, ym);

		// map terrain utm xy to row/col

		ProjToRowCol(xm, ym, row, col);
		break;
	}
	case GIS::Projection::Geographic:
	{
		//TODO:
		assert(false);
		break;
	}
	default:
		Logger::Write(__FUNCTION__, "Unhandled terrain projection: %d", m_Header.Projection);
		assert(false);
		break;
	}

	return (IsValid(row, col));
}

void Terrain::RowColToLL(int row, int col, double& lat, double& lon)
{
	// Map terrain row/col to lat/lon.
	//

	switch (static_cast<GIS::Projection>(m_Header.Projection)) {
	case GIS::Projection::UTM:
		{
			double xm, ym;
			int zone = m_Header.ProjZone;
			RowColToProj(row, col, xm, ym);
			GIS::XYToLatLon_UTM(zone, ym < 0.0 ? GIS::Hemi::S : GIS::Hemi::N, xm, ym, lat, lon);
		}
		break;
	case GIS::Projection::Geographic:
		{
			// just interpolate into geo extents
			double dx = (double)col / (double)(GetColCount() - 1);
			double dy = (double)row / (double)(GetRowCount() - 1);
			lat = Lerp(dy, m_Header.SWY, m_Header.NEY);
			lon = Lerp(dx, m_Header.SWX, m_Header.NEX);
		}
		break;
	case GIS::Projection::None:
		lat = 0.0;
		lon = 0.0;
		break;
	default:
		Logger::Write(__FUNCTION__, "Unhandled terrain projection: %d", m_Header.Projection);
		assert(false);
		break;
	}
}

int Terrain::AddDistancePoint(VEC2& pos)
{
	mv_Distance.push_back(pos);
	return static_cast<int>(mv_Distance.size());
}

int Terrain::GetDistancePointCount() const
{
	return static_cast<int>(mv_Distance.size());
}

const VEC2 Terrain::GetDistancePoint(unsigned int index)
{
	if (index < mv_Distance.size())
		return mv_Distance[index];

	return VEC2();
}

void Terrain::ClearDistancePoints()
{
	return mv_Distance.clear();
}

double Terrain::GetDistance(unsigned int index)
{
	// return distance from mv_Distance[index-1] to mv_Distance[index]

	if (index > 0 && index < mv_Distance.size())
		return (mv_Distance[index] - mv_Distance[index - 1]).Magnitude();

	return 0.0;
}

// Derivation from the fortran version of CONREC by Paul Bourke
//
// The pitch gives horizontal spacing, but xy offsets of data
// assumed to be 0. If we need to start somewhere else, add
// xoffset/yoffset parameters.
//
// Note:
// There may be a bug or (literally) edge case, where the outer triangles
// aren't processed. Saw fix on one web site that just added data on end.
// Think i can do something cleaner.
//
// Inputs:
//		z = vector of elevation for which contours are to be created, low to high
//		pixColor = solid color to use for all contour lines
//		strScaleColor = color scale to use (exclusive of pixColor)
//

void Terrain::Contour(std::vector<double> z, PixelType pixColor, XString strScaleColor)
{
#define xsect(p1, p2) { (h[p2]*xh[p1]-h[p1]*xh[p2])/(h[p2]-h[p1]) }
#define ysect(p1, p2) { (h[p2]*yh[p1]-h[p1]*yh[p2])/(h[p2]-h[p1]) }

	int nc = (int)z.size();
	if (nc == 0)
		return;

	// allocate a list for each contour level
	mv_Contour.clear();
	mv_Contour.resize(nc);
	mv_ContourElev.resize(nc);
	for (int k = 0; k < nc; ++k)
	{
		mv_Contour[k].clear();
		mv_ContourElev[k] = z[k];
	}

	double pitch = GetPitch();

	int im[4] = { 0, 1, 1, 0 };
	int jm[4] = { 0, 0, 1, 1 };

	int case_table[3][3][3] =
	{
		{ { 0, 0, 8 },{ 0, 2, 5 },{ 7, 6, 9 } },
		{ { 0, 3, 4 },{ 1, 3, 1 },{ 4, 3, 0 } },
		{ { 9, 6, 7 },{ 5, 2, 0 },{ 8, 0, 0 } }
	};

	int colCount = GetColCount();
	int rowCount = GetRowCount();
	for (int x = (colCount - 2); x >= 0; --x)
	{
		for (int y = 0; y < (rowCount - 1); ++y)
		{
			// get min/max elevation in adjacent quad
			double vx1 = GetHeight(y, x);
			double vx2 = GetHeight(y, x + 1);
			double vx3 = GetHeight(y + 1, x + 1);
			double vx4 = GetHeight(y + 1, x);
			double temp1 = std::min(vx1, vx2);
			double temp2 = std::min(vx4, vx3);
			double dmin = std::min(temp1, temp2);
			temp1 = std::max(vx1, vx2);
			temp2 = std::max(vx4, vx3);
			double dmax = std::max(temp1, temp2);

			// if it does not intersect any contour level, skip
			if (dmax < z[0] || dmin > z[nc - 1])
				continue;

			// for each contour
			for (int k = 0; k < nc; ++k)
			{
				// if it does not intersect current contour level, skip
				if (z[k] < dmin || z[k] > dmax)
					continue;

				// calc values for current quad, 
				// 0 = center vertex & 1-4 are corners
				double h[5];		// heights for the quad
				double xh[5];		// x coordinates for quad
				double yh[5];		// y coordiantes for quad
				int sh[5];

				for (int m = 4; m >= 0; --m)
				{
					if (m > 0)
					{
						int xi = x + jm[m - 1];		// x index
						int yi = y + im[m - 1];		// y index
						h[m] = GetHeight(yi, xi) - z[k];
						//h[m] = data[xi][yi] - z[k];
						xh[m] = (double)xi*pitch;
						yh[m] = (double)yi*pitch;
					}
					else
					{
						// calc height & xy coordinates of quad midpoint
						h[0] = 0.25 * (h[1] + h[2] + h[3] + h[4]);
						xh[0] = (double)(x + 0.5)*pitch;
						yh[0] = (double)(y + 0.5)*pitch;
					}
					if (h[m] > 0.0)
						sh[m] = 1;
					else if (h[m] < 0.0)
						sh[m] = -1;
					else
						sh[m] = 0;
				}

				/*
				Note: at this stage the relative heights of the corners and the
				centre are in the h array, and the corresponding coordinates are
				in the xh and yh arrays. The centre of the box is indexed by 0
				and the 4 corners by 1 to 4 as shown below.

				Each triangle is then indexed by the parameter m, and the 3
				vertices of each triangle are indexed by parameters m1,m2,and m3.
				It is assumed that the centre of the box is always vertex 2
				(in the triangle) though this is important only when all 3
				vertices lie exactly on the same contour level, in which case
				only the side of the box is drawn.

				vertex 4 +-------------------+ vertex 3
						 | \               / |
						 |   \    m=3    /   |
						 |     \       /     |
						 |       \   /       |
						 |  m=4    X   m=2   |       the centre is vertex 0
						 |       /   \       |
						 |     /       \     |
						 |   /    m=1    \   |
						 | /               \ |
				vertex 1 +-------------------+ vertex 2
				*/

				// Scan each triangle in the box
				for (int m = 1; m <= 4; ++m)
				{
					int m1 = m;
					int m2 = 0;
					int m3;

					if (m != 4)
						m3 = m + 1;
					else
						m3 = 1;

					int case_value = case_table[sh[m1] + 1][sh[m2] + 1][sh[m3] + 1];
					if (case_value == 0)
						continue;

					double x0 = 0.0;		// output line
					double y0 = 0.0;
					double x1 = 0.0;
					double y1 = 0.0;

					switch (case_value) {
					case 1:						// Line between vertices 1 and 2
						x0 = xh[m1];
						y0 = yh[m1];
						x1 = xh[m2];
						y1 = yh[m2];
						break;
					case 2:						// Line between vertices 2 and 3
						x0 = xh[m2];
						y0 = yh[m2];
						x1 = xh[m3];
						y1 = yh[m3];
						break;
					case 3:						// Line between vertices 3 and 1
						x0 = xh[m3];
						y0 = yh[m3];
						x1 = xh[m1];
						y1 = yh[m1];
						break;
					case 4:						// Line between vertex 1 and side 2-3
						x0 = xh[m1];
						y0 = yh[m1];
						x1 = xsect(m2, m3);
						y1 = ysect(m2, m3);
						break;
					case 5:						// Line between vertex 2 and side 3-1
						x0 = xh[m2];
						y0 = yh[m2];
						x1 = xsect(m3, m1);
						y1 = ysect(m3, m1);
						break;
					case 6:						// Line between vertex 3 and side 1-2
						x0 = xh[m3];
						y0 = yh[m3];
						x1 = xsect(m1, m2);
						y1 = ysect(m1, m2);
						break;
					case 7:						// Line between sides 1-2 and 2-3
						x0 = xsect(m1, m2);
						y0 = ysect(m1, m2);
						x1 = xsect(m2, m3);
						y1 = ysect(m2, m3);
						break;
					case 8:						// Line between sides 2-3 and 3-1
						x0 = xsect(m2, m3);
						y0 = ysect(m2, m3);
						x1 = xsect(m3, m1);
						y1 = ysect(m3, m1);
						break;
					case 9:						// Line between sides 3-1 and 1-2
						x0 = xsect(m3, m1);
						y0 = ysect(m3, m1);
						x1 = xsect(m1, m2);
						y1 = ysect(m1, m2);
						break;
					default:
						break;
					}

					// Store the line segments for this elevation level
					RectD rect;
					rect.x0 = x0; rect.y0 = y0; rect.x1 = x1; rect.y1 = y1;
					mv_Contour[k].push_back(rect);
				}	// m
			}	// k - contour
		}	// y
	}	// x

	SortContours();

	// set contour colors
	ScaleColor* pScale = nullptr;
	if (strScaleColor.IsEmpty() == false)
		pScale = new ScaleColor(strScaleColor.c_str());
	for (unsigned int i = 0; i < mv_ContourLines.size(); ++i)
	{
		if (pScale == nullptr)
		{
			mv_ContourLines[i].color = pixColor;
		}
		else
		{
			double elev = (mv_ContourLines[i].elev - GetMinElev()) / (GetMaxElev() - GetMinElev());
			mv_ContourLines[i].color = pScale->GetColorByHeight(elev);
		}
	}
	delete pScale;
}

void Terrain::SortContours()
{
	// Convert unordered lists of line segments to ordered list of points.
	//
	// Inputs:
	//		mv_ContourElev
	//		mv_Contour
	// Outputs:
	//		mv_ContourLines
	//

	const double DELTA = 0.001;		// how close points have to be to be considered equal

	mv_ContourLines.clear();

	// make working copies of contour structures
	std::vector<double> contourElev;				// contour elevation levels, 0 = lowest
	std::vector<std::vector<RectD>> contour;		// all contour line segments, each "rect" is single line segment, index is level
	contourElev = mv_ContourElev;
	contour = mv_Contour;

	// for each contour level
	for (int k = 0; k < contourElev.size(); ++k)
	{
		// until all points have been added to a contour line
		while (contour[k].size() > 0)
		{
			// current list of points
			std::vector<PointD>ptList;

			// extract a line segment from mv_Contour
			// (don't remove 0th, need to find from other end of it)
			RectD cur_seg = contour[k][0];
			PointD cur_pt(cur_seg.x1, cur_seg.y1);
			ptList.push_back(PointD(cur_seg.x0, cur_seg.y0));
			contour[k].erase(contour[k].begin());

			// while not "at end", connect contour segments
			bool at_end = false;
			int scan_count = 0;
			while (at_end == false)
			{
				// starting at x1/y1, find connected segment - think this will be slow since must scan all segments for match (with small delta)
				bool match = false;
				for (int i = 0; i < contour[k].size(); ++i)
				{
					PointD pt0(contour[k][i].x0, contour[k][i].y0);
					PointD pt1(contour[k][i].x1, contour[k][i].y1);

					if ((fabs(cur_pt.x - pt0.x) < DELTA) && (fabs(cur_pt.y - pt0.y) < DELTA))
					{
						// this is a match on (x0/y0)
						match = true;

						// add it to list & start searching from (x1,y1)
						if (scan_count == 0)
							ptList.push_back(pt0);
						else
							ptList.insert(ptList.begin(), pt0);
						cur_pt = pt1;

						// remove this line segment so we don't find it again
						contour[k].erase(contour[k].begin() + i);
					}
					else if ((fabs(cur_pt.x - pt1.x) < DELTA) && (fabs(cur_pt.y - pt1.y) < DELTA))
					{
						// this is a match on (x1/y1)
						match = true;

						// add it to list & start searching from (x0,y0)
						if (scan_count == 0)
							ptList.push_back(pt1);
						else
							ptList.insert(ptList.begin(), pt1);
						cur_pt = pt0;

						// remove this line segment so we don't find it again
						contour[k].erase(contour[k].begin() + i);
					}
				}
				if (match == false)
				{
					// did not find matching line segment, at end
					if (++scan_count < 2)
					{
						// start again at init point & scan other direction
						if (ptList.size() == 0)
							at_end = true;
						else
							cur_pt = ptList[0];
					}
					else
					{
						at_end = true;
					}
				}
			}

			// now should have complete contour line
			if (ptList.size() > 1)
			{
				ContourLineType line;
				line.closed = false;
				// if last point connects to first
				// i'm assuming if it ends within less than a single point spacing, it must be closed
				// not sure that's 100% accurate, but seems to work so far
				PointD ptFirst = ptList[0];
				PointD ptLast = ptList[ptList.size()-1];
				if ((fabs(ptFirst.x - ptLast.x) < GetPitch() * 0.5) && (fabs(ptFirst.y - ptLast.y) < GetPitch() * 0.5))
				{
					// perhaps also verify we're not right on an edge
					line.closed = true;
				}
				line.elev = contourElev[k];
				line.pts = ptList;
				mv_ContourLines.push_back(line);		// add to list of actual contour lines
			}
		}
	}
}

XString Terrain::GetMetaData()
{
	XString strMeta;	// return value
	XString str;

	str = XString::Format("Terrain File: %s\n", ms_TerrainFile.c_str()); strMeta += str;
	str = XString::Format("Rows: %d\n", GetRowCount()); strMeta += str;
	str = XString::Format("Columns: %d\n", GetColCount()); strMeta += str;
	str = XString::Format("Min Elevation: %0.2f\n", GetMinElev()); strMeta += str;
	str = XString::Format("Max Elevation: %0.2f\n", GetMaxElev()); strMeta += str;
	str = XString::Format("Grid Spacing: %0.2f\n", GetPitch()); strMeta += str;
	if (static_cast<GIS::Projection>(m_Header.Projection) == GIS::Projection::UTM)
		str = XString::Format("Projection: %s (UTM Zone: %d)\n", GIS::ProjectionStr(static_cast<GIS::Projection>(m_Header.Projection)), m_Header.ProjZone);
	else
		str = XString::Format("Projection: %s\n", GIS::ProjectionStr(static_cast<GIS::Projection>(m_Header.Projection)));
	strMeta += str;
	str = XString::Format("SW Coordinates: %0.1f, %0.1f\n", m_Header.SWX, m_Header.SWY); strMeta += str;

	return strMeta;
}

void Terrain::SetProjection(GIS::Projection proj, int zone)
{
	// Set projection.
	// 
	// Inputs:
	//		proj = GIS::Projection value
	//		zome = optional zone, if applicable for projection
	//

	m_Header.Projection = static_cast<int>(proj);
	m_Header.ProjZone = zone;
}

GIS::Projection Terrain::GetProjection(int* zone)
{
	if (zone)
		*zone = m_Header.ProjZone;

	return static_cast<GIS::Projection>(m_Header.Projection);
}

void Terrain::SetGeoExtents(double sw_x, double sw_y, double ne_x, double ne_y)
{
	// Set the geospatial extents of the model.
	//
	// These are the lat/lon or projected x/y coordinates of the
	// model extents.
	//
	// Inputs:
	//		(sw_x,sw_y) = sw corner of model
	//		(ne_x,ne_y) = ne corner
	//
	//		could be projected or lat/lon, depends
	//		Projection value
	//

	m_Header.SWX = sw_x;
	m_Header.SWY = sw_y;
	m_Header.NEX = ne_x;
	m_Header.NEY = ne_y;
}

RectD Terrain::GetGeoExents()
{
	RectD rect;
	rect.x0 = m_Header.SWX;
	rect.y0 = m_Header.SWY;
	rect.x1 = m_Header.NEX;
	rect.y1 = m_Header.NEY;
	return rect;
}

PointD Terrain::GetGeoCoordSW()
{
	return PointD(m_Header.SWX, m_Header.SWY);
}

bool Terrain::IsNoData(int row, int col)
{
	double height = m_trn[row][col].Height;
	return (height == m_Header.NoData);
}

//------------------------------------------------------------------------------------------------------------------------
void Terrain::OnDataChanged()
{
	// Called every time data changes - keep it fast, can be called for every point 
	// in a large terrain database (set dirty flag, etc - not IO).
	//

	mb_IsDirty = true;
}

//------------------------------------------------------------------------------------------------------------------------
// protected members

Terrain::TerrainVertexDiskType Terrain::GetVertex(int row, int col)
{
	// Return terrain elevation at row/col, feet msl.
	//

	TerrainVertexDiskType vx = { 0 };

	if (IsValid(row, col))
		vx = m_trn[row][col];

	return vx;
}
