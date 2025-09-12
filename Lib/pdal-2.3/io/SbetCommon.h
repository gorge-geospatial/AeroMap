/******************************************************************************
* Copyright (c) 2014, Peter J. Gadomski (pete.gadomski@gmail.com)
****************************************************************************/

#pragma once

#include <pdal/Dimension.h>

namespace pdal
{
	namespace sbet
	{
		// This is static so as to be made local (internal linkage) in the translation
		// units in which it's included.
		static inline Dimension::IdList fileDimensions()
		{
			// Data for each point is in the source file in the order these dimensions
			 // are listed, I would suppose.  Would be really nice to have a reference
			 // to the file spec.  I searched the Internet and found that it is from
			 // some company called Applanix (Trimble), but I can't find anything
			 // describing the file format on their website.

			using namespace Dimension;
			return { Id::GpsTime, Id::Y, Id::X, Id::Z, Id::XVelocity, Id::YVelocity,
				Id::ZVelocity, Id::Roll, Id::Pitch, Id::Azimuth, Id::WanderAngle,
				Id::XBodyAccel, Id::YBodyAccel, Id::ZBodyAccel, Id::XBodyAngRate,
				Id::YBodyAngRate, Id::ZBodyAngRate };
		}

		static inline bool isAngularDimension(Dimension::Id dimension) {
			using namespace Dimension;
			switch (dimension) {
			case Id::X:
			case Id::Y:
			case Id::Roll:
			case Id::Pitch:
			case Id::Azimuth:
			case Id::WanderAngle:
			case Id::XBodyAngRate:
			case Id::YBodyAngRate:
			case Id::ZBodyAngRate:
				return true;
			default:
				return false;
			}
		};
	}
}
