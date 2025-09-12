// LasFile.cpp
// Manager for compressed and uncompressed (.laz and .las)
// lidar files.
//
// Depends on LasTools.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <assert.h>

#include "Logger.h"			// message logging facility
#include "LasFile.h"

LasFile::LasFile(const char* fileName)
	: mi_DataOffset(0)
	, mi_VarRecCount(0)
	, mi_PointCount(0)
	, mi_PointFormat(0)
	, mi_FileCreateDay(0)
	, mi_FileCreateYear(0)
	, mi_FileSourceID(0)
	, mf_MaxX(0.0)
	, mf_MinX(0.0)
	, mf_MaxY(0.0)
	, mf_MinY(0.0)
	, mf_MaxZ(0.0)
	, mf_MinZ(0.0)
	, mf_ScaleX(0.0)
	, mf_ScaleY(0.0)
	, mf_ScaleZ(0.0)
	, mf_OffsetX(0.0)
	, mf_OffsetY(0.0)
	, mf_OffsetZ(0.0)
	, mi_FileHandle(0)
	, mp_LasReader(nullptr)
{
	memset(ms_FileName, 0, sizeof(ms_FileName));
	memset(&m_Header10, 0, sizeof(m_Header10));

	Load(fileName);		// load file
}

LasFile::~LasFile()
{
	//mp_LasReader->close();
	delete mp_LasReader;

	CloseFile();
}

bool LasFile::Load(const char* fileName)
{
	XString strFileName = fileName;
	if (strFileName.EndsWithNoCase(".LAS"))
		return LoadLasFile(fileName);
	else if (strFileName.EndsWithNoCase(".LAZ"))
		return LoadLazFile(fileName);

	assert(false);
	return false;
}

bool LasFile::LoadLazFile(const char* fileName)
{
	// load laz file
	//
	// for compressed files, we just instantiate a lasReader
	// and use that to service read requests
	//
	// we could use lasreader for everything, but i'm limiting
	// my used of the laszip/laslib software to access of 
	// isenburg's compression format.
	//

	if (mp_LasReader)
	{
		mp_LasReader->close();
		delete mp_LasReader;
	}

	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(fileName);

	mp_LasReader = lasreadopener.open();
	if (mp_LasReader == nullptr)
	{
		Logger::Write(__FUNCTION__, "Unable to open file: '%s'", fileName);
		return false;
	}

	strncpy(ms_FileName, fileName, _countof(ms_FileName) - 1);

	return true;
}

