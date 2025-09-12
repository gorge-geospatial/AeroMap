// Angle.h
// Header-only class that allows encapsulates angles
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#ifndef ANGLE_H
#define ANGLE_H
 
class Angle
{
public:

	enum class Base
    {
		Radians,
        Degrees
	};
        
    Angle()
		: m_radians(0.0)
    {
    }
 
	Angle(double angle, Angle::Base base = Base::Radians)
		: m_radians(0.0)
	{
		if (base == Base::Radians)
			m_radians = angle;
		else
			m_radians = Angle::DegreesToRadians(angle);
	}

	void SetDegrees(double degrees)
	{
		m_radians = Angle::DegreesToRadians(degrees);
	}
	void SetRadians(double radians)
	{
		m_radians = radians;
	}

	double Radians()
	{
		return m_radians;
	}
	double Degrees()
	{
		return Angle::RadiansToDegrees(m_radians);
	}

private:

	static double DegreesToRadians(double d) { return d * 0.0055555555555555555555555555555556 
														* 3.1415926535897932384626433832795; }		// double precision PI value
	static double RadiansToDegrees(double r) { return r * 0.31830988618379067153776752674503 * 180.0; }

private:

	double m_radians;       // angle stored internally as radians
};

#endif // #ifndef ANGLE_H
