#ifndef LASFILE_H
#define LASFILE_H

#include "lasreader.hpp"
//#include "laswriter.h"
//#include "laswritercompatible.h"
//#include "laswaveform13reader.h"
//#include "laswaveform13writer.h"
//#include "bytestreamin.h"
//#include "bytestreamout_array.h"
//#include "bytestreamin_array.h"
////#include "geoprojectionconverter.h"
//#include "lasindex.h"
//#include "lasquadtree.h"

#include "XString.h"
#include "Gis.h"

class LasFile
{
public:

	// these data types are public because the are returned
	// directly from class methods

#pragma pack(1)

	// variable length records

//TODO:
//these all the same?
	struct VarRecType_10		// version 1.0 variable length record
	{
		UInt16 Signature;		// 2 bytes, Record Signature (0xAABB) *
		char UserID[16];		// 16 bytes, User ID *
		UInt16 RecordID;		// 2 bytes, unsigned short, RecordID *
		UInt16 LenAfterHdr;		// 2 bytes, unsigned short,	Record Length After Header *
		char Desc[32];			// 32 bytes
	};

	struct VarRecType_11		// version 1.1 variable length record
	{
		UInt16 Reserved;		// 2 bytes
		char UserID[16];		// 16 bytes, User ID *
		UInt16 RecordID;		// 2 bytes, unsigned short, RecordID *
		UInt16 LenAfterHdr;		// 2 bytes, unsigned short,	Record Length After Header *
		char Desc[32];			// 32 bytes
	};

	struct VarRecType_13		// version 1.3 variable length record
	{
		UInt16 Reserved;		// 2 bytes
		char UserID[16];		// 16 bytes, User ID *
		UInt16 RecordID;		// 2 bytes, unsigned short, RecordID *
		UInt16 LenAfterHdr;		// 2 bytes, unsigned short,	Record Length After Header *
		char Desc[32];			// 32 bytes
	};

	struct VarRecType_14		// version 1.4 variable length record
	{
		UInt16 Reserved;		// 2 bytes, must be set to zero
		char UserID[16];		// 16 bytes, User ID *
		UInt16 RecordID;		// 2 bytes, unsigned short, RecordID *
		UInt16 LenAfterHdr;		// 2 bytes, unsigned short,	Record Length After Header *
		char Desc[32];			// 32 bytes
	};

	// point data records

	struct PointRecType_0		// point data record format 0, 20 bytes
	{
		Int32  X;				// 4 bytes,	long *
		Int32  Y;				// 4 bytes,	long *
		Int32  Z;				// 4 bytes,	long *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt8  ReturnMask;		// 8 bit mask, we don't use bit fields because implementation may not pack them
								// 3 bits Return Number, numbered 1-5 *
								// 3 bits Number of Returns (given pulse) *
								// 1 bit Scan Direction Flag *
								// 1 bit Edge of Flight Line *
		UInt8  Classification;	// 1 byte
		Int8   ScanAngle;		// 1 byte, Scan Angle Rank (-90 to + 90) – Left side	*
		UInt8  FileMarker;		// 1 byte, File Marker,	unsigned char		
		UInt16 UserBitField;	// 2 bytes, User Bit Field,	unsigned short		
	};

	struct PointRecType_1		// point data record format 1, 28 bytes
	{
		Int32  X;				// 4 bytes,	long  *
		Int32  Y;				// 4 bytes,	long  *
		Int32  Z;				// 4 bytes,	long  *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt8  ReturnMask;		// 8 bit mask, we don't use bit field because implementation may not pack them
								// 3 bits Return Number, numbered 1-5  *
								// 3 bits Number of Returns (given pulse) *
								// 1 bit Scan Direction Flag *
								// 1 bit Edge of Flight Line *
		UInt8  Classification;	// 1 byte
		Int8   ScanAngle;		// 1 byte, Scan Angle Rank (-90 to + 90) – Left side	*
		UInt8  UserData;		// 1 byte, File Marker,	unsigned char, available for any use
		UInt16 PointSourceID;	// 2 bytes, User Bit Field,	unsigned short		
		double GPSTime;			// 8 bytes, double *
	};

