/******************************************************************************
* Copyright (c) 2015, Peter J. Gadomski <pete.gadomski@gmail.com>
****************************************************************************/

#include <pdal/util/Georeference.h>

#include <cmath>
#include <iostream>
#include <vector>

namespace pdal
{
	namespace georeference
	{
		namespace
		{
			static const double a = 6378137.0;
			static const double f = 1 / 298.257223563;
			static const double e2 = 2 * f - f * f;

			Xyz rotate(const Xyz& point, const RotationMatrix& matrix)
			{
				return Xyz(
					matrix.m00 * point.X + matrix.m01 * point.Y + matrix.m02 * point.Z,
					matrix.m10 * point.X + matrix.m11 * point.Y + matrix.m12 * point.Z,
					matrix.m20 * point.X + matrix.m21 * point.Y + matrix.m22 * point.Z);
			}

			Xyz cartesianToCurvilinear(const Xyz& point, double latitude)
			{
				double w = std::sqrt(1 - e2 * std::sin(latitude) * std::sin(latitude));
				double n = a / w;
				double m = a * (1 - e2) / (w * w * w);
				return Xyz(point.X / (n * std::cos(latitude)), point.Y / m, point.Z);
			}
		}

		Xyz georeferenceWgs84(double range, double scanAngle,
			const RotationMatrix& boresightMatrix,
			const RotationMatrix& imuMatrix, const Xyz& gpsPoint)
		{
			Xyz pSocs = Xyz(range * std::sin(scanAngle), 0, -range * std::cos(scanAngle));

			Xyz pSocsAligned = rotate(pSocs, boresightMatrix);
			Xyz pLocalLevel = rotate(pSocsAligned, imuMatrix);
			Xyz pCurvilinear = cartesianToCurvilinear(pLocalLevel, gpsPoint.Y);

			return Xyz(gpsPoint.X + pCurvilinear.X, gpsPoint.Y + pCurvilinear.Y,
				gpsPoint.Z + pCurvilinear.Z);
		}
	}
}