bool LasFile::LoadLasFile(const char* fileName)
{
	// load a las file
	//

	mi_GlobalEncoding = 0;
	mi_FileSourceID = 0;
	mi_FileCreateDay = 0;
	mi_FileCreateYear = 0;
	mi_DataOffset = 0;
	mi_VarRecCount = 0;
	mi_PointCount = 0;
	mi_PointFormat = 0;
	mf_MaxX = mf_MinX = 0.0;
	mf_MaxY = mf_MinY = 0.0;
	mf_MaxZ = mf_MinZ = 0.0;
	mf_ScaleX = mf_ScaleY = mf_ScaleZ = 0.0;
	mf_OffsetX = mf_OffsetY = mf_OffsetZ = 0.0;

	CloseFile();

	mi_FileHandle = _open(fileName, O_RDONLY | O_BINARY);
	if (mi_FileHandle == -1)
	{
		Logger::Write(__FUNCTION__, "Unable to open file '%s'.", fileName);
		return false;
	}

	// read header

	_lseek(mi_FileHandle, 0L, SEEK_SET);
	int byteCount = _read(mi_FileHandle, &m_Header10, sizeof(m_Header10));
	assert(byteCount == sizeof(m_Header10));

	if (memcmp(m_Header10.Signature, "LASF", 4) != 0)
	{
		Logger::Write(__FUNCTION__, "Invalid file signature: '%s'", m_Header10.Signature);
		CloseFile();
		return false;
	}

	// read version specific header

	if (m_Header10.VerMajor == 1 && m_Header10.VerMinor == 0)
	{
		if (m_Header10.HdrSize != byteCount)
		{
			Logger::Write(__FUNCTION__, "Invalid header size. Expected %d, encountered %d.", m_Header10.HdrSize, byteCount);
			CloseFile();
			return false;
		}

		mi_GlobalEncoding = 0;
		mi_FileSourceID = 0;
		mi_FileCreateDay = m_Header10.FlightDate;
		mi_FileCreateYear = m_Header10.Year;
		mi_PointFormat = m_Header10.PointFormat;
		mi_PointCount = m_Header10.PointDataRecCount;
		mi_VarRecCount = m_Header10.VarLenRecCount;
		mi_DataOffset = m_Header10.DataOffset;
		mf_MaxX = m_Header10.MaxX;
		mf_MinX = m_Header10.MinX;
		mf_MaxY = m_Header10.MaxY;
		mf_MinY = m_Header10.MinY;
		mf_MaxZ = m_Header10.MaxZ;
		mf_MinZ = m_Header10.MinZ;
		mf_ScaleX = m_Header10.XScale;
		mf_ScaleY = m_Header10.YScale;
		mf_ScaleZ = m_Header10.ZScale;
		mf_OffsetX = m_Header10.XOffset;
		mf_OffsetY = m_Header10.YOffset;
		mf_OffsetZ = m_Header10.ZOffset;
	}
	else if (m_Header10.VerMajor == 1 && m_Header10.VerMinor == 1)
	{
		_lseek(mi_FileHandle, 0L, SEEK_SET);
		int byteCount = _read(mi_FileHandle, &m_Header11, sizeof(m_Header11));
		assert(byteCount == sizeof(m_Header11));

		if (m_Header11.HdrSize != byteCount)
		{
			Logger::Write(__FUNCTION__, "Invalid header size. Expected %d, encountered %d.", m_Header11.HdrSize, byteCount);
			CloseFile();
			return false;
		}

		mi_GlobalEncoding = 0;
		mi_FileSourceID = m_Header11.FileSourceID;
		mi_FileCreateDay = m_Header11.FileCreateDay;
		mi_FileCreateYear = m_Header11.FileCreateYear;
		mi_PointFormat = m_Header11.PointFormat;
		mi_PointCount = m_Header11.PointDataRecCount;
		mi_VarRecCount = m_Header11.VarLenRecCount;
		mi_DataOffset = m_Header11.DataOffset;
		mf_MaxX = m_Header11.MaxX;
		mf_MinX = m_Header11.MinX;
		mf_MaxY = m_Header11.MaxY;
		mf_MinY = m_Header11.MinY;
		mf_MaxZ = m_Header11.MaxZ;
		mf_MinZ = m_Header11.MinZ;
		mf_ScaleX = m_Header11.XScale;
		mf_ScaleY = m_Header11.YScale;
		mf_ScaleZ = m_Header11.ZScale;
		mf_OffsetX = m_Header11.XOffset;
		mf_OffsetY = m_Header11.YOffset;
		mf_OffsetZ = m_Header11.ZOffset;

		//TODO:
				//doc says 1 mandatory var length record but m_Header11.VarLenRecCount == 0,
				//how do i handle that?
	}
	else if (m_Header10.VerMajor == 1 && m_Header10.VerMinor == 2)
	{
		_lseek(mi_FileHandle, 0L, SEEK_SET);
		int byteCount = _read(mi_FileHandle, &m_Header12, sizeof(m_Header12));
		assert(byteCount == sizeof(m_Header12));

		if (m_Header12.HdrSize != byteCount)
		{
			Logger::Write(__FUNCTION__, "Invalid header size. Expected %d, encountered %d.", m_Header12.HdrSize, byteCount);
			CloseFile();
			return false;
		}

		mi_GlobalEncoding = m_Header12.GlobalEncoding;
		mi_FileSourceID = m_Header12.FileSourceID;
		mi_FileCreateDay = m_Header12.FileCreateDay;
		mi_FileCreateYear = m_Header12.FileCreateYear;
		mi_PointFormat = m_Header12.PointFormat;
		mi_PointCount = m_Header12.PointDataRecCount;
		mi_VarRecCount = m_Header12.VarLenRecCount;
		mi_DataOffset = m_Header12.DataOffset;
		mf_MaxX = m_Header12.MaxX;
		mf_MinX = m_Header12.MinX;
		mf_MaxY = m_Header12.MaxY;
		mf_MinY = m_Header12.MinY;
		mf_MaxZ = m_Header12.MaxZ;
		mf_MinZ = m_Header12.MinZ;
		mf_ScaleX = m_Header12.XScale;
		mf_ScaleY = m_Header12.YScale;
		mf_ScaleZ = m_Header12.ZScale;
		mf_OffsetX = m_Header12.XOffset;
		mf_OffsetY = m_Header12.YOffset;
		mf_OffsetZ = m_Header12.ZOffset;
	}
	else if (m_Header10.VerMajor == 1 && m_Header10.VerMinor == 3)
	{
		_lseek(mi_FileHandle, 0L, SEEK_SET);
		int byteCount = _read(mi_FileHandle, &m_Header13, sizeof(m_Header13));
		assert(byteCount == sizeof(m_Header13));

		if (m_Header13.HdrSize != byteCount)
		{
			Logger::Write(__FUNCTION__, "Invalid header size. Expected %d, encountered %d.", m_Header13.HdrSize, byteCount);
			CloseFile();
			return false;
		}

		mi_GlobalEncoding = m_Header13.GlobalEncoding;
		mi_FileSourceID = m_Header13.FileSourceID;
		mi_FileCreateDay = m_Header13.FileCreateDay;
		mi_FileCreateYear = m_Header13.FileCreateYear;
		mi_PointFormat = m_Header13.PointFormat;
		mi_PointCount = m_Header13.PointDataRecCount;
		mi_VarRecCount = m_Header13.VarLenRecCount;
		mi_DataOffset = m_Header13.DataOffset;
		mf_MaxX = m_Header13.MaxX;
		mf_MinX = m_Header13.MinX;
		mf_MaxY = m_Header13.MaxY;
		mf_MinY = m_Header13.MinY;
		mf_MaxZ = m_Header13.MaxZ;
		mf_MinZ = m_Header13.MinZ;
		mf_ScaleX = m_Header13.XScale;
		mf_ScaleY = m_Header13.YScale;
		mf_ScaleZ = m_Header13.ZScale;
		mf_OffsetX = m_Header13.XOffset;
		mf_OffsetY = m_Header13.YOffset;
		mf_OffsetZ = m_Header13.ZOffset;
	}
	else if (m_Header10.VerMajor == 1 && m_Header10.VerMinor == 4)
	{
		_lseek(mi_FileHandle, 0L, SEEK_SET);
		int byteCount = _read(mi_FileHandle, &m_Header14, sizeof(m_Header14));
		assert(byteCount == sizeof(m_Header14));

		if (m_Header14.HdrSize != byteCount)
		{
			Logger::Write(__FUNCTION__, "Invalid header size. Expected %d, encountered %d.", m_Header14.HdrSize, byteCount);
			CloseFile();
			return false;
		}

		mi_GlobalEncoding = m_Header14.GlobalEncoding;
		mi_FileSourceID = m_Header14.FileSourceID;
		mi_FileCreateDay = m_Header14.FileCreateDay;
		mi_FileCreateYear = m_Header14.FileCreateYear;
		mi_PointFormat = m_Header14.PointFormat;
		mi_PointCount = m_Header14.PointDataRecCount;
		mi_VarRecCount = m_Header14.VarLenRecCount;
		mi_DataOffset = m_Header14.DataOffset;
		mf_MaxX = m_Header14.MaxX;
		mf_MinX = m_Header14.MinX;
		mf_MaxY = m_Header14.MaxY;
		mf_MinY = m_Header14.MinY;
		mf_MaxZ = m_Header14.MaxZ;
		mf_MinZ = m_Header14.MinZ;
		mf_ScaleX = m_Header14.XScale;
		mf_ScaleY = m_Header14.YScale;
		mf_ScaleZ = m_Header14.ZScale;
		mf_OffsetX = m_Header14.XOffset;
		mf_OffsetY = m_Header14.YOffset;
		mf_OffsetZ = m_Header14.ZOffset;
	}
	else
	{
		Logger::Write(__FUNCTION__, "Unsupported LAS file version: %d.%d.", m_Header10.VerMajor, m_Header10.VerMinor);
		_close(mi_FileHandle);
		return false;
	}

	strncpy(ms_FileName, fileName, _countof(ms_FileName) - 1);

	return true;
}