	struct PointRecType_2		// data record format 2, 26 bytes
	{
		Int32  X;				// 4 bytes,	long *
		Int32  Y;				// 4 bytes,	long *
		Int32  Z;				// 4 bytes,	long *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt8  ReturnMask;		// 8 bit mask, we don't use bit fields because implementation may not pack them
								// 3 bits Return Number, numbered 1-5  *
								// 3 bits Number of Returns (given pulse) *
								// 1 bit Scan Direction Flag *
								// 1 bit Edge of Flight Line *
		UInt8  Classification;	// 1 byte
		Int8   ScanAngle;		// 1 byte, Scan Angle Rank (-90 to + 90) – Left side	*
		UInt8  UserData;		// 1 byte, unsigned char
		UInt16 PointSourceID;	// 2 bytes, File from which this point originated, unsigned short *
		UInt16 Red;				// 2 bytes, unsigned short *
		UInt16 Green;			// 2 bytes, unsigned short *
		UInt16 Blue;			// 2 bytes, unsigned short *
	};

	struct PointRecType_3		// data record format 3, 34 bytes
	{
		Int32  X;				// 4 bytes,	long *
		Int32  Y;				// 4 bytes,	long *
		Int32  Z;				// 4 bytes,	long *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt8  ReturnMask;		// 8 bit mask, we don't use bit fields because implementation may not pack them
								// 3 bits Return Number, numbered 1-5  *
								// 3 bits Number of Returns (given pulse) *
								// 1 bit Scan Direction Flag *
								// 1 bit Edge of Flight Line *
		UInt8  Classification;	// 1 byte
		Int8   ScanAngle;		// 1 byte, Scan Angle Rank (-90 to + 90) – Left side	*
		UInt8  UserData;		// 1 byte, unsigned char
		UInt16 PointSourceID;	// 2 bytes, File from which this point originated, unsigned short *
		double GPSTime;			// 8 bytes, double *
		UInt16 Red;				// 2 bytes, unsigned short *
		UInt16 Green;			// 2 bytes, unsigned short *
		UInt16 Blue;			// 2 bytes, unsigned short *
	};

	struct PointRecType_4		// data record format 4, 57 bytes
	{
		Int32  X;				// 4 bytes,	long  *
		Int32  Y;				// 4 bytes,	long  *
		Int32  Z;				// 4 bytes,	long  *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt8  ReturnMask;		// 8 bit mask, we don't use bit fields because implementation may not pack them
								// 3 bits Return Number, numbered 1-5  *
								// 3 bits Number of Returns (given pulse) *
								// 1 bit Scan Direction Flag *
								// 1 bit Edge of Flight Line *
		UInt8  Classification;	// 1 byte, ungined char *
		Int8   ScanAngle;		// 1 byte, Scan Angle Rank (-90 to + 90) – Left side	*
		UInt8  UserData;		// 1 byte, unsigned char
		UInt16 PointSourceID;	// 2 bytes, File from which this point originated, unsigned short *
		double GPSTime;			// 8 bytes, double *
		UInt8  WaveIndex;		// 1 byte, Wave Packet Descriptor Index, unsigned char *
		UInt64 WaveByteOffset;	// 8 bytes, Byte offset to waveform data, unsigned long long *
		UInt32 WaveLen;			// 4 bytes, Waveform packet size in bytes, unsigned long *
		float  WaveTimeOffset;	// 4 bytes, Return Point Waveform Location,	float *
		float  Xt;				// 4 bytes, float *
		float  Yt;				// 4 bytes, float *
		float  Zt;				// 4 bytes, float *
	};

	struct PointRecType_5		// data record format 5, 63 bytes
	{
		Int32  X;				// 4 bytes,	long *
		Int32  Y;				// 4 bytes,	long *
		Int32  Z;				// 4 bytes,	long *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt8  ReturnMask;		// 8 bit mask, we don't use bit fields because implementation may not pack them
								// 3 bits Return Number, numbered 1-5  *
								// 3 bits Number of Returns (given pulse) *
								// 1 bit Scan Direction Flag *
								// 1 bit Edge of Flight Line *
		UInt8  Classification;	// 1 byte, unsigned char *
		Int8   ScanAngle;		// 1 byte, Scan Angle Rank (-90 to + 90) – Left side, char *
		UInt8  UserData;		// 1 byte, User Data, unsigned char
		UInt16 PointSourceID;	// 2 bytes, File from which this point originated, unsigned short *
		double GPSTime;			// 8 bytes, GPS Time, double *
		UInt16 Red;				// 2 bytes, unsigned short, *
		UInt16 Green;			// 2 bytes, unsigned short, *
		UInt16 Blue;			// 2 bytes, unsigned short, *
		UInt8  WaveIndex;		// 1 byte, Wave Packet Descriptor Index, unsigned char *
		UInt64 WaveByteOffset;	// 8 bytes, Byte offset to waveform data, unsigned long long *
		UInt32 WaveSize;		// 4 bytes, Waveform packet size in bytes, unsigned long *
		float  WaveTimeOffset;	// 4 bytes, Return Point Waveform Location,	float *
		float  Xt;				// 4 bytes, X(t), float *
		float  Yt;				// 4 bytes, Y(t), float *
		float  Zt;				// 4 bytes, Z(t), float *
	};

