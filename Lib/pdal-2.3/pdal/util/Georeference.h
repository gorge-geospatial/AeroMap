#ifndef GEOREFERENCE_H
#define GEOREFERENCE_H

namespace pdal
{
	namespace georeference
	{
		struct Xyz
		{
			Xyz(double x, double y, double z)
				: X(x)
				, Y(y)
				, Z(z)
			{
			}

			double X;
			double Y;
			double Z;
		};

		struct RotationMatrix
		{
			// Row-major
			RotationMatrix(double m00, double m01, double m02, double m10, double m11,
				double m12, double m20, double m21, double m22)
				: m00(m00)
				, m01(m01)
				, m02(m02)
				, m10(m10)
				, m11(m11)
				, m12(m12)
				, m20(m20)
				, m21(m21)
				, m22(m22)
			{
			}

			double m00, m01, m02, m10, m11, m12, m20, m21, m22;
		};

		inline RotationMatrix createIdentityMatrix()
		{
			return RotationMatrix(1, 0, 0, 0, 1, 0, 0, 0, 1);
		}

		// Returns Latitude, Longitude, Height triplet with angles in radians
		Xyz georeferenceWgs84(double range, double scanAngle,
			const RotationMatrix& boresightMatrix,
			const RotationMatrix& imuMatrix, const Xyz& gpsPoint);
	}
}

#endif // #ifndef GEOREFERENCE_H