void LasFile::CloseFile()
{
	if (mi_FileHandle > 0)
	{
		int status = _close(mi_FileHandle);
		if (status != 0)
		{
			Logger::Write(__FUNCTION__, "close() failed, status = %d.", status);
		}
		else
		{
			mi_FileHandle = 0;
		}
	}
}

int LasFile::GetVersionMajor() const
{
	// offset/size same for all versions
	//

	if (mp_LasReader)
		return mp_LasReader->header.version_major;

	return (int)m_Header10.VerMajor;
}

int LasFile::GetVersionMinor() const
{
	// offset/size same for all versions
	//

	if (mp_LasReader)
		return mp_LasReader->header.version_minor;

	return (int)m_Header10.VerMinor;
}

UInt16 LasFile::GetFileSourceID() const
{
	if (mp_LasReader)
		return mp_LasReader->header.file_source_ID;

	return mi_FileSourceID;
}

UInt16 LasFile::GetGlobalEncoding() const
{
	// get global encoding bit mask
	//

	if (mp_LasReader)
		return mp_LasReader->header.global_encoding;

	return mi_GlobalEncoding;
}

const char* LasFile::GetGenSoftware() const
{
	// offset/size same for all versions
	//

	if (mp_LasReader)
		return mp_LasReader->header.generating_software;

	return m_Header10.GenSoftware;
}

const char* LasFile::GetSystemID() const
{
	// offset/size same for all versions
	//

	if (mp_LasReader)
		return mp_LasReader->header.system_identifier;

	return m_Header10.SystemID;
}

UInt16 LasFile::GetCreateDay() const
{
	if (mp_LasReader)
		return mp_LasReader->header.file_creation_day;

	return mi_FileCreateDay;
}

UInt16 LasFile::GetCreateYear() const
{
	if (mp_LasReader)
		return mp_LasReader->header.file_creation_year;

	return mi_FileCreateYear;
}

UInt16 LasFile::GetHeaderSize() const
{
	// offset/size same for all versions
	//

	if (mp_LasReader)
		return mp_LasReader->header.header_size;

	return m_Header10.HdrSize;
}

UInt64 LasFile::GetPointCount() const
{
	if (mp_LasReader)
	{
		// from 1.4 on, extended number of point records
		// always populated
		if (mp_LasReader->header.version_minor >= 4)
			return mp_LasReader->header.extended_number_of_point_records;
		return mp_LasReader->header.number_of_point_records;
	}

	return mi_PointCount;
}

UInt8 LasFile::GetPointFormat() const
{
	if (mp_LasReader)
		return mp_LasReader->header.point_data_format;

	return mi_PointFormat;
}

int LasFile::GetReturnMax() const
{
	// point rec types 0-5 have 3 bit return #s (1-5), from 6 on,
	// they have 4 bit values (1-15)
	if (GetPointFormat() < 6)
		return 5;
	else
		return 15;
}

UInt32 LasFile::GetVarRecCount() const
{
	if (mp_LasReader)
		return mp_LasReader->header.number_of_variable_length_records;

	return mi_VarRecCount;
}

double LasFile::GetMaxX() const
{
	if (mp_LasReader)
		return mp_LasReader->header.max_x;

	return mf_MaxX;
}

double LasFile::GetMinX() const
{
	if (mp_LasReader)
		return mp_LasReader->header.min_x;

	return mf_MinX;
}

double LasFile::GetMaxY() const
{
	if (mp_LasReader)
		return mp_LasReader->header.max_y;

	return mf_MaxY;
}

double LasFile::GetMinY() const
{
	if (mp_LasReader)
		return mp_LasReader->header.min_y;

	return mf_MinY;
}