	struct PointRecType_6		// data record format 6, common to types 6-10, 30 bytes
	{
		Int32  X;				// 4 bytes,	long *
		Int32  Y;				// 4 bytes,	long *
		Int32  Z;				// 4 bytes,	long *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt16 ReturnMask;		// 16 bit mask
								// 4 bits (0 - 3), Return Number, numbered 1-15 *
								// 4 bits (4 - 7), Number of Returns (given pulse) *
								// 4 bits (0 - 3), Classification Flags
								// 2 bits (4 - 5), Scanner Channel *
								// 1 bit (6), Scan Direction Flag *
								// 1 bit (7), Edge of Flight Line *
		UInt8  Classification;	// 1 byte, unsigned char *
		UInt8  UserData;		// 1 byte, unsigned char
		Int16  ScanAngle;		// 2 bytes, short *
		UInt16 PointSourceID;	// 2 bytes, File from which this point originated, unsigned short *
		double GPSTime;			// 8 bytes, double *
	};

	struct PointRecType_7		// data record format 7, 36 bytes
	{
		Int32  X;				// 4 bytes,	long *
		Int32  Y;				// 4 bytes,	long *
		Int32  Z;				// 4 bytes,	long *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt16 ReturnMask;		// 16 bit mask
								// 4 bits (0 - 3), Return Number, numbered 1-15 *
								// 4 bits (4 - 7), Number of Returns (given pulse) *
								// 4 bits (0 - 3), Classification Flags
								// 2 bits (4 - 5), Scanner Channel *
								// 1 bit (6), Scan Direction Flag *
								// 1 bit (7), Edge of Flight Line *
		UInt8  Classification;	// 1 byte, unsigned char *
		UInt8  UserData;		// 1 byte, unsigned char
		Int16  ScanAngle;		// 2 bytes, short *
		UInt16 PointSourceID;	// 2 bytes, File from which this point originated, unsigned short *
		double GPSTime;			// 8 bytes, double *
		UInt16 Red;				// 2 bytes, unsigned short *
		UInt16 Green;			// 2 bytes, unsigned short *
		UInt16 Blue;			// 2 bytes, unsigned short *
	};

	struct PointRecType_8		// data record format 8, 38 bytes
	{
		Int32  X;				// 4 bytes,	long *
		Int32  Y;				// 4 bytes,	long *
		Int32  Z;				// 4 bytes,	long *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt16 ReturnMask;		// 16 bit mask
								// 4 bits (0 - 3), Return Number, numbered 1-15 *
								// 4 bits (4 - 7), Number of Returns (given pulse) *
								// 4 bits (0 - 3), Classification Flags
								// 2 bits (4 - 5), Scanner Channel *
								// 1 bit (6), Scan Direction Flag *
								// 1 bit (7), Edge of Flight Line *
		UInt8  Classification;	// 1 byte, unsigned char *
		UInt8  UserData;		// 1 byte, unsigned char
		Int16  ScanAngle;		// 2 bytes, short *
		UInt16 PointSourceID;	// 2 bytes, File from which this point originated, unsigned short *
		double GPSTime;			// 8 bytes, double *
		UInt16 Red;				// 2 bytes, unsigned short *
		UInt16 Green;			// 2 bytes, unsigned short *
		UInt16 Blue;			// 2 bytes, unsigned short *
		UInt16 NIR;				// 2 bytes, Near Infrared, unsigned short *
	};

	struct PointRecType_9		// data record format 9, 59 bytes
	{
		Int32  X;				// 4 bytes,	long *
		Int32  Y;				// 4 bytes,	long *
		Int32  Z;				// 4 bytes,	long *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt16 ReturnMask;		// 16 bit mask
								// 4 bits (0 - 3), Return Number, numbered 1-15 *
								// 4 bits (4 - 7), Number of Returns (given pulse) *
								// 4 bits (0 - 3), Classification Flags
								// 2 bits (4 - 5), Scanner Channel *
								// 1 bit (6), Scan Direction Flag *
								// 1 bit (7), Edge of Flight Line *
		UInt8  Classification;	// 1 byte, unsigned char *
		UInt8  UserData;		// 1 byte, unsigned char
		Int16  ScanAngle;		// 2 bytes, short *
		UInt16 PointSourceID;	// 2 bytes, File from which this point originated, unsigned short *
		double GPSTime;			// 8 bytes, double *
		UInt8  WaveIndex;		// 1 byte, Wave Packet Descriptor Index, unsigned char *
		UInt64 WaveByteOffset;	// 8 bytes, Byte offset to waveform data, unsigned long long *
		UInt32 WaveLen;			// 4 bytes, Waveform packet size in bytes, unsigned long *
		float  WaveTimeOffset;	// 4 bytes, Return Point Waveform Location,	float *
		float  Xt;				// 4 bytes, float *
		float  Yt;				// 4 bytes, float *
		float  Zt;				// 4 bytes, float *
	};

