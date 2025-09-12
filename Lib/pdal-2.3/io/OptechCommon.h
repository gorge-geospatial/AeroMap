#ifndef OPTECHCOMMON_H
#define OPTECHCOMMON_H

#include <pdal/util/Georeference.h>

namespace pdal
{
	// Optech csd files contain misalignment angles and IMU offsets.
	// Misalignment angles and IMU offsets combine to form the boresight matrix.
	typedef struct
	{
		char signature[4];
		char vendorId[64];
		char softwareVersion[32];
		float formatVersion;
		uint16_t headerSize;
		uint16_t gpsWeek;
		double minTime; // seconds
		double maxTime; // seconds
		uint32_t numRecords;
		uint16_t numStrips;
		uint32_t stripPointers[256];
		double misalignmentAngles[3]; // radians
		double imuOffsets[3];         // radians
		double temperature;           // degrees
		double pressure;              // mbar
		char freeSpace[830];
	} CsdHeader;


	typedef struct
	{
		double gpsTime;
		uint8_t returnCount;
		float range[4]; // metres
		uint16_t intensity[4];
		float scanAngle;  // radians
		float roll;       // radians
		float pitch;      // radians
		float heading;    // radians
		double latitude;  // radians
		double longitude; // radians
		float elevation;  // metres
	} CsdPulse;


	// Optech does it like R3(heading) * R1(-pitch) * R2(-roll)
	inline pdal::georeference::RotationMatrix
		createOptechRotationMatrix(double roll, double pitch, double heading)
	{
		return georeference::RotationMatrix(
			std::cos(roll) * std::cos(heading) +
			std::sin(pitch) * std::sin(roll) * std::sin(heading), // m00
			std::cos(pitch) * std::sin(heading),                      // m01
			std::cos(heading) * std::sin(roll) -
			std::cos(roll) * std::sin(pitch) * std::sin(heading), // m02
			std::cos(heading) * std::sin(pitch) * std::sin(roll) -
			std::cos(roll) * std::sin(heading), // m10
			std::cos(pitch) * std::cos(heading),    // m11
			-std::sin(roll) * std::sin(heading) -
			std::cos(roll) * std::cos(heading) * std::sin(pitch), // m12
			-std::cos(pitch) * std::sin(roll),                        // m20
			std::sin(pitch),                                          // m21
			std::cos(pitch) * std::cos(roll)                          // m22
		);
	}
}

#endif // #ifndef OPTECHCOMMON_H