double LasFile::GetMaxZ() const
{
	if (mp_LasReader)
		return mp_LasReader->header.max_z;

	return mf_MaxZ;
}

double LasFile::GetMinZ() const
{
	if (mp_LasReader)
		return mp_LasReader->header.min_z;

	return mf_MinZ;
}

double LasFile::GetXScale() const
{
	// x scale factor
	//

	if (mp_LasReader)
		return mp_LasReader->header.x_scale_factor;

	return mf_ScaleX;
}

double LasFile::GetYScale() const
{
	// y scale factor
	//

	if (mp_LasReader)
		return mp_LasReader->header.y_scale_factor;

	return mf_ScaleY;
}

double LasFile::GetZScale() const
{
	// z scale factor
	//

	if (mp_LasReader)
		return mp_LasReader->header.z_scale_factor;

	return mf_ScaleZ;
}

double LasFile::GetXOffset() const
{
	// X offset
	//

	if (mp_LasReader)
		return mp_LasReader->header.x_offset;

	return mf_OffsetX;
}

double LasFile::GetYOffset() const
{
	// Y offset
	//

	if (mp_LasReader)
		return mp_LasReader->header.y_offset;

	return mf_OffsetY;
}

double LasFile::GetZOffset() const
{
	// Z offset
	//

	if (mp_LasReader)
		return mp_LasReader->header.z_offset;

	return mf_OffsetZ;
}

VEC3 LasFile::GetScaledXYZ(UInt64 pointIndex)
{
	// Return the X value with scale/offset applied for given point index.
	//

	Int32 X = 0;
	Int32 Y = 0;
	Int32 Z = 0;

	//TODO:
	// how to get point x without switching through all point types?
	switch (GetPointFormat()) {
	case 0:
		X = GetPointRec_0(pointIndex).X;
		Y = GetPointRec_0(pointIndex).Y;
		Z = GetPointRec_0(pointIndex).Z;
		break;
	case 1:
		X = GetPointRec_1(pointIndex).X;
		Y = GetPointRec_1(pointIndex).Y;
		Z = GetPointRec_1(pointIndex).Z;
		break;
	case 2:
		X = GetPointRec_2(pointIndex).X;
		Y = GetPointRec_2(pointIndex).Y;
		Z = GetPointRec_2(pointIndex).Z;
		break;
	case 3:
		X = GetPointRec_3(pointIndex).X;
		Y = GetPointRec_3(pointIndex).Y;
		Z = GetPointRec_3(pointIndex).Z;
		break;
	case 4:
		X = GetPointRec_4(pointIndex).X;
		Y = GetPointRec_4(pointIndex).Y;
		Z = GetPointRec_4(pointIndex).Z;
		break;
	case 5:
		X = GetPointRec_5(pointIndex).X;
		Y = GetPointRec_5(pointIndex).Y;
		Z = GetPointRec_5(pointIndex).Z;
		break;
	case 6:
		X = GetPointRec_6(pointIndex).X;
		Y = GetPointRec_6(pointIndex).Y;
		Z = GetPointRec_6(pointIndex).Z;
		break;
	case 7:
		X = GetPointRec_7(pointIndex).X;
		Y = GetPointRec_7(pointIndex).Y;
		Z = GetPointRec_7(pointIndex).Z;
		break;
	case 8:
		X = GetPointRec_8(pointIndex).X;
		Y = GetPointRec_8(pointIndex).Y;
		Z = GetPointRec_8(pointIndex).Z;
		break;
	case 9:
		X = GetPointRec_9(pointIndex).X;
		Y = GetPointRec_9(pointIndex).Y;
		Z = GetPointRec_9(pointIndex).Z;
		break;
	case 10:
		X = GetPointRec_10(pointIndex).X;
		Y = GetPointRec_10(pointIndex).Y;
		Z = GetPointRec_10(pointIndex).Z;
		break;
	}

	VEC3 vecScaled;
	if (mp_LasReader)
	{
		vecScaled.x = mp_LasReader->get_x();;
		vecScaled.y = mp_LasReader->get_y();;
		vecScaled.z = mp_LasReader->get_z();;
	}
	else
	{
		vecScaled.x = ((double)X * GetXScale()) + GetXOffset();
		vecScaled.y = ((double)Y * GetYScale()) + GetYOffset();
		vecScaled.z = ((double)Z * GetZScale()) + GetZOffset();
	}

	return vecScaled;
}

int LasFile::GetPointClass(UInt64 pointIndex)
{
	// Return point classification.
	//
	
	int pt_class = -1;

	//TODO:
	// how to get point without switching through all point types?
	switch (GetPointFormat()) {
	case 0:
		pt_class = GetPointRec_0(pointIndex).Classification;
		break;
	case 1:
		pt_class = GetPointRec_1(pointIndex).Classification;
		break;
	case 2:
		pt_class = GetPointRec_2(pointIndex).Classification;
		break;
	case 3:
		pt_class = GetPointRec_3(pointIndex).Classification;
		break;
	case 4:
		pt_class = GetPointRec_4(pointIndex).Classification;
		break;
	case 5:
		pt_class = GetPointRec_5(pointIndex).Classification;
		break;
	case 6:
		pt_class = GetPointRec_6(pointIndex).Classification;
		break;
	case 7:
		pt_class = GetPointRec_7(pointIndex).Classification;
		break;
	case 8:
		pt_class = GetPointRec_8(pointIndex).Classification;
		break;
	case 9:
		pt_class = GetPointRec_9(pointIndex).Classification;
		break;
	case 10:
		pt_class = GetPointRec_10(pointIndex).Classification;
		break;
	default:
		assert(false);
	}
	
	return pt_class;
}