	struct PointRecType_10		// data record format 10, 67 bytes
	{
		Int32  X;				// 4 bytes,	long *
		Int32  Y;				// 4 bytes,	long *
		Int32  Z;				// 4 bytes,	long *
		UInt16 Intensity;		// 2 bytes unsigned short
		UInt16 ReturnMask;		// 16 bit mask
								// 4 bits (0 - 3), Return Number, numbered 1-15 *
								// 4 bits (4 - 7), Number of Returns (given pulse) *
								// 4 bits (0 - 3), Classification Flags
								// 2 bits (4 - 5), Scanner Channel *
								// 1 bit (6), Scan Direction Flag *
								// 1 bit (7), Edge of Flight Line *
		UInt8  Classification;	// 1 byte, unsigned char *
		UInt8  UserData;		// 1 byte, unsigned char
		Int16  ScanAngle;		// 2 bytes, short *
		UInt16 PointSourceID;	// 2 bytes, File from which this point originated, unsigned short *
		double GPSTime;			// 8 bytes, double *
		UInt16 Red;				// 2 bytes, unsigned short *
		UInt16 Green;			// 2 bytes, unsigned short *
		UInt16 Blue;			// 2 bytes, unsigned short *
		UInt16 NIR;				// 2 bytes, Near Infrared, unsigned short *
		UInt8  WaveIndex;		// 1 byte, Wave Packet Descriptor Index, unsigned char *
		UInt64 WaveByteOffset;	// 8 bytes, Byte offset to waveform data, unsigned long long *
		UInt32 WaveLen;			// 4 bytes, Waveform packet size in bytes, unsigned long *
		float  WaveTimeOffset;	// 4 bytes, Return Point Waveform Location,	float *
		float  Xt;				// 4 bytes, float *
		float  Yt;				// 4 bytes, float *
		float  Zt;				// 4 bytes, float *
	};

#pragma pack()

public:

	// construction / destruction

	LasFile(const char* fileName);
	virtual ~LasFile();

	const char* GetFileName() const { return ms_FileName; }

	int GetVersionMajor() const;
	int GetVersionMinor() const;
	
	UInt16 GetFileSourceID() const;
	UInt16 GetGlobalEncoding() const;

	const char* GetGenSoftware() const;
	const char* GetSystemID() const;
	UInt16 GetCreateDay() const;
	UInt16 GetCreateYear() const;
	UInt16 GetHeaderSize() const;
	UInt64 GetPointCount() const;
	UInt8  GetPointFormat() const;
	UInt32 GetVarRecCount() const;
	int    GetReturnMax() const;

	double GetMaxX() const;
	double GetMinX() const;
	double GetMaxY() const;
	double GetMinY() const;
	double GetMaxZ() const;
	double GetMinZ() const;
	double GetXScale() const;
	double GetYScale() const;
	double GetZScale() const;
	double GetXOffset() const;
	double GetYOffset() const;
	double GetZOffset() const;

	VEC3 GetScaledXYZ(UInt64 pointIndex);		// xyz coordinates with scale/offset applied for given point
	int  GetPointClass(UInt64 pointIndex);		// point classification
	int  GetReturnNumber(UInt64 pointIndex);	// return #, 1-GetReturnMax()

	const VarRecType_10 GetVarRec_10(UInt32 recIndex) const;			// get copy of variable length record, version 1.0
	const VarRecType_13 GetVarRec_13(UInt32 recIndex) const;			// get copy of variable length record, version 1.3
	const VarRecType_14 GetVarRec_14(UInt32 recIndex) const;			// get copy of variable length record, version 1.4

	const PointRecType_0 GetPointRec_0(UInt64 recIndex) const;			// get copy of point data record, format 0
	const PointRecType_1 GetPointRec_1(UInt64 recIndex) const;			// get copy of point data record, format 1
	const PointRecType_2 GetPointRec_2(UInt64 recIndex) const;			// get copy of point data record, format 2
	const PointRecType_3 GetPointRec_3(UInt64 recIndex) const;			// get copy of point data record, format 3
	const PointRecType_4 GetPointRec_4(UInt64 recIndex) const;			// get copy of point data record, format 4
	const PointRecType_5 GetPointRec_5(UInt64 recIndex) const;			// get copy of point data record, format 5
	const PointRecType_6 GetPointRec_6(UInt64 recIndex) const;			// get copy of point data record, format 6
	const PointRecType_7 GetPointRec_7(UInt64 recIndex) const;			// get copy of point data record, format 7
	const PointRecType_8 GetPointRec_8(UInt64 recIndex) const;			// get copy of point data record, format 8
	const PointRecType_9 GetPointRec_9(UInt64 recIndex) const;			// get copy of point data record, format 9
	const PointRecType_10 GetPointRec_10(UInt64 recIndex) const;		// get copy of point data record, format 10