int LasFile::GetReturnNumber(UInt64 pointIndex)
{
	// Get return number.
	// 
	// point rec types 0-5 have 3 bit return #s (1-5), from 6 on,
	// they have 4 bit values (1-15)
	//

	int ret_num = 0;
	UInt8 ret_mask8 = 0;
	UInt16 ret_mask16 = 0;

	//TODO:
	// how to get point without switching through all point types?
	switch (GetPointFormat()) {
	case 0:
		ret_mask8 = GetPointRec_0(pointIndex).ReturnMask;
		break;
	case 1:
		ret_mask8 = GetPointRec_1(pointIndex).ReturnMask;
		break;
	case 2:
		ret_mask8 = GetPointRec_2(pointIndex).ReturnMask;
		break;
	case 3:
		ret_mask8 = GetPointRec_3(pointIndex).ReturnMask;
		break;
	case 4:
		ret_mask8 = GetPointRec_4(pointIndex).ReturnMask;
		break;
	case 5:
		ret_mask8 = GetPointRec_5(pointIndex).ReturnMask;
		break;
	case 6:
		ret_mask16 = GetPointRec_6(pointIndex).ReturnMask;
		break;
	case 7:
		ret_mask16 = GetPointRec_7(pointIndex).ReturnMask;
		break;
	case 8:
		ret_mask16 = GetPointRec_8(pointIndex).ReturnMask;
		break;
	case 9:
		ret_mask16 = GetPointRec_9(pointIndex).ReturnMask;
		break;
	case 10:
		ret_mask16 = GetPointRec_10(pointIndex).ReturnMask;
		break;
	default:
		assert(false);
	}

	if (ret_mask8 > 0)
		ret_num = ret_mask8 & 0x07;			// low 3 bits
	else
		ret_num = ret_mask16 & 0xF;			// low 4 bits

	return ret_num;
}

const LasFile::VarRecType_10 LasFile::GetVarRec_10(UInt32 recIndex) const
{
	// return a copy of a type 1.0 variable length record
	//
	// inputs:
	//		recIndex = 0-based index of variable length record
	// outputs:
	//		return = variable length record type 1.0
	//

	VarRecType_10 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetVarRecCount())
	{
		if (mp_LasReader)
		{
			memcpy(rec.Desc, mp_LasReader->header.vlrs[recIndex].description, sizeof(rec.Desc) - 1);
			rec.LenAfterHdr = mp_LasReader->header.vlrs[recIndex].record_length_after_header;
			rec.RecordID = mp_LasReader->header.vlrs[recIndex].record_id;
			// in 1.0, first 2 bytes are called "signature" and expected to be 0xAABB, 
			// after that they are just "reserved"
			rec.Signature = mp_LasReader->header.vlrs[recIndex].reserved;
			memcpy(rec.UserID, mp_LasReader->header.vlrs[recIndex].user_id, sizeof(rec.UserID) - 1);
		}
		else
		{
			long offset = GetHeaderSize();

			// since these are variable length, need to read sequentially 
			// from 0 to find the one we want

			for (UInt32 i = 0; i <= recIndex; ++i)
			{
				// read fixed portion
				_lseek(mi_FileHandle, offset, SEEK_SET);
				int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
				if (byteCount != sizeof(rec))
					Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);

				offset += sizeof(VarRecType_10);

				// add variable portion
				offset += rec.LenAfterHdr;
			}
		}
	}

	return rec;
}

const LasFile::VarRecType_13 LasFile::GetVarRec_13(UInt32 recIndex) const
{
	// return a copy of a type 1.3 variable length record
	//
	// inputs:
	//		recIndex = 0-based index of variable length record
	// outputs:
	//		return = variable length record type 1.3
	//
	// Note: This may be identical to other variable length records.
	//

	VarRecType_13 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetVarRecCount())
	{
		long offset = GetHeaderSize();

		// since these are variable length, need to read sequentially 
		// from 0 to find the one we want

		for (UInt32 i = 0; i <= recIndex; ++i)
		{
			// read fixed portion
			_lseek(mi_FileHandle, offset, SEEK_SET);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);

			offset += sizeof(VarRecType_13);

			// add variable portion
			offset += rec.LenAfterHdr;
		}
	}

	return rec;
}

const LasFile::VarRecType_14 LasFile::GetVarRec_14(UInt32 recIndex) const
{
	// return a copy of a type 1.4 variable length record
	//
	// inputs:
	//		recIndex = 0-based index of variable length record
	// outputs:
	//		return = variable length record type 1.4
	//
	// Note: This may be identical to other variable length records.
	//

	VarRecType_14 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetVarRecCount())
	{
		long offset = GetHeaderSize();

		// since these are variable length, need to read sequentially 
		// from 0 to find the one we want

		for (UInt32 i = 0; i <= recIndex; ++i)
		{
			// read fixed portion
			_lseek(mi_FileHandle, offset, SEEK_SET);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);

			offset += sizeof(VarRecType_14);

			// add variable portion
			offset += rec.LenAfterHdr;
		}
	}

	return rec;
}