	GIS::GEODATA GetFileType() { return mp_LasReader == nullptr ? GIS::GEODATA::LAS : GIS::GEODATA::LAZ; }
	XString GetMetaData();
	void GetExtents(double& yn, double& ys, double& xe, double& xw);
	double GetMinElev();
	double GetMaxElev();

private:

	// version 1.0 file header
	// (All data that is not required and not filled with data must be set to zeros)
	// * = required field
	//

#pragma pack(1)

	struct HeaderType_10					// version 1.0 header
	{
		char Signature[4];					// 4 bytes, "LASF" *
		UInt32 Reserved;					// 4 bytes, reserved
		UInt32 GUID1;						// 4 bytes
		UInt16 GUID2;						// 2 byte
		UInt16 GUID3;						// 2 byte
		unsigned char GUID4[8];				// 8 bytes
		unsigned char VerMajor;				// 1 byte *
		unsigned char VerMinor;				// 1 byte *
		char SystemID[32];					// 32 bytes, populated by the generating software *
		char GenSoftware[32];				// 32 bytes, populated by the generating software *
		UInt16 FlightDate;					// 2 bytes, julian day of the year data was collected
		UInt16 Year;						// 2 bytes, year in which the data was collected
		UInt16 HdrSize;						// 2 bytes *
		UInt32 DataOffset;					// 4 bytes, bytes from beginning of file to start of data *
		UInt32 VarLenRecCount;				// 4 bytes, Number of variable length records *
		UInt8  PointFormat;					// 1 byte, Point Data Format ID (0 - 99 for spec), unsigned char *
											//		Version 1.0 only had types 0-1
		UInt16 PointDataRecLen;				// 2 bytes, Point Data Record Length, unsigned short *
		UInt32 PointDataRecCount;			// 4 bytes, Number of point records, unsigned long *
		UInt32 ReturnPoints[5];				// 20 bytes, Number of points by return, unsigned long[5] *
		double XScale;						// 8 bytes X scale factor *
		double YScale;						// 8 bytes Y scale factor *
		double ZScale;						// 8 bytes Z scale factor *
		double XOffset;						// 8 bytes X offset *
		double YOffset;						// 8 bytes Y offset *
		double ZOffset;						// 8 bytes Z offset *
		double MaxX;						// 8 bytes, it appears that these are scaled values (VERIFY) *
		double MinX;						// 8 bytes *
		double MaxY;						// 8 bytes *
		double MinY;						// 8 bytes *
		double MaxZ;						// 8 bytes *
		double MinZ;						// 8 bytes *
	};

	struct HeaderType_11					// version 1.1 header
	{
		char Signature[4];					// 4 bytes, "LASF" *
		UInt16 FileSourceID;				// (1.1) 2 bytes, File Source ID unsigned short	*
		UInt16 Reserved;					// (1.1) 2 bytes, Reserved	unsigned short
		UInt32 GUID1;						// (1.1) 4 bytes, Project ID - GUID data 1 
		UInt16 GUID2;						// (1.1) 2 bytes, Project ID - GUID data 2 
		UInt16 GUID3;						// (1.1) 2 bytes, Project ID - GUID data 3 
		unsigned char GUID4[8];				// (1.1) 8 bytes, Project ID - GUID data 4 
		unsigned char VerMajor;				// 1 byte *
		unsigned char VerMinor;				// 1 byte *
		char SystemID[32];					// (1.1) 32 bytes, populated by the generating software, char[32] *
		char GenSoftware[32];				// 32 bytes, populated by the generating software *
		UInt16 FileCreateDay;				// (1.1) 2 bytes, File Creation Day of Year, unsigned short
		UInt16 FileCreateYear;				// (1.1) 2 bytes, File Creation Year, unsigned short
		UInt16 HdrSize;						// 2 bytes, Header Size, unsigned short *
		UInt32 DataOffset;					// 4 bytes, Offset to point data, unsigned long *
		UInt32 VarLenRecCount;				// 4 bytes, Number of variable length records, unsigned long *
		UInt8  PointFormat;					// 1 byte, Point Data Format ID(0 - 99 for spec), unsigned char *
											//		Version 1.1 only had types 0-1
		UInt16 PointDataRecLen;				// 2 bytes, Point Data Record Length, unsigned short *
		UInt32 PointDataRecCount;			// 4 bytes, Number of point records, unsigned long *
		UInt32 ReturnPoints[5];				// 20 bytes, Number of points by return, unsigned long[5] *
		double XScale;						// 8 bytes X scale factor *
		double YScale;						// 8 bytes Y scale factor *
		double ZScale;						// 8 bytes Z scale factor *
		double XOffset;						// 8 bytes X offset *
		double YOffset;						// 8 bytes Y offset *
		double ZOffset;						// 8 bytes Z offset *
		double MaxX;						// 8 bytes *
		double MinX;						// 8 bytes *
		double MaxY;						// 8 bytes *
		double MinY;						// 8 bytes *
		double MaxZ;						// 8 bytes *
		double MinZ;						// 8 bytes *
	};

	// version 1.2 header (same as 1.1 except usage of GlobalID field)

	struct HeaderType_12					// version 1.2 header
	{
		char Signature[4];					// 4 bytes, "LASF" *
		UInt16 FileSourceID;				// 2 bytes, File Source ID unsigned short *
		UInt16 GlobalEncoding;				// (1.2) 2 bytes, bit 0: gps time, bit 1-15: not used, unsigned short
				// Bits  Field Name	    Description
				// 0     GPS Time Type	The meaning of GPS Time in the Point Records
				//						0 -> GPS time in the point record fields
				//						 	 is GPS Week Time(the same as previous versions of LAS)
				//						1 -> GPS Time is standard GPS Time (satellite GPS Time)
				//							 minus 1 x 10^9. The offset moves the time back to near zero 
				//							 to improve floating point resolution.
				// 1:15  Reserved		Must be set to zero
		UInt32 GUID1;						// 4 bytes Project ID - GUID data 1 
		UInt16 GUID2;						// 2 bytes Project ID - GUID data 2 
		UInt16 GUID3;						// 2 bytes Project ID - GUID data 3 
		unsigned char GUID4[8];				// 8 bytes Project ID - GUID data 4 
		unsigned char VerMajor;				// 1 byte *
		unsigned char VerMinor;				// 1 byte *
		char SystemID[32];					// 32 bytes, populated by the generating software, char[32] *
		char GenSoftware[32];				// 32 bytes, populated by the generating software, char[32] *
		UInt16 FileCreateDay;				// 2 bytes, File Creation Day of Year, unsigned short
		UInt16 FileCreateYear;				// 2 bytes, File Creation Year, unsigned short
		UInt16 HdrSize;						// 2 bytes, Header Size, unsigned short *
		UInt32 DataOffset;					// 4 bytes, Offset to point data, unsigned long *
		UInt32 VarLenRecCount;				// 4 bytes, Number of variable length records, unsigned long *
		UInt8  PointFormat;					// 1 byte, Point Data Format ID(0 - 99 for spec), unsigned char *
											//		Version 1.2 only had types 0-3
		UInt16 PointDataRecLen;				// 2 bytes, Point Data Record Length, unsigned short *
		UInt32 PointDataRecCount;			// 4 bytes, Number of point records, unsigned long *
		UInt32 ReturnPoints[5];				// 20 bytes, Number of points by return, unsigned long[5] *
		double XScale;						// 8 bytes X scale factor *
		double YScale;						// 8 bytes Y scale factor *
		double ZScale;						// 8 bytes Z scale factor *
		double XOffset;						// 8 bytes X offset *
		double YOffset;						// 8 bytes Y offset *
		double ZOffset;						// 8 bytes Z offset *
		double MaxX;						// 8 bytes *
		double MinX;						// 8 bytes *
		double MaxY;						// 8 bytes *
		double MinY;						// 8 bytes *
		double MaxZ;						// 8 bytes *
		double MinZ;						// 8 bytes *
	};