const LasFile::PointRecType_0 LasFile::GetPointRec_0(UInt64 recIndex) const
{
	// get copy of point data record, format 0
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 0
	//

	assert(GetPointFormat() == 0);

	PointRecType_0 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.Classification = mp_LasReader->point.classification;
				//rec.FileMarker = 0xAABB;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask8(&mp_LasReader->point);
				rec.ScanAngle = mp_LasReader->point.scan_angle_rank;
				//rec.UserBitField = 
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_0));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

const LasFile::PointRecType_1 LasFile::GetPointRec_1(UInt64 recIndex) const
{
	// get copy of point data record, format 1
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 1
	//

	assert(GetPointFormat() == 1);

	PointRecType_1 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.Classification = mp_LasReader->point.classification;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask8(&mp_LasReader->point);
				rec.ScanAngle = mp_LasReader->point.scan_angle_rank;
				rec.GPSTime = mp_LasReader->point.gps_time;
				rec.PointSourceID = mp_LasReader->point.point_source_ID;
				rec.UserData = mp_LasReader->point.user_data;
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_1));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

const LasFile::PointRecType_2 LasFile::GetPointRec_2(UInt64 recIndex) const
{
	// get copy of point data record, format 2
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 2
	//

	assert(GetPointFormat() == 2);

	PointRecType_2 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask8(&mp_LasReader->point);
				rec.Classification = mp_LasReader->point.classification;
				rec.ScanAngle = mp_LasReader->point.scan_angle_rank;
				rec.UserData = mp_LasReader->point.user_data;
				rec.PointSourceID = mp_LasReader->point.point_source_ID;
				rec.Red = mp_LasReader->point.rgb[0];
				rec.Green = mp_LasReader->point.rgb[1];
				rec.Blue = mp_LasReader->point.rgb[2];
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_2));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

const LasFile::PointRecType_3 LasFile::GetPointRec_3(UInt64 recIndex) const
{
	// get copy of point data record, format 3
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 3
	//

	assert(GetPointFormat() == 3);

	PointRecType_3 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask8(&mp_LasReader->point);
				rec.Classification = mp_LasReader->point.classification;
				rec.ScanAngle = mp_LasReader->point.scan_angle_rank;
				rec.UserData = mp_LasReader->point.user_data;
				rec.PointSourceID = mp_LasReader->point.point_source_ID;
				rec.GPSTime = mp_LasReader->point.gps_time;
				rec.Red = mp_LasReader->point.rgb[0];
				rec.Green = mp_LasReader->point.rgb[1];
				rec.Blue = mp_LasReader->point.rgb[2];
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_3));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

const LasFile::PointRecType_4 LasFile::GetPointRec_4(UInt64 recIndex) const
{
	// get copy of point data record, format 4
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 4
	//

	assert(GetPointFormat() == 4);

	PointRecType_4 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask8(&mp_LasReader->point);
				rec.Classification = mp_LasReader->point.classification;
				rec.ScanAngle = mp_LasReader->point.scan_angle_rank;
				rec.UserData = mp_LasReader->point.user_data;
				rec.PointSourceID = mp_LasReader->point.point_source_ID;
				rec.GPSTime = mp_LasReader->point.gps_time;
				rec.Xt = mp_LasReader->point.wavepacket.getXt();
				rec.Yt = mp_LasReader->point.wavepacket.getYt();
				rec.Zt = mp_LasReader->point.wavepacket.getZt();
				rec.WaveByteOffset = mp_LasReader->point.wavepacket.getOffset();
				rec.WaveIndex = mp_LasReader->point.wavepacket.getIndex();
				rec.WaveLen = mp_LasReader->point.wavepacket.getSize();
				rec.WaveTimeOffset = mp_LasReader->point.wavepacket.getLocation();
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_4));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

const LasFile::PointRecType_5 LasFile::GetPointRec_5(UInt64 recIndex) const
{
	// get copy of point data record, format 5
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 5
	//

	assert(GetPointFormat() == 5);

	PointRecType_5 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask8(&mp_LasReader->point);
				rec.Classification = mp_LasReader->point.classification;
				rec.ScanAngle = mp_LasReader->point.scan_angle_rank;
				rec.UserData = mp_LasReader->point.user_data;
				rec.PointSourceID = mp_LasReader->point.point_source_ID;
				rec.GPSTime = mp_LasReader->point.gps_time;
				rec.Xt = mp_LasReader->point.wavepacket.getXt();
				rec.Yt = mp_LasReader->point.wavepacket.getYt();
				rec.Zt = mp_LasReader->point.wavepacket.getZt();
				rec.WaveByteOffset = mp_LasReader->point.wavepacket.getOffset();
				rec.WaveIndex = mp_LasReader->point.wavepacket.getIndex();
				rec.WaveSize = mp_LasReader->point.wavepacket.getSize();
				rec.WaveTimeOffset = mp_LasReader->point.wavepacket.getLocation();
				rec.Red = mp_LasReader->point.rgb[0];
				rec.Green = mp_LasReader->point.rgb[1];
				rec.Blue = mp_LasReader->point.rgb[2];
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_5));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

const LasFile::PointRecType_6 LasFile::GetPointRec_6(UInt64 recIndex) const
{
	// get copy of point data record, format 6
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 6
	//

	assert(GetPointFormat() == 6);

	PointRecType_6 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask16(&mp_LasReader->point);
				rec.Classification = mp_LasReader->point.classification;
				rec.ScanAngle = mp_LasReader->point.extended_scan_angle;
				rec.UserData = mp_LasReader->point.user_data;
				rec.PointSourceID = mp_LasReader->point.point_source_ID;
				rec.GPSTime = mp_LasReader->point.gps_time;
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_6));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

const LasFile::PointRecType_7 LasFile::GetPointRec_7(UInt64 recIndex) const
{
	// get copy of point data record, format 7
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 7
	//

	assert(GetPointFormat() == 7);

	PointRecType_7 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask16(&mp_LasReader->point);
				rec.Classification = mp_LasReader->point.classification;
				rec.ScanAngle = mp_LasReader->point.extended_scan_angle;
				rec.UserData = mp_LasReader->point.user_data;
				rec.PointSourceID = mp_LasReader->point.point_source_ID;
				rec.GPSTime = mp_LasReader->point.gps_time;
				rec.Red = mp_LasReader->point.rgb[0];
				rec.Green = mp_LasReader->point.rgb[1];
				rec.Blue = mp_LasReader->point.rgb[2];
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_7));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

const LasFile::PointRecType_8 LasFile::GetPointRec_8(UInt64 recIndex) const
{
	// get copy of point data record, format 8
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 8
	//

	assert(GetPointFormat() == 8);

	PointRecType_8 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask16(&mp_LasReader->point);
				rec.Classification = mp_LasReader->point.classification;
				rec.ScanAngle = mp_LasReader->point.extended_scan_angle;
				rec.UserData = mp_LasReader->point.user_data;
				rec.PointSourceID = mp_LasReader->point.point_source_ID;
				rec.GPSTime = mp_LasReader->point.gps_time;
				rec.Red = mp_LasReader->point.rgb[0];
				rec.Green = mp_LasReader->point.rgb[1];
				rec.Blue = mp_LasReader->point.rgb[2];
				rec.NIR = mp_LasReader->point.get_NIR();
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_8));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

const LasFile::PointRecType_9 LasFile::GetPointRec_9(UInt64 recIndex) const
{
	// get copy of point data record, format 9
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 9
	//

	assert(GetPointFormat() == 9);

	PointRecType_9 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask16(&mp_LasReader->point);
				rec.Classification = mp_LasReader->point.classification;
				rec.ScanAngle = mp_LasReader->point.extended_scan_angle;
				rec.UserData = mp_LasReader->point.user_data;
				rec.PointSourceID = mp_LasReader->point.point_source_ID;
				rec.GPSTime = mp_LasReader->point.gps_time;
				rec.Xt = mp_LasReader->point.wavepacket.getXt();
				rec.Yt = mp_LasReader->point.wavepacket.getYt();
				rec.Zt = mp_LasReader->point.wavepacket.getZt();
				rec.WaveByteOffset = mp_LasReader->point.wavepacket.getOffset();
				rec.WaveIndex = mp_LasReader->point.wavepacket.getIndex();
				rec.WaveLen = mp_LasReader->point.wavepacket.getSize();
				rec.WaveTimeOffset = mp_LasReader->point.wavepacket.getLocation();
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_9));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

const LasFile::PointRecType_10 LasFile::GetPointRec_10(UInt64 recIndex) const
{
	// get copy of point data record, format 10
	//
	// inputs:
	//		recIndex = 0-based index of point data record
	// outputs:
	//		return = point data record, format 10
	//

	assert(GetPointFormat() == 10);

	PointRecType_10 rec;					// return value
	memset(&rec, 0, sizeof(rec));

	if (recIndex < GetPointCount())
	{
		if (mp_LasReader)
		{
			if (mp_LasReader->seek(recIndex) && mp_LasReader->read_point())
			{
				rec.X = mp_LasReader->point.X;
				rec.Y = mp_LasReader->point.Y;
				rec.Z = mp_LasReader->point.Z;
				rec.Intensity = mp_LasReader->point.intensity;
				rec.ReturnMask = PackReturnMask16(&mp_LasReader->point);
				rec.Classification = mp_LasReader->point.classification;
				rec.ScanAngle = mp_LasReader->point.extended_scan_angle;
				rec.UserData = mp_LasReader->point.user_data;
				rec.PointSourceID = mp_LasReader->point.point_source_ID;
				rec.GPSTime = mp_LasReader->point.gps_time;
				rec.Xt = mp_LasReader->point.wavepacket.getXt();
				rec.Yt = mp_LasReader->point.wavepacket.getYt();
				rec.Zt = mp_LasReader->point.wavepacket.getZt();
				rec.WaveByteOffset = mp_LasReader->point.wavepacket.getOffset();
				rec.WaveIndex = mp_LasReader->point.wavepacket.getIndex();
				rec.WaveLen = mp_LasReader->point.wavepacket.getSize();
				rec.WaveTimeOffset = mp_LasReader->point.wavepacket.getLocation();
				rec.Red = mp_LasReader->point.rgb[0];
				rec.Green = mp_LasReader->point.rgb[1];
				rec.Blue = mp_LasReader->point.rgb[2];
				rec.NIR = mp_LasReader->point.get_NIR();
			}
			else
			{
				Logger::Write(__FUNCTION__, "Unable to read point %lu", recIndex);
			}
		}
		else
		{
			// point count is UInt64, but _lseeki64() offset is signed
			Int64 offset = mi_DataOffset + (recIndex * sizeof(PointRecType_10));

			Int64 pos = _lseeki64(mi_FileHandle, offset, SEEK_SET);
			if (pos != offset)
				Logger::Write(__FUNCTION__, "lseeki64() returned unexpected value: expected %d, returned %d", offset, pos);
			int byteCount = _read(mi_FileHandle, &rec, sizeof(rec));
			if (byteCount != sizeof(rec))
				Logger::Write(__FUNCTION__, "read() returned unexpected byte count: expected %d, read %d", sizeof(rec), byteCount);
		}
	}

	return rec;
}