	struct HeaderType_13					// version 1.3 header
	{
		char Signature[4];					// 4 bytes, "LASF" *
		UInt16 FileSourceID;				// 2 bytes, File Source ID unsigned short *
		UInt16 GlobalEncoding;				// 2 bytes, bit mask, unsigned short
		// Bits Field Name	    Description
		// 0    GPS Time Type	The meaning of GPS Time in the Point Records
		//						0 -> GPS time in the point record fields
		//						 	 is GPS Week Time(the same as previous versions of LAS)
		//						1 -> GPS Time is standard GPS Time (satellite GPS Time)
		//							 minus 1 x 10^9. The offset moves the time back to near zero 
		//							 to improve floating point resolution.
		// 1	Waveform Data Packets Internal
		//						1 -> waveform data packets are
		//							 located within this file (note that this bit is mutually
		//							 exclusive with bit 2)
		// 2    Waveform Data Packets External
		//						1 -> waveform data packets are located external to this 
		//							 file in an auxiliary file with the same base name 
		//							 as this file and the extension “.wdp”.
		//							 (note that this bit is mutually exclusive with bit 1)
		// 3	Return  numbers have been synthetically generated.
		//						1 -> point return numbers in the Point Data Records have been synthetically 
		//							 generated. This could be the case, for example, when a composite 
		//							 file is created by combining a First Return File and a Last Return File.
		//							 In this case, first return data will be labeled “1 of 2” and second 
		//							 return data will be labeled “2 of 2”.
		// 4:15  Reserved		Must be set to zero
		UInt32 GUID1;						// 4 bytes Project ID - GUID data 1 
		UInt16 GUID2;						// 2 bytes Project ID - GUID data 2 
		UInt16 GUID3;						// 2 bytes Project ID - GUID data 3 
		unsigned char GUID4[8];				// 8 bytes Project ID - GUID data 4 
		unsigned char VerMajor;				// 1 byte *
		unsigned char VerMinor;				// 1 byte *
		char SystemID[32];					// 32 bytes, populated by the generating software, char[32] *
		char GenSoftware[32];				// 32 bytes, populated by the generating software, char[32] *
		UInt16 FileCreateDay;				// 2 bytes, File Creation Day of Year, unsigned short
		UInt16 FileCreateYear;				// 2 bytes, File Creation Year, unsigned short
		UInt16 HdrSize;						// 2 bytes, Header Size, unsigned short *
		UInt32 DataOffset;					// 4 bytes, Offset to point data, unsigned long *
		UInt32 VarLenRecCount;				// 4 bytes, Number of variable length records, unsigned long *
		UInt8  PointFormat;					// 1 byte, Point Data Format ID(0 - 99 for spec), unsigned char *
											//		Version 1.3 only had types 0-5
		UInt16 PointDataRecLen;				// 2 bytes, Point Data Record Length, unsigned short *
		UInt32 PointDataRecCount;			// 4 bytes, Number of point records, unsigned long *
		UInt32 ReturnPoints[5];				// 20 bytes, Number of points by return, unsigned long[5] *
		double XScale;						// 8 bytes X scale factor *
		double YScale;						// 8 bytes Y scale factor *
		double ZScale;						// 8 bytes Z scale factor *
		double XOffset;						// 8 bytes X offset *
		double YOffset;						// 8 bytes Y offset *
		double ZOffset;						// 8 bytes Z offset *
		double MaxX;						// 8 bytes *
		double MinX;						// 8 bytes *
		double MaxY;						// 8 bytes *
		double MinY;						// 8 bytes *
		double MaxZ;						// 8 bytes *
		double MinZ;						// 8 bytes *
		UInt64 WaveStart;					// (1.3) 8 bytes, start of waveform data packet record, unsinged long long *
	};

	struct HeaderType_14					// version 1.4 header
	{
		char Signature[4];					// 4 bytes, "LASF" *
		UInt16 FileSourceID;				// 2 bytes, File Source ID unsigned short *
		UInt16 GlobalEncoding;				// 2 bytes, bit mask, unsigned short
		// Bits Field Name	    Description
		// 0    GPS Time Type	The meaning of GPS Time in the Point Records
		//						0 -> GPS time in the point record fields
		//						 	 is GPS Week Time(the same as previous versions of LAS)
		//						1 -> GPS Time is standard GPS Time (satellite GPS Time)
		//							 minus 1 x 10^9. The offset moves the time back to near zero 
		//							 to improve floating point resolution.
		// 1	Waveform Data Packets Internal
		//						1 -> waveform data packets are located within this file
		//							 (note that this bit is mutually exclusive with bit 2)
		// 2    Waveform Data Packets External
		//						1 -> waveform data packets are located external to this 
		//							 file in an auxiliary file with the same base name 
		//							 as this file and the extension “.wdp”.
		//							 (note that this bit is mutually exclusive with bit 1)
		// 3	Return  numbers have been synthetically generated.
		//						1 -> point return numbers in the Point Data Records have been synthetically 
		//							 generated. This could be the case, for example, when a composite 
		//							 file is created by combining a First Return File and a Last Return File.
		//							 In this case, first return data will be labeled “1 of 2” and second 
		//							 return data will be labeled “2 of 2”.
		// 4	WKT				1 -> Coordinate Reference System (CRS) is WKT. 
		//						0 -> CRS is GeoTIFF.
		//						It should not be set if the file writer wishes to ensure legacy
		//						compatibility (which means the CRS must be GeoTIFF)
		// 5:15  Reserved		Must be set to zero.
		UInt32 GUID1;						// 4 bytes Project ID - GUID data 1 
		UInt16 GUID2;						// 2 bytes Project ID - GUID data 2 
		UInt16 GUID3;						// 2 bytes Project ID - GUID data 3 
		unsigned char GUID4[8];				// 8 bytes Project ID - GUID data 4 
		unsigned char VerMajor;				// 1 byte *
		unsigned char VerMinor;				// 1 byte *
		char SystemID[32];					// 32 bytes, populated by the generating software, char[32] *
		char GenSoftware[32];				// 32 bytes, populated by the generating software, char[32] *
		UInt16 FileCreateDay;				// 2 bytes, File Creation Day of Year, unsigned short
		UInt16 FileCreateYear;				// 2 bytes, File Creation Year, unsigned short
		UInt16 HdrSize;						// 2 bytes, Header Size, unsigned short *
		UInt32 DataOffset;					// 4 bytes, Offset to point data, unsigned long *
		UInt32 VarLenRecCount;				// 4 bytes, Number of variable length records, unsigned long *
		UInt8  PointFormat;					// 1 byte, Point Data Format ID(0 - 99 for spec), unsigned char *
											//		Version 1.4 only had types 0-10
		UInt16 PointDataRecLen;				// 2 bytes, Point Data Record Length, unsigned short *
		UInt32 LegacyPointDataRecCount;		// 4 bytes, Number of point records, unsigned long *
		UInt32 LegacyReturnPoints[5];		// 20 bytes, Number of points by return, unsigned long[5] *
		double XScale;						// 8 bytes X scale factor *
		double YScale;						// 8 bytes Y scale factor *
		double ZScale;						// 8 bytes Z scale factor *
		double XOffset;						// 8 bytes X offset *
		double YOffset;						// 8 bytes Y offset *
		double ZOffset;						// 8 bytes Z offset *
		double MaxX;						// 8 bytes *
		double MinX;						// 8 bytes *
		double MaxY;						// 8 bytes *
		double MinY;						// 8 bytes *
		double MaxZ;						// 8 bytes *
		double MinZ;						// 8 bytes *
		UInt64 WaveStart;					// (1.3) 8 bytes, start of waveform data packet record, unsinged long long *
		UInt64 ExtVarStart;					// (1.4) 8 bytes, Start of first Extended Variable Length Record unsigned long long *
		UInt32 ExtVarCount;					// (1.4) 4 bytes, Number of Extended Variable Length Records unsigned long *
		UInt64 PointDataRecCount;			// (1.4) 8 bytes, Number of point records, unsigned long long *
		UInt64 ReturnPionts[15];			// (1.4) 120 bytes, Number of points by return, unsigned long long[15] *
	};

#pragma pack()

	HeaderType_10 m_Header10;		// version 1.0 header
	HeaderType_11 m_Header11;		// version 1.1 header
	HeaderType_12 m_Header12;		// version 1.2 header
	HeaderType_13 m_Header13;		// version 1.3 header
	HeaderType_14 m_Header14;		// version 1.4 header

	double mf_MaxX;
	double mf_MinX;
	double mf_MaxY;
	double mf_MinY;
	double mf_MaxZ;
	double mf_MinZ;
	double mf_ScaleX;
	double mf_ScaleY;
	double mf_ScaleZ;
	double mf_OffsetX;
	double mf_OffsetY;
	double mf_OffsetZ;
	UInt16 mi_GlobalEncoding;
	UInt16 mi_FileSourceID;
	UInt16 mi_FileCreateDay;		// File Creation Day of Year
	UInt16 mi_FileCreateYear;		// File Creation Year
	UInt8  mi_PointFormat;			// point record format
	UInt32 mi_VarRecCount;			// # of variable length records
	UInt64 mi_PointCount;			// # of point data records, 32 bits before 1.4
	UInt32 mi_DataOffset;			// byte offset to point data

	char ms_FileName[255];			// file name
	int  mi_FileHandle;				// file handle

	LASreader* mp_LasReader;

private:

	bool Load(const char* fileName);
	bool LoadLasFile(const char* fileName);
	bool LoadLazFile(const char* fileName);
	void CloseFile();
	UInt16 GetVersion() const;
	UInt8  PackReturnMask8(LASpoint* point) const;
	UInt16 PackReturnMask16(LASpoint* point) const;
};

#endif // #ifndef LASFILE_H