void LasFile::GetExtents(double& latN, double& latS, double& lonW, double& lonE)
{
	//TODO:
	//what are they?
	latN = 0.0;
	latS = 0.0;
	lonE = 0.0;
	lonW = 0.0;
}

double LasFile::GetMinElev()
{
	//TODO:
	return 0.0;
}

double LasFile::GetMaxElev()
{
	//TODO:
	return 0.0;
}

XString LasFile::GetMetaData()
{
	XString meta;		// return value
	XString str;

	str = XString::Format("File Name: %s\n", ms_FileName); meta += str;
	str = XString::Format("Version: %d.%d\n", GetVersionMajor(), GetVersionMinor()); meta += str;
	str = XString::Format("Point Format: %u\n", GetPointFormat()); meta += str;
	str = XString::Format("Point Count: %llu\n", GetPointCount()); meta += str;
	str = XString::Format("Variable Record Count: %lu\n", GetVarRecCount()); meta += str;
	str = XString::Format("File Source ID: %lu\n", GetFileSourceID()); meta += str;
	str = XString::Format("Global Encoding: 0x%04x\n", GetGlobalEncoding()); meta += str;
	str = XString::Format("Header Size: %lu\n", GetHeaderSize()); meta += str;
	str = XString::Format("System ID: %s\n", GetSystemID()); meta += str;
	str = XString::Format("Gen Software: %s\n", GetGenSoftware()); meta += str;
	str = XString::Format("Create Day:  %u\n", GetCreateDay()); meta += str;
	str = XString::Format("Create Year: %u\n", GetCreateYear()); meta += str;
	str = XString::Format("Min X: %f\n", GetMinX()); meta += str;
	str = XString::Format("Max X: %f\n", GetMaxX()); meta += str;
	str = XString::Format("Min Y: %f\n", GetMinY()); meta += str;
	str = XString::Format("Max Y: %f\n", GetMaxY()); meta += str;
	str = XString::Format("Min Z: %f\n", GetMinZ()); meta += str;
	str = XString::Format("Max Z: %f\n", GetMaxZ()); meta += str;
	str = XString::Format("X Scale: %f\n", GetXScale()); meta += str;
	str = XString::Format("Y Scale: %f\n", GetYScale()); meta += str;
	str = XString::Format("Z Scale: %f\n", GetZScale()); meta += str;
	str = XString::Format("X Offset: %f\n", GetXOffset()); meta += str;
	str = XString::Format("Y Offset: %f\n", GetYOffset()); meta += str;
	str = XString::Format("Z Offset: %f\n", GetZOffset()); meta += str;

	return meta;
}

UInt16 LasFile::GetVersion() const
{
	// return version as single integer
	//

	UInt16 ver = 0;

	if (GetVersionMajor() == 1)
	{
		switch (GetVersionMinor())
		{
		case 0: ver = 10; break;
		case 1: ver = 11; break;
		case 2: ver = 12; break;
		case 3: ver = 13; break;
		case 4: ver = 14; break;
		}
	}

	assert(ver > 0);

	return ver;
}

UInt8 LasFile::PackReturnMask8(LASpoint* pPoint) const
{
	// Pack an 8-bit return mask
	//
	// inputs:
	//		pPoint = LAS point format 0-5 record
	// outputs:
	//		return = 8-bit mask as defined by LAS
	//				 format specifications for point
	//				 types 0 through 5
	//

	// 8 bit mask
	//		3 bits - Return Number
	//		3 bits - Number of Returns (given pulse)
	//		1 bit  - Scan Direction Flag
	//		1 bit  - Edge of Flight Line

	UInt8 returnMask = (pPoint->return_number & 0x07)
		| (pPoint->number_of_returns & 0x07) << 3
		| (pPoint->scan_direction_flag & 0x01) << 6
		| (pPoint->edge_of_flight_line & 0x01) << 7;

	return returnMask;
}

UInt16 LasFile::PackReturnMask16(LASpoint* pPoint) const
{
	// Pack a 16-bit return mask
	//
	// inputs:
	//		pPoint = LAS point format 6+ record
	// outputs:
	//		return = 16-bit mask as defined by LAS
	//				 format specifications for point
	//				 types 6 through 10

	// 16 bit mask
	//		4 bits (0 - 3), Return Number *
	//		4 bits (4 - 7), Number of Returns(given pulse) *
	//		4 bits (0 - 3), Classification Flags
	//		2 bits (4 - 5), Scanner Channel *
	//		1 bit (6), Scan Direction Flag *
	//		1 bit (7), Edge of Flight Line *

	UInt16 returnMask = (pPoint->return_number & 0x0F)
		| (pPoint->number_of_returns & 0x0F) << 4
		| (pPoint->extended_classification_flags & 0x0F) << 8
		| (pPoint->extended_scanner_channel & 0x03) << 12
		| (pPoint->scan_direction_flag & 0x01) << 14
		| (pPoint->edge_of_flight_line & 0x01) << 15;

	return returnMask;
}
